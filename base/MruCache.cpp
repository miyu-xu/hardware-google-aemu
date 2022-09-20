// Copyright (C) 2022 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "base/MruCache.h"

#include <array>
#include <fstream>
#include <iterator>
#include <sstream>

#include "FlatByteVectorMruCache_generated.h"
#include "flatbuffers/flatbuffers.h"
#include "host-common/GfxstreamFatalError.h"

namespace android {
namespace base {
namespace {
using emugl::ABORT_REASON_OTHER;
using emugl::FatalError;

constexpr std::array<uint32_t, 256> crc32ForBytes() {
    std::array<uint32_t, 256> res{};
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t x = i;
        for (size_t j = 0; j < 8; ++j) {
            if (x & 1) {
                x = 0xedb88320 ^ (x >> 1);
            } else {
                x >>= 1;
            }
        }
        res[i] = x;
    }
    return res;
}

// T must support range-based for loop, and the elements must be of uint8_t type.
template <class T>
uint32_t crc32(const T& input) {
    constexpr std::array<uint32_t, 256> kTable = crc32ForBytes();

    uint32_t crc32 = 0xffff'ffff;

    for (uint8_t x : input) {
        crc32 = (crc32 >> 8) ^ kTable[(crc32 & 0xff) ^ x];
    }

    return ~crc32;
}
}  // namespace

template <>
std::unique_ptr<typename MruCache<ByteVector, ByteVector>::CacheFlattener>
MruCache<ByteVector, ByteVector>::CacheFlattener::createFlatBuffersFlattener() {
    class FlatBuffersFlattener : public CacheFlattener {
       public:
        FlatBuffersFlattener() {}
        virtual std::optional<std::vector<uint8_t>> handleFlatten(
            const std::map<EntryWithSize<ByteVector>, EntryWithSize<ByteVector>>& cache)
            const override {
            flatbuffers::FlatBufferBuilder builder;
            std::vector<flatbuffers::Offset<ByteVectorMruCacheEntry>> entryVector;
            for (const auto& [key, value] : cache) {
                auto keyOffset = builder.CreateVector(key.data.data(), key.dataSize);
                auto valueOffset = builder.CreateVector(value.data.data(), value.dataSize);
                ByteVectorMruCacheEntryBuilder entryBuilder(builder);
                entryBuilder.add_key(keyOffset);
                entryBuilder.add_value(valueOffset);
                entryBuilder.add_key_crc32(crc32(key.data));
                entryBuilder.add_value_crc32(crc32(value.data));
                entryVector.push_back(entryBuilder.Finish());
            }
            auto entriesOffset = builder.CreateVector(entryVector);
            auto rootOffset = CreateByteVectorMruCache(builder, entriesOffset);
            builder.Finish(rootOffset);
            return std::vector(builder.GetBufferPointer(),
                               builder.GetBufferPointer() + builder.GetSize());
        }

        virtual bool handleUnflatten(MruCache& cache, const uint8_t* buf,
                                     size_t bufSize) const override {
            const auto cacheFromBuffer = GetByteVectorMruCache(buf);
            flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(buf), bufSize);
            if (!VerifyByteVectorMruCacheBuffer(verifier)) {
                return false;
            }
            const auto entries = cacheFromBuffer->entries();
            bool res = true;
            for (size_t i = 0; i < entries->size(); i++) {
                const auto entry = entries->Get(i);
                if (crc32(*entry->key()) != entry->key_crc32() ||
                    crc32(*entry->value()) != entry->value_crc32()) {
                    res = false;
                    continue;
                }
                std::vector<uint8_t> key(entry->key()->begin(), entry->key()->end());
                std::vector<uint8_t> value(entry->value()->begin(), entry->value()->end());
                size_t valueSize = value.size();
                cache.put(key, key.size(), std::move(value), valueSize);
            }
            return res;
        }
        virtual ~FlatBuffersFlattener() {}
    };
    return std::make_unique<FlatBuffersFlattener>();
}

FileBackupCacheObserverFsWorker::FileBackupCacheObserverFsWorker()
    : mWorker([this](FileBackupCacheObserverFsWorker::Cmd&& cmd) {
          return processor(std::move(cmd));
      }) {
    if (!mWorker.start()) {
        GFXSTREAM_ABORT(FatalError(ABORT_REASON_OTHER))
            << "Failed to start the FileBackupCacheObserverFsWorker thread worker.";
    }
}

FileBackupCacheObserverFsWorker::~FileBackupCacheObserverFsWorker() {
    mWorker.enqueue(ExitCmd{}).wait();
}

std::future<std::optional<std::vector<uint8_t>>> FileBackupCacheObserverFsWorker::readFile(
    const std::string& srcPath) {
    std::promise<std::optional<std::vector<uint8_t>>> resultData;
    auto res = resultData.get_future();

    mWorker.enqueue(ReadIovCmd{
        .path = srcPath,
        .resultData = std::move(resultData),
    });
    return res;
}

std::future<void> FileBackupCacheObserverFsWorker::writeFile(const std::string& path,
                                                             std::vector<uint8_t> data) {
    return mWorker.enqueue(WriteIovCmd{
        .path = path,
        .data = std::move(data),
    });
}

WorkerProcessingResult FileBackupCacheObserverFsWorker::processor(Cmd&& cmd) {
    struct {
        WorkerProcessingResult operator()(ExitCmd) { return WorkerProcessingResult::Stop; }
        WorkerProcessingResult operator()(WriteIovCmd writeIovCmd) {
            std::ofstream stream(writeIovCmd.path,
                                 std::ios::out | std::ios::binary | std::ios::trunc);
            if (stream.fail()) {
                ERR("Failed to open %s for writing.", writeIovCmd.path.c_str());
                return WorkerProcessingResult::Continue;
            }
            stream.write(reinterpret_cast<const char*>(writeIovCmd.data.data()),
                         writeIovCmd.data.size());
            if (stream.fail()) {
                ERR("Failed to write to %s.", writeIovCmd.path.c_str());
                return WorkerProcessingResult::Continue;
            }
            return WorkerProcessingResult::Continue;
        }
        WorkerProcessingResult operator()(ReadIovCmd readIovCmd) {
            std::ifstream stream(readIovCmd.path, std::ios::in | std::ios::binary);
            if (stream.fail()) {
                ERR("Failed to open %s for reading.", readIovCmd.path.c_str());
                readIovCmd.resultData.set_value(std::nullopt);
                return WorkerProcessingResult::Continue;
            }
            std::vector<uint8_t> content(std::istreambuf_iterator<char>(stream), {});
            if (stream.fail()) {
                ERR("Failed to read from %s.", readIovCmd.path.c_str());
                readIovCmd.resultData.set_value(std::nullopt);
                return WorkerProcessingResult::Continue;
            }
            readIovCmd.resultData.set_value(std::move(content));
            return WorkerProcessingResult::Continue;
        }
    } visitor;
    return std::visit(visitor, std::move(cmd));
}

}  // namespace base
}  // namespace android