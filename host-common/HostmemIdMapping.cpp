// Copyright 2019 The Android Open Source Project
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
#include "host-common/HostmemIdMapping.h"

#include <utility>

using android::base::ManagedDescriptor;

namespace android {
namespace emulation {

static HostmemIdMapping* sMapping() {
    static HostmemIdMapping* s = new HostmemIdMapping;
    return s;
}

// static
HostmemIdMapping* HostmemIdMapping::get() {
    return sMapping();
}

void HostmemIdMapping::addMapping(uint32_t ctxId, uint64_t blobId, void* addr, uint32_t caching) {
    struct HostMemInfo info = {
        .addr = addr,
        .caching = caching,
    };

    auto key = std::make_pair(ctxId, blobId);
    mHostMemInfos.insert(std::make_pair(key, std::move(info)));
}

std::optional<HostMemInfo> HostmemIdMapping::removeMapping(uint32_t ctxId, uint64_t blobId) {
    auto key = std::make_pair(ctxId, blobId);
    auto found = mHostMemInfos.find(key);
    if (found != mHostMemInfos.end()) {
        std::optional<HostMemInfo> ret = std::move(found->second);
        mHostMemInfos.erase(found);
        return ret;
    }

    return std::nullopt;
}

void HostmemIdMapping::addDescriptorInfo(uint32_t ctxId, uint64_t blobId,
                                         ManagedDescriptor descriptor, uint32_t handleType,
                                         uint32_t caching,
                                         std::optional<VulkanInfo> vulkanInfoOpt) {
    struct ManagedDescriptorInfo info =
        {
            .descriptor = std::move(descriptor),
            .handleType = handleType,
            .caching = caching,
            .vulkanInfoOpt = std::move(vulkanInfoOpt),
        };

    auto key = std::make_pair(ctxId, blobId);
    mDescriptorInfos.insert(std::make_pair(key, std::move(info)));
}

std::optional<ManagedDescriptorInfo> HostmemIdMapping::removeDescriptorInfo(uint32_t ctxId,
                                                                            uint64_t blobId) {
    auto key = std::make_pair(ctxId, blobId);
    auto found = mDescriptorInfos.find(key);
    if (found != mDescriptorInfos.end()) {
        std::optional<ManagedDescriptorInfo> ret = std::move(found->second);
        mDescriptorInfos.erase(found);
        return ret;
    }

    return std::nullopt;
}

} // namespace android
}  // namespace android
