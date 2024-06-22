// Copyright (C) 2015 The Android Open Source Project
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

#include "aemu/base/async/Looper.h"
#include "aemu/base/Compiler.h"
#include "aemu/base/synchronization/Lock.h"

#include <functional>
#include <memory>

namespace android {
namespace base {

// A RecurrentTask is an object that allows you to run a task repeatedly on the
// event loop, until you're done.
// Example:
//
//     class AreWeThereYet {
//     public:
//         AreWeThereYet(Looper* looper) :
//                 mAskRepeatedly(looper,
//                                [this]() { return askAgain(); },
//                                1 * 60 * 1000) {}
//
//         bool askAgain() {
//             std::cout << "Are we there yet?" << std::endl;
//             return rand() % 2;
//         }
//
//         void startHike() {
//             mAskRepeatedly.start();
//         }
//
//     private:
//         RecurrentTask mAskRepeatedly;
//     };
//
// Note: RecurrentTask is meant to execute a task __on the looper thread__.
// It is thread safe though.
class RecurrentTask {
public:
    using TaskFunction = std::function<bool()>;

    RecurrentTask(Looper* looper,
                  TaskFunction function,
                  Looper::Duration taskIntervalMs)
        : mTaskIntervalMs(int(taskIntervalMs))
        , mFunction(function)
        , mTimer(looper->createTimer(&RecurrentTask::taskCallbackStatic, this)) {}

    ~RecurrentTask() { stopAndWait(); }

    void start(bool runImmediately = false) {
        AutoLock lock(mControlLock);
        mInFlight = true;
        mTimer->startRelative(runImmediately ? 0 : mTaskIntervalMs);
    }

    void stopAsync() {
        AutoLock lock(mControlLock);
        mInFlight = false;
        mTimer->stop();
    }

    void stopAndWait() {
        stopAsync();
        // TODO: this implementation is incomplete
    }

    bool inFlight() const {
        AutoLock lock(mControlLock);
        return mInFlight;
    }

    Looper::Duration taskIntervalMs() const { return mTaskIntervalMs; }

private:
    void taskCallback(Looper::Timer* timer) {
        if (inFlight()) {
            if (mFunction()) {
                AutoLock lock(mControlLock);
                if (mInFlight) {
                    mTimer->startRelative(mTaskIntervalMs);
                }
            } else {
                AutoLock lock(mControlLock);
                mInFlight = false;
            }
        }
    }

    static void taskCallbackStatic(void* opaqueThis, Looper::Timer* timer) {
        static_cast<RecurrentTask*>(opaqueThis)->taskCallback(timer);
    }

    // The order is important: mTimer must be destroyed first
    const int mTaskIntervalMs;
    bool mInFlight = false;
    mutable Lock mControlLock;
    const TaskFunction mFunction;
    const std::unique_ptr<Looper::Timer> mTimer;
};

}  // namespace base
}  // namespace android
