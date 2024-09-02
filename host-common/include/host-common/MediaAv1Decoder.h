// Copyright (C) 2024 The Android Open Source Project
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

#pragma once

#include "host-common/GoldfishMediaDefs.h"
#include "host-common/MediaCodec.h"

#include <stddef.h>

namespace android {
namespace emulation {

class MediaAv1Decoder : public MediaCodec {
public:
    // Platform dependent
    static MediaAv1Decoder* create();
    virtual ~MediaAv1Decoder() = default;

    // For snapshots
    virtual void save(base::Stream* stream) const = 0;
    virtual bool load(base::Stream* stream) = 0;

protected:
    MediaAv1Decoder() = default;
};

}  // namespace emulation
}  // namespace android
