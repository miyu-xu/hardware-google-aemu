
aemu_base_cargs = [
    '-Wno-unused-but-set-variable',
    '-Wno-unused-variable',
    '-Wno-unused-const-variable',
]
aemu_base_link_args = []
aemu_base_src = [
    'AlignedBuf.cpp',
    'CLog.cpp',
    'CpuTime.cpp',
    'FileUtils.cpp',
    'FunctorThread.cpp',
    'GLObjectCounter.cpp',
    'HealthMonitor.cpp',
    'LayoutResolver.cpp',
    'MemStream.cpp',
    'StdioStream.cpp',
    'MemoryTracker.cpp',
    'MessageChannel.cpp',
    'PathUtils.cpp',
    'ring_buffer.cpp',
    'SharedLibrary.cpp',
    'StringFormat.cpp',
    'Stream.cpp',
    'StreamSerializing.cpp',
    'SubAllocator.cpp',
    'System.cpp',
    'Tracing.cpp',
    'CompressingStream.cpp',
    'DecompressingStream.cpp',
]

if targetos == 'windows'
    aemu_base_cargs += ['-DWIN32_LEAN_AND_MEAN']
    aemu_base_src += [
        'msvc.cpp',
        'SharedMemory_win32.cpp',
        'Thread_win32.cpp',
        'Win32UnicodeString.cpp',
    ]
    aemu_base_deps = [dependency('Shlwapi', required: true)]
endif
if targetos == 'darwin'
    aemu_base_src += [
        'SharedMemory_posix.cpp',
        'Thread_pthread.cpp',
        'system-native-mac.mm',
    ]
    aemu_base_deps = [
        dependency(
            'appleframeworks',
            modules: 'Foundation',
            required: true,
        ),
        dependency(
            'appleframeworks',
            modules: 'AppKit',
            required: true,
        ),
        dependency(
            'appleframeworks',
            modules: 'IOKit',
            required: true,
        ),
    ]
endif
if targetos == 'linux'
    aemu_base_src += ['SharedMemory_posix.cpp', 'Thread_pthread.cpp']
    aemu_base_deps = [dependency('dl', required: true)]
    aemu_base_link_args += ['-lrt']
endif

aemu_base_deps += [threads, aemu_host_common_dep, aemu_logging_dep, lz4]
aemu_base_inc = [include_directories('include'), include_directories('..')]
aemu_base = static_library(
    'aemu.base',
    aemu_base_src,
    include_directories: aemu_base_inc,
    link_args: aemu_base_link_args,
    c_args: aemu_base_cargs,
    dependencies: aemu_base_deps,
)
aemu_base_dep = declare_dependency(
    include_directories: aemu_base_inc,
    link_with: [aemu_base],
)
