objc_library(
    name = "base-darwin",
    srcs = ["system-native-mac.mm"],
)

cc_library(
    name = "headers",
    hdrs = glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

# Note that this is the gxfstream variant.

cc_library(
    name = "base",
    srcs = [
        "AlignedBuf.cpp",
        # "CompressingStream.cpp",
        "CLog.cpp",
        "CpuTime.cpp",
        # "DecompressingStream.cpp",
        "FileUtils.cpp",
        "FunctorThread.cpp",
        "GLObjectCounter.cpp",
        "HealthMonitor.cpp",
        "LayoutResolver.cpp",
        "MemStream.cpp",
        "StdioStream.cpp",
        "MemoryTracker.cpp",
        "MessageChannel.cpp",
        "Metrics.cpp",
        "PathUtils.cpp",
        "ring_buffer.cpp",
        "SharedLibrary.cpp",
        "StringFormat.cpp",
        "Stream.cpp",
        "StreamSerializing.cpp",
        "SubAllocator.cpp",
        "System.cpp",
        "Tracing.cpp",
    ] + select(
        {
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
            "//conditions:default": [],
        },
    ),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = ["include"],
    linkopts = select(
        {
            "@platforms//os:windows": [
            ],
            "@platforms//os:macos": [
                "-pthread",
            ],
            "@platforms//os:linux": [
                "-pthread",
            ],
            "//conditions:default": [],
        },
    ),
    visibility = ["//visibility:public"],
    deps = ["//hardware/google/aemu/host-common:headers"] +
           select({
               "@platforms//os:macos": [
                   "base-darwin",
               ],
               "//conditions:default": [],
           }),
)

# TODO(jansene): Add lz4 support?
