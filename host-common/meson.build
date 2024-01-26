aemu_logging_inc = include_directories(
    'include/host-common',
    'include',
    '../base/include',
)
aemu_logging_lib = static_library(
    'aemu-logging',
    ['logging.cpp', 'GfxstreamFatalError.cpp', '../base/Metrics.cpp'],
    include_directories: aemu_logging_inc,
    pic: true,
)


aemu_logging_dep = declare_dependency(
    include_directories: aemu_logging_inc,
    link_with: aemu_logging_lib,
)

prod_override_lib = static_library(
    'product-feature-override',
    ['FeatureControlOverride.cpp'],
    include_directories: aemu_logging_inc,
    pic: true,
)

prod_override_dep = declare_dependency(
    include_directories: aemu_logging_inc,
    link_with: prod_override_lib,
)


host_common_inc = [
    include_directories('include'),
    include_directories('include/host-common'),
]
host_common_src = [
    'AndroidPipe.cpp',
    'DmaMap.cpp',
    'GfxstreamFatalError.cpp',
    'GoldfishDma.cpp',
    'GoldfishSyncCommandQueue.cpp',
    'GraphicsAgentFactory.cpp',
    'HostmemIdMapping.cpp',
    'RefcountPipe.cpp',
    'address_space_device.cpp',
    'address_space_device_control_ops.cpp',
    'address_space_graphics.cpp',
    'address_space_host_media.cpp',
    'address_space_host_memory_allocator.cpp',
    'address_space_shared_slots_host_memory_allocator.cpp',
    'crash_reporter.cpp',
    'dma_device.cpp',
    'empty-crash-handler.cpp',
    'feature_control.cpp',
    'goldfish_sync.cpp',
    'hw-config.cpp',
    'logging.cpp',
    'misc.cpp',
    'sync_device.cpp',
    'vm_operations.cpp',
    'window_operations.cpp',
]
aemu_host_common_lib = static_library(
    'aemu-host-common',
    host_common_src,
    include_directories: host_common_inc,
    dependencies: [aemu_logging_dep, prod_override_dep],
    pic: true,
    c_args: [
        '-Wno-unused-variable',
        '-Wno-unused-private-field',
        '-Wno-unused-const-variable',
    ],
)
aemu_host_common_dep = declare_dependency(
    include_directories: aemu_logging_inc,
    link_with: aemu_host_common_lib,
)
