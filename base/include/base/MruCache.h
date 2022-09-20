/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_BASE_MRUCACHE_
#define ANDROID_BASE_MRUCACHE_

#include <algorithm>
#include <filesystem>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "base/Tracing.h"
#include "base/WorkerThread.h"
#include "host-common/logging.h"

namespace android {
namespace base {

// A trivial MRU cache. Not thread-safe.
template <class K, class V>
class MruCache {
   public:
    template <class S>
    struct EntryWithSize {
        explicit EntryWithSize(S&& d)
            : EntryWithSize(std::move(d), std::size(d) * sizeof(typename S::value_type)) {}

        explicit EntryWithSize(S&& d, const size_t ds) : data(std::move(d)), dataSize(ds) {
            static_assert(std::is_same<S, K>::value || std::is_same<S, V>::value,
                          "Cache entry instantiated with invalid types");
        }

        const S data;
        size_t dataSize;

        bool operator==(const EntryWithSize& rhs) const { return data == rhs.data; }
        bool operator<(const EntryWithSize& rhs) const { return data < rhs.data; }
    };

    class MruCacheObserver {
       public:
        virtual void cacheChanged(bool forceUpdate) = 0;
        virtual ~MruCacheObserver() {}
    };

    class CacheFlattener {
       public:
        static std::unique_ptr<CacheFlattener> createFlatBuffersFlattener();
        virtual std::optional<std::vector<uint8_t>> handleFlatten(
            const std::map<EntryWithSize<K>, EntryWithSize<V>>& cache) const = 0;
        virtual bool handleUnflatten(MruCache& cache, const uint8_t* buf, size_t bufSize) const = 0;
        virtual ~CacheFlattener() {}
    };

    MruCache(size_t maxEntries, const CacheFlattener* cacheFlattener)
        : mMaxEntries(maxEntries), mCacheObserver(nullptr), mCacheFlattener(cacheFlattener) {}

    bool put(const K& key, size_t keySize, V&& value, size_t valueSize) {
        evictIfNecessary();
        K keyCopy = key;
        EntryWithSize<K> cacheKey(std::move(keyCopy), keySize);
        EntryWithSize<V> cacheValue(std::move(value), valueSize);

        auto exists = mCache.find(cacheKey);
        bool overwrite = exists != mCache.end();
        if (overwrite) {
            auto iter = std::find(mMostRecents.begin(), mMostRecents.end(), cacheKey);
            mMostRecents.splice(mMostRecents.begin(), mMostRecents, iter);
            mCache.erase(exists);
        } else {
            mMostRecents.push_front(cacheKey);
        }

        const auto [_, res] = mCache.insert({std::move(cacheKey), std::move(cacheValue)});

        if (mCacheObserver != nullptr && res) {
            mCacheObserver->cacheChanged(overwrite);
        }

        return res;
    }

    bool unflatten(const void* buf, size_t bufSize) {
        return mCacheFlattener != nullptr
                   ? mCacheFlattener->handleUnflatten(*this, reinterpret_cast<const uint8_t*>(buf),
                                                      bufSize)
                   : false;
    }

    std::optional<std::vector<uint8_t>> flatten() {
        return mCacheFlattener != nullptr ? mCacheFlattener->handleFlatten(mCache) : std::nullopt;
    }

    bool get(const K& key, const V** value) {
        K keyCopy = key;
        EntryWithSize<K> cacheKey(std::move(keyCopy));
        auto res = mCache.find(cacheKey);

        if (res == mCache.end()) {
            return false;
        }

        *value = &(res->second.data);
        auto iter = std::find(mMostRecents.begin(), mMostRecents.end(), cacheKey);
        mMostRecents.splice(mMostRecents.begin(), mMostRecents, iter);

        return true;
    }

    void setObserver(MruCacheObserver* observer) { mCacheObserver = observer; }

   private:
    using MruCacheMap = std::map<EntryWithSize<K>, EntryWithSize<V>>;
    using MostRecentList = std::list<EntryWithSize<K>>;

    void evictIfNecessary() {
        auto entryCount = mMostRecents.size();
        if (entryCount >= mMaxEntries) {
            auto threshold = entryCount * 0.9;

            for (int i = mMostRecents.size(); i > threshold; i--) {
                const EntryWithSize<K>& key = mMostRecents.front();
                mCache.erase(key);
                mMostRecents.pop_front();
            }
        }
    }

    MruCacheMap mCache;
    const size_t mMaxEntries;
    MostRecentList mMostRecents;
    MruCacheObserver* mCacheObserver;
    const CacheFlattener* mCacheFlattener;
};

class FileBackupCacheObserverFsWorker {
   public:
    FileBackupCacheObserverFsWorker();
    ~FileBackupCacheObserverFsWorker();
    std::future<std::optional<std::vector<uint8_t>>> readFile(const std::string& srcPath);
    std::future<void> writeFile(const std::string& path, std::vector<uint8_t> data);

   private:
    struct WriteIovCmd {
        std::string path;
        std::vector<uint8_t> data;
    };

    struct ReadIovCmd {
        std::string path;
        std::promise<std::optional<std::vector<uint8_t>>> resultData;
    };

    struct ExitCmd {};

    using Cmd = std::variant<WriteIovCmd, ReadIovCmd, ExitCmd>;
    WorkerProcessingResult processor(Cmd&&);
    WorkerThread<Cmd> mWorker;
};

template <class K, class V>
class FileBackupCacheObserver final : public MruCache<K, V>::MruCacheObserver {
   public:
    FileBackupCacheObserver(android::base::MruCache<K, V>& cacheReference, uint32_t cacheUpdateRate)
        : FileBackupCacheObserver(cacheReference, std::nullopt, cacheUpdateRate) {}
    FileBackupCacheObserver(android::base::MruCache<K, V>& cacheReference,
                            std::optional<std::string> fileName, const uint32_t cacheUpdateRate)
        : mFsWorker(),
          mFileName(std::move(fileName)),
          mIsLastWriteSuccess(true),
          mCacheChangeCount(0),
          mCacheUpdateRate(cacheUpdateRate),
          mCache(cacheReference) {
        preloadCache();
        mCache.setObserver(this);
    }
    virtual ~FileBackupCacheObserver() {}
    virtual void cacheChanged(bool forceUpdate) override {
        mCacheChangeCount++;
        if (mIsLastWriteSuccess && !forceUpdate && (mCacheChangeCount % mCacheUpdateRate != 0)) {
            return;
        }
        if (mFileName) {
            auto maybeFlatCache = mCache.flatten();
            if (!maybeFlatCache.has_value()) {
                ERR("Failed to flatten the cache.");
                mIsLastWriteSuccess = false;
                return;
            }
            std::vector<uint8_t> flatCache = std::move(*maybeFlatCache);
            mFsWorker.writeFile(*mFileName, std::move(flatCache));
            mCacheChangeCount = 0;
            mIsLastWriteSuccess = true;
        }
    }

   private:
    FileBackupCacheObserverFsWorker mFsWorker;
    std::optional<std::string> mFileName;
    bool mIsLastWriteSuccess;
    uint64_t mCacheChangeCount;
    uint64_t mCacheUpdateRate;
    MruCache<K, V>& mCache;

    void preloadCache() {
        if (mFileName.has_value()) {
            std::optional<std::vector<uint8_t>> maybeData = mFsWorker.readFile(*mFileName).get();
            if (!maybeData.has_value()) {
                ERR("Failed to read from file: %s.", mFileName->c_str());
                return;
            }
            std::vector<uint8_t> data = std::move(*maybeData);
            if (data.empty()) {
                return;
            }
            if (!mCache.unflatten(data.data(), data.size())) {
                ERR("Failed to unflatten the cache from %s.", mFileName->c_str());
            }
        }
    }
};

using ByteVector = std::vector<uint8_t>;
}  // namespace base
}  // namespace android

#endif
