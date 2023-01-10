// Copyright 2022 The Android Open Source Project
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

#if __has_include("host-common/logging.h")
#include "host-common/logging.h"
#else
#define WARN(x) void(0)
#endif

#include <chrono>
#include <string>

namespace android {
namespace base {

using std::chrono::duration;
using std::chrono::steady_clock;
using std::chrono::time_point;

// A class for rate limiting arbitrary events
class RateLimiter {
   public:
    RateLimiter(std::string eventName, uint64_t maxUsesPerWindow,
                uint64_t windowDurationMs = 60'000)
        : mEventName(std::move(eventName)),
          mMaxUsesPerWindow(maxUsesPerWindow),
          mUsesInCurrentWindow(0),
          mWindowDuration(std::chrono::milliseconds(windowDurationMs)),
          mStartOfCurrentWindow(steady_clock::now()) {}

    bool use() {
        auto now = steady_clock::now();

        // Check if we need to start a new window.
        if (now - mWindowDuration > mStartOfCurrentWindow) {
            mStartOfCurrentWindow = now;
            if (mUsesInCurrentWindow > mMaxUsesPerWindow) {
                WARN("Previously dropped %u %s events", (mUsesInCurrentWindow - mMaxUsesPerWindow),
                     mEventName.c_str());
            }
            mUsesInCurrentWindow = 0;
        }

        mUsesInCurrentWindow++;
        if (mUsesInCurrentWindow == mMaxUsesPerWindow) {
            WARN("Reached %s events rate limiting threshold", mEventName.c_str());
        }
        return mUsesInCurrentWindow <= mMaxUsesPerWindow;
    }

   private:
    std::string mEventName;
    uint64_t mMaxUsesPerWindow;
    uint64_t mUsesInCurrentWindow;
    steady_clock::duration mWindowDuration;
    steady_clock::time_point mStartOfCurrentWindow;
};

}  // namespace base
}  // namespace android