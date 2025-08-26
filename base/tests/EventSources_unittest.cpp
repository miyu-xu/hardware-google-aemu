// Copyright (C) 2025 The Android Open Source Project
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
#include "aemu/base/events/EventSources.h"

#include <gtest/gtest.h>

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// A simple event type for testing.
struct TestEvent {
    int value;
    bool operator==(const TestEvent& other) const { return value == other.value; }
};

// A mock listener that records received events in a thread-safe manner.
template <class T>
class MockEventListener : public android::base::eventing::EventListener<T> {
public:
    void eventArrived(const T& event) override {
        std::lock_guard<std::mutex> lock(mMutex);
        received_events.push_back(event);
    }

    std::vector<T> received_events;
    std::mutex mMutex;
};

// A typed test suite to verify the common interface of all raw-pointer-based
// event sources. This reduces code duplication.
template <typename T>
class EventSourceTest : public ::testing::Test {};

using RawPointerSources = ::testing::Types<
    android::base::eventing::FastEventSource<TestEvent>,
    android::base::eventing::HighContentionEventSource<TestEvent>>;

TYPED_TEST_SUITE(EventSourceTest, RawPointerSources);

TYPED_TEST(EventSourceTest, AddAndFireEvent) {
    TypeParam source;
    MockEventListener<TestEvent> listener;
    TestEvent event{42};

    source.addListener(&listener);
    ASSERT_EQ(source.size(), 1);

    source.fireEvent(event);
    ASSERT_EQ(listener.received_events.size(), 1);
    EXPECT_EQ(listener.received_events[0], event);
}

TYPED_TEST(EventSourceTest, RemoveListener) {
    TypeParam source;
    MockEventListener<TestEvent> listener;
    TestEvent event{42};

    source.addListener(&listener);
    source.removeListener(&listener);
    ASSERT_EQ(source.size(), 0);

    source.fireEvent(event);
    EXPECT_TRUE(listener.received_events.empty());
}

TYPED_TEST(EventSourceTest, FireToMultipleListeners) {
    TypeParam source;
    MockEventListener<TestEvent> listener1, listener2;
    TestEvent event{42};

    source.addListener(&listener1);
    source.addListener(&listener2);
    ASSERT_EQ(source.size(), 2);

    source.fireEvent(event);
    ASSERT_EQ(listener1.received_events.size(), 1);
    EXPECT_EQ(listener1.received_events[0], event);
    ASSERT_EQ(listener2.received_events.size(), 1);
    EXPECT_EQ(listener2.received_events[0], event);
}

// A specific test to verify the basic thread safety of HighContentionEventSource.
TEST(HighContentionEventSourceTest, BasicThreadSafety) {
    android::base::eventing::HighContentionEventSource<TestEvent> source;
    MockEventListener<TestEvent> listener;
    source.addListener(&listener);

    constexpr int kNumThreads = 4;
    constexpr int kEventsPerThread = 100;

    std::vector<std::thread> threads;
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&source, i]() {
            for (int j = 0; j < kEventsPerThread; ++j) {
                source.fireEvent({i * 1000 + j});
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(listener.received_events.size(), kNumThreads * kEventsPerThread);
}

// Tests for SafeEventSource, focusing on its weak_ptr behavior.
TEST(SafeEventSourceTest, AddAndFireEvent) {
    android::base::eventing::SafeEventSource<TestEvent> source;
    auto listener = std::make_shared<MockEventListener<TestEvent>>();
    TestEvent event{42};

    source.addListener(listener);
    ASSERT_EQ(source.size(), 1);

    source.fireEvent(event);
    ASSERT_EQ(listener->received_events.size(), 1);
    EXPECT_EQ(listener->received_events[0], event);
}

TEST(SafeEventSourceTest, HandlesDestroyedListenerGracefully) {
    android::base::eventing::SafeEventSource<TestEvent> source;
    auto listener = std::make_shared<MockEventListener<TestEvent>>();
    TestEvent event{42};

    source.addListener(listener);
    ASSERT_EQ(source.size(), 1);

    // Destroy the listener object.
    listener.reset();

    // The source should correctly report that there are no more live listeners.
    ASSERT_EQ(source.size(), 0);

    // Firing the event should not crash and should clean up the expired pointer
    // internally.
    source.fireEvent(event);
    ASSERT_EQ(source.size(), 0);
}

// Tests for CallbackEventSource, focusing on the WithCallbacks API.
TEST(CallbackEventSourceTest, AddCallbackAndFire) {
    android::base::eventing::CallbackEventSource<TestEvent> source;
    std::vector<TestEvent> received_events;
    TestEvent event{42};

    auto handle = makeScopedCallback(
            source, [&](const TestEvent& e) { received_events.push_back(e); });

    ASSERT_EQ(source.size(), 1);

    source.fireEvent(event);
    ASSERT_EQ(received_events.size(), 1);
    EXPECT_EQ(received_events[0], event);
}

TEST(CallbackEventSourceTest, HandleUnsubscribesOnDestruction) {
    android::base::eventing::CallbackEventSource<TestEvent> source;
    std::vector<TestEvent> received_events;
    TestEvent event{42};

    {
        auto handle = makeScopedCallback(
                source, [&](const TestEvent& e) { received_events.push_back(e); });
        ASSERT_EQ(source.size(), 1);
    }  // The RAII handle goes out of scope here, unsubscribing the callback.

    ASSERT_EQ(source.size(), 0);

    source.fireEvent(event);
    EXPECT_TRUE(received_events.empty());
}