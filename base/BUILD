# Interface library
cc_library(
    name = "aemu-base-headers",
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
    deps = ["//hardware/google/aemu/host-common:aemu-host-common-headers"],
)

cc_library(
    name = "aemu-base-metrics",
    srcs = ["Metrics.cpp"],
    visibility = ["//visibility:public"],
    deps = [":aemu-base-headers"],
)


cc_library(
    name = "aemu-base-allocator",
    srcs = ["SubAllocator.cpp"],
    visibility = ["//visibility:public"],
    deps = [":aemu-base-headers"],
)

objc_library(
    name = "aemu-base-darwin",
    srcs = [
        "system-native-mac.mm",
    ],
    deps = [":aemu-base-headers"],
)

cc_library(
    name = "aemu-base",
    srcs = [
        "AlignedBuf.cpp",
        "CLog.cpp",
        "CompressingStream.cpp",
        "CpuTime.cpp",
        "DecompressingStream.cpp",
        "FileUtils.cpp",
        "FunctorThread.cpp",
        "GLObjectCounter.cpp",
        "HealthMonitor.cpp",
        "LayoutResolver.cpp",
        "MemStream.cpp",
        "MemoryTracker.cpp",
        "MessageChannel.cpp",
        "PathUtils.cpp",
        "SharedLibrary.cpp",
        "StdioStream.cpp",
        "Stream.cpp",
        "StreamSerializing.cpp",
        "StringFormat.cpp",
        "SubAllocator.cpp",
        "System.cpp",
        "Tracing.cpp",
        "ring_buffer.cpp",
    ] + select({
        "@platforms//os:windows": [
            "SharedMemory_win32.cpp",
            "Thread_win32.cpp",
            "Win32UnicodeString.cpp",
            "msvc.cpp",
        ],
        "@platforms//os:macos": [
            "SharedMemory_posix.cpp",
            "Thread_pthread.cpp",
        ],
        "@platforms//os:linux": [
            "SharedMemory_posix.cpp",
            "Thread_pthread.cpp",
        ],
    }),
    deps = [
        ":aemu-base-headers",
        "//external/lz4",
    ] + select({
        "@platforms//os:macos": [
            ":aemu-base-darwin",
        ],
        "//conditions:default": [],
    }),
)

cc_test(
    name = "aemu-base_unittests",
    srcs = [
        "AlignedBuf_unittest.cpp",
        "ArraySize_unittest.cpp",
        "HealthMonitor_unittest.cpp",
        "HybridEntityManager_unittest.cpp",
        "LayoutResolver_unittest.cpp",
        "LruCache_unittest.cpp",
        "ManagedDescriptor_unittest.cpp",
        "Optional_unittest.cpp",
        "StringFormat_unittest.cpp",
        "SubAllocator_unittest.cpp",
        "TypeTraits_unittest.cpp",
        "WorkerThread_unittest.cpp",
        "ring_buffer_unittest.cpp",
        "testing/file_io.cpp",
    ] + select({
        "@platforms//os:windows": ["Win32UnicodeString_unittest.cpp"],
        "//conditions:default": [],
    }),
    deps = [
        ":aemu-base",
        ":aemu-base-headers",
        "//hardware/google/aemu/host-common:logging",
        "@com_google_googletest//:gtest_main",
    ],
)
