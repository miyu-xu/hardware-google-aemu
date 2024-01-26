cc_library(
    name = "logging",
    srcs = [
        "GfxstreamFatalError.cpp",
        "logging.cpp",
    ],
    hdrs = [
        "include/host-common/GfxstreamFatalError.h",
        "include/host-common/logging.h",
    ],
    includes = [
        "include",
        "include/host-common",
    ],
    visibility = ["//visibility:public"],
    deps = ["//hardware/google/aemu/base"],
)

cc_library(
    name = "headers",
    hdrs = glob(["include/host-common/*.h"]),
    includes = [
        "include",
        "include/host-common",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "host-common",
    srcs = [
        "AndroidPipe.cpp",
        "DmaMap.cpp",
        "FeatureControlOverride.cpp",
        "GfxstreamFatalError.cpp",
        "GoldfishDma.cpp",
        "GoldfishSyncCommandQueue.cpp",
        "GraphicsAgentFactory.cpp",
        "HostmemIdMapping.cpp",
        "RefcountPipe.cpp",
        "address_space_device.cpp",
        "address_space_device_control_ops.cpp",
        "address_space_graphics.cpp",
        "address_space_host_media.cpp",
        "address_space_host_memory_allocator.cpp",
        "address_space_shared_slots_host_memory_allocator.cpp",
        "crash_reporter.cpp",
        "dma_device.cpp",
        "empty-crash-handler.cpp",
        "feature_control.cpp",
        "goldfish_sync.cpp",
        "hw-config.cpp",
        "logging.cpp",
        "misc.cpp",
        "sync_device.cpp",
        "vm_operations.cpp",
        "window_operations.cpp",
    ] + glob([
        "include/**/*.h",
        "include/**/*.hpp",
    ]),
    includes = [
        "include",
        "include/host-common",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":logging",
        ":product-feature-override",
        "//hardware/google/aemu/base",
        "//hardware/google/aemu/base:headers",
    ],
)

cc_library(
    name = "product-feature-override",
    srcs = ["FeatureControlOverride.cpp"],
    visibility = ["//visibility:public"],
    deps = [
        ":headers",
        "//hardware/google/aemu/base:headers",
    ],
)
