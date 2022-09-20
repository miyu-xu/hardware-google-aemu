/*
 * Copyright (C) 2022 The Android Open Source Project
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "base/MruCache.h"

#include <algorithm>
#include <iterator>
#include <tuple>
#include <vector>

#include "FlatByteVectorMruCache_generated.h"
#include "flatbuffers/flatbuffers.h"
#include "testing/TestUtils.h"

namespace android {
namespace base {
namespace {

using ::testing::_;
using ::testing::Args;
using ::testing::ElementsAreArray;
using ::testing::MatcherCast;
using ::testing::Ref;
using ::testing::Return;

struct FlatBuffersFlattenerContext {
    std::unique_ptr<MruCache<ByteVector, ByteVector>> cache;
    std::unique_ptr<MruCache<ByteVector, ByteVector>::CacheFlattener> flattener;
    std::vector<uint8_t> flatCache;
};

std::unique_ptr<FlatBuffersFlattenerContext> createFlatBuffersFlattenerContext(
    const std::vector<std::tuple<ByteVector, ByteVector>>& entries) {
    std::unique_ptr<MruCache<ByteVector, ByteVector>::CacheFlattener> flattener =
        MruCache<ByteVector, ByteVector>::CacheFlattener::createFlatBuffersFlattener();
    EXPECT_NE(flattener, nullptr);
    if (!flattener) {
        return nullptr;
    }
    auto cache =
        std::make_unique<MruCache<ByteVector, ByteVector>>(entries.size(), flattener.get());
    for (const auto& [key, value] : entries) {
        ByteVector valueCopy = value;
        bool res = false;
        EXPECT_TRUE(res = cache->put(key, key.size(), std::move(valueCopy), value.size()));
        if (!res) {
            return nullptr;
        }
    }
    std::optional<std::vector<uint8_t>> maybeFlatCache = cache->flatten();
    EXPECT_TRUE(maybeFlatCache.has_value());
    if (!maybeFlatCache.has_value()) {
        return nullptr;
    }
    std::vector<uint8_t> flatCache = std::move(*maybeFlatCache);
    auto res = std::make_unique<FlatBuffersFlattenerContext>();
    res->cache = std::move(cache);
    res->flattener = std::move(flattener);
    res->flatCache = std::move(flatCache);
    return res;
}

TEST(MruCacheTest, FlatBuffersFlattenerFlattenAndUnflattenShouldRestoreTheOriginalData) {
    std::vector<std::tuple<ByteVector, ByteVector>> entries = {
        {{1, 2, 3}, {38, 48, 29}},
        {{9}, {}},
        {{}, {2, 5, 71}},
    };
    std::unique_ptr<FlatBuffersFlattenerContext> context =
        createFlatBuffersFlattenerContext(entries);
    ASSERT_NE(context, nullptr);
    const std::vector<uint8_t>& flatCache = context->flatCache;
    MruCache<ByteVector, ByteVector> cache(10, context->flattener.get());

    ASSERT_TRUE(cache.unflatten(flatCache.data(), flatCache.size()));
    for (const auto& [key, expectedValue] : entries) {
        const ByteVector* actualValue = nullptr;
        ASSERT_TRUE(cache.get(key, &actualValue));
        EXPECT_THAT(*actualValue, ElementsAreArray(expectedValue));
    }
}

TEST(MruCacheTest, FlatBuffersFlattenerUnflattenShouldDiscardIncompleteData) {
    std::unique_ptr<FlatBuffersFlattenerContext> context = createFlatBuffersFlattenerContext({
        {{1, 2, 3}, {38, 48, 29}},
        {{9}, {}},
        {{100, 200, 34}, {2, 5, 71}},
    });
    ASSERT_NE(context, nullptr);
    const std::vector<uint8_t>& flatCache = context->flatCache;
    ASSERT_GE(flatCache.size(), 2);
    std::vector<uint8_t> flatCacheIncomplete(flatCache.begin(),
                                             flatCache.begin() + flatCache.size() / 2);
    MruCache<ByteVector, ByteVector> cache(10, context->flattener.get());

    ASSERT_FALSE(cache.unflatten(flatCacheIncomplete.data(), flatCacheIncomplete.size()));
}

TEST(MruCacheTest, FlatBuffersFlattenerUnflattenShouldDiscardInvalidData) {
    std::unique_ptr<FlatBuffersFlattenerContext> context = createFlatBuffersFlattenerContext({
        {{1, 2, 3}, {38, 48, 29}},
        {{9}, {}},
        {{100, 200, 34}, {2, 5, 71}},
    });
    ASSERT_NE(context, nullptr);
    const std::vector<uint8_t>& flatCache = context->flatCache;
    ASSERT_GE(flatCache.size(), 2);
    std::vector<uint8_t> flatCacheInvalid = flatCache;
    std::fill(flatCacheInvalid.begin() + flatCacheInvalid.size() / 2, flatCacheInvalid.end(), 0);
    MruCache<ByteVector, ByteVector> cache(10, context->flattener.get());

    ASSERT_FALSE(cache.unflatten(flatCacheInvalid.data(), flatCacheInvalid.size()));
}

TEST(MruCacheTest, FlatBuffersFlattenerUnflattenShouldDiscardDataWithInvalidKey) {
    std::unique_ptr<FlatBuffersFlattenerContext> context = createFlatBuffersFlattenerContext({
        {{1, 2, 3}, {38, 48, 29}},
        {{9}, {}},
        {{100, 200, 34}, {2, 5, 71}},
    });
    ASSERT_NE(context, nullptr);
    std::vector<uint8_t> flatCacheInvalid = context->flatCache;
    auto cacheFromBuffer = GetMutableByteVectorMruCache(flatCacheInvalid.data());
    ASSERT_GE(cacheFromBuffer->entries()->size(), 1);
    auto entry = cacheFromBuffer->mutable_entries()->GetMutableObject(
        cacheFromBuffer->entries()->size() - 1);
    auto key = entry->mutable_key();
    for (size_t i = 0; i < key->size(); i++) {
        key->Mutate(i, 0);
    }
    std::vector<uint8_t> invalidKey(key->begin(), key->end());
    MruCache<ByteVector, ByteVector> cache(10, context->flattener.get());
    EXPECT_FALSE(cache.unflatten(flatCacheInvalid.data(), flatCacheInvalid.size()));
    const std::vector<uint8_t>* value = nullptr;
    EXPECT_FALSE(cache.get(invalidKey, &value));
}

TEST(MruCacheTest, FlatBuffersFlattenerUnflattenShouldDiscardDataWithInvalidValue) {
    std::unique_ptr<FlatBuffersFlattenerContext> context = createFlatBuffersFlattenerContext({
        {{1, 2, 3}, {38, 48, 29}},
        {{9}, {}},
        {{100, 200, 34}, {2, 5, 71}},
    });
    ASSERT_NE(context, nullptr);
    std::vector<uint8_t> flatCacheInvalid = context->flatCache;
    auto cacheFromBuffer = GetMutableByteVectorMruCache(flatCacheInvalid.data());
    ASSERT_GE(cacheFromBuffer->entries()->size(), 1);
    auto entry = cacheFromBuffer->mutable_entries()->GetMutableObject(
        cacheFromBuffer->entries()->size() - 1);
    auto invalidValue = entry->mutable_value();
    for (size_t i = 0; i < invalidValue->size(); i++) {
        invalidValue->Mutate(i, 0);
    }
    std::vector<uint8_t> key(entry->key()->begin(), entry->key()->end());
    MruCache<ByteVector, ByteVector> cache(10, context->flattener.get());
    EXPECT_FALSE(cache.unflatten(flatCacheInvalid.data(), flatCacheInvalid.size()));
    const std::vector<uint8_t>* value = nullptr;
    EXPECT_FALSE(cache.get(key, &value));
}

using StringMruCache = MruCache<std::string, std::string>;
using StringEntryWithSize = StringMruCache::EntryWithSize<std::string>;
using StringEntryMap = std::map<StringEntryWithSize, StringEntryWithSize>;
class MockFlattener : public StringMruCache::CacheFlattener {
   public:
    MOCK_METHOD(std::optional<std::vector<uint8_t>>, handleFlatten, (const StringEntryMap& cache),
                (const, override));
    MOCK_METHOD(bool, handleUnflatten, (StringMruCache & cache, const uint8_t* buf, size_t bufSize),
                (const, override));
};

TEST(MruCacheTest, FileBackupObserverShouldPreloadTheDataFromFile) {
    MockFlattener mockFlattener;
    StringMruCache cache(10, &mockFlattener);
    std::string content = "test\n \ncontent";
    auto tempFile = ScopedTempFile::create(
        ::testing::UnitTest::GetInstance()->current_test_info()->name(), content);
    ASSERT_NE(tempFile, nullptr);
    EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, content.size()))
        .With(Args<1, 2>(ElementsAreArray(content)))
        .Times(1)
        .WillOnce(Return(true));
    FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 1);
}

TEST(MruCacheTest, FileBackupObserverShouldDoNothingIfCacheFileDoesNotExist) {
    MockFlattener mockFlattener;
    StringMruCache cache(10, &mockFlattener);
    EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, _)).Times(0);
    FileBackupCacheObserver<std::string, std::string> observer(cache, "not/existing/file/path", 1);
}

TEST(MruCacheTest, FileBackupObserverShouldNotLockTheCacheFileAfterPreloading) {
    MockFlattener mockFlattener;
    StringMruCache cache(10, &mockFlattener);
    auto tempFile = ScopedTempFile::create(
        ::testing::UnitTest::GetInstance()->current_test_info()->name(), "test content");
    ASSERT_NE(tempFile, nullptr);
    EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, _)).Times(1).WillOnce(Return(true));
    FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 1);
    // The attempt to open the backup file should success.
    {
        std::ifstream stream(tempFile->mFilePath, std::ios::binary | std::ios::in);
        EXPECT_FALSE(stream.fail());
    }
    {
        std::ofstream stream(tempFile->mFilePath, std::ios::binary | std::ios::out | std::ios::ate);
        EXPECT_FALSE(stream.fail());
    }
}

TEST(MruCacheTest, FileBackupObserverShouldDoNothingIfFlattenReturnsEmptyBlob) {
    MockFlattener mockFlattener;
    StringMruCache cache(10, &mockFlattener);
    auto tempFile =
        ScopedTempFile::create(::testing::UnitTest::GetInstance()->current_test_info()->name(), "");
    ASSERT_NE(tempFile, nullptr);
    EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, _)).Times(0);
    FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 5);
}

TEST(MruCacheTest, FileBackupObserverShouldSaveToCacheFileOnChangeBasedOnFlushRate) {
    std::string content = "test\n content\n 8321";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile =
        ScopedTempFile::create(::testing::UnitTest::GetInstance()->current_test_info()->name(), "");
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 5);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("a", 1, "0", 1);
        cache.put("b", 1, "1", 1);
        cache.put("c", 1, "2", 1);
        cache.put("d", 1, "3", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(contentVector));
        cache.put("e", 1, "4", 1);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

TEST(MruCacheTest, FileBackupObserverShouldOverwriteExistingCacheFileWhenSaving) {
    std::string content = "test\n content\n 292178";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile = ScopedTempFile::create(
        ::testing::UnitTest::GetInstance()->current_test_info()->name(), "xyz\n1234\n4567");
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, _))
            .Times(1)
            .WillOnce(Return(true));
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(contentVector));
        cache.put("a", 1, "0", 1);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

TEST(MruCacheTest, FileBackupObserverShouldAlwaysSaveToCacheFileOnChangeIfEntriesAreUpdated) {
    std::string content = "test\n content\n 4947823";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile =
        ScopedTempFile::create(::testing::UnitTest::GetInstance()->current_test_info()->name(), "");
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 5);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("a", 1, "0", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(contentVector));
        cache.put("a", 1, "00", 2);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

TEST(MruCacheTest, FileBackupObserverShouldResetFlushCounterIfEntriesAreUpdated) {
    std::string content = "test\n content\n 2817390";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile =
        ScopedTempFile::create(::testing::UnitTest::GetInstance()->current_test_info()->name(), "");
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 5);
        std::vector<uint8_t> differentContentVector = contentVector;
        differentContentVector.emplace_back('0');
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("a", 1, "0", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_))
            .Times(1)
            .WillOnce(Return(differentContentVector));
        cache.put("a", 1, "00", 2);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("b", 1, "1", 1);
        cache.put("c", 1, "2", 1);
        cache.put("d", 1, "3", 1);
        cache.put("e", 1, "4", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(contentVector));
        cache.put("f", 1, "5", 1);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

TEST(MruCacheTest, FileBackupObserverShouldDoNothingIfFlattenFails) {
    std::string content = "test\n content\n 937219";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile = ScopedTempFile::create(
        ::testing::UnitTest::GetInstance()->current_test_info()->name(), content);
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        EXPECT_CALL(mockFlattener, handleUnflatten(Ref(cache), _, _))
            .Times(1)
            .WillOnce(Return(true));
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(std::nullopt));
        cache.put("a", 1, "0", 1);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

TEST(MruCacheTest, FileBackupObserverShouldFlushIfPreviousFlattenFails) {
    std::string content = "test\n content\n 028920279";
    std::vector<uint8_t> contentVector(content.begin(), content.end());
    auto tempFile =
        ScopedTempFile::create(::testing::UnitTest::GetInstance()->current_test_info()->name(), "");
    ASSERT_NE(tempFile, nullptr);
    {
        // Have to destroy the cache and the observer to make sure the contents are written to the
        // file.
        MockFlattener mockFlattener;
        StringMruCache cache(10, &mockFlattener);
        FileBackupCacheObserver<std::string, std::string> observer(cache, tempFile->mFilePath, 5);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("a", 1, "0", 1);
        cache.put("b", 1, "1", 1);
        cache.put("c", 1, "2", 1);
        cache.put("d", 1, "3", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(std::nullopt));
        cache.put("e", 1, "4", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(1).WillOnce(Return(contentVector));
        cache.put("f", 1, "5", 1);
        EXPECT_CALL(mockFlattener, handleFlatten(_)).Times(0);
        cache.put("g", 1, "6", 1);
        cache.put("h", 1, "7", 1);
    }
    std::ifstream stream(tempFile->mFilePath, std::ios::in | std::ios::binary);
    ASSERT_FALSE(stream.fail());
    std::vector<uint8_t> actualContent(std::istreambuf_iterator<char>(stream), {});
    ASSERT_FALSE(stream.fail());
    EXPECT_THAT(actualContent, ElementsAreArray(contentVector));
}

}  // namespace
}  // namespace base
}  // namespace android