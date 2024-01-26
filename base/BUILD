objc_library(
    name = "base-darwin",
    srcs = ["system-native-mac.mm"] + glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = ["include"],
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
        "Metrics.cpp",
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
    defines = select(
        {
            "@platforms//os:windows": [
                "-DWIN32_LEAN_AND_MEAN",
            ],
            "@platforms//os:macos": [
            ],
            "@platforms//os:linux": [],
            "//conditions:default": [],
        },
    ),
    includes = ["include"],
    linkopts = select(
        {
            "@platforms//os:windows": [
            ],
            "@platforms//os:macos": [
            ],
            "@platforms//os:linux": [
                "-ldl",
                "-lrt",
            ],
            "//conditions:default": [],
        },
    ),
    visibility = ["//visibility:public"],
    deps = [
        "//external/lz4",
        "//hardware/google/aemu/host-common:headers",
    ] + select({
        "@platforms//os:macos": [
            "base-darwin",
        ],
        "//conditions:default": [],
    }),
)

# TODO(jansene): Add lz4 support?
