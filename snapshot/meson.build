
aemu_snapshot_inc = [
    include_directories('include'),
    include_directories('../base/include'),
    include_directories('../host-common/include'),
]
aemu_snapshot_cargs = []
aemu_snapshot_src = [
    'TextureLoader.cpp',
    'TextureSaver.cpp',
]

if targetos == 'darwin'
    aemu_snapshot_cargs += ['Dfseeko64=fseek', '-Dftello64=ftell']
endif

aemu_snapshot_lib = static_library(
    'aemu-snapshot',
    aemu_snapshot_src,
    include_directories: aemu_snapshot_inc,
    dependencies: [lz4],
    c_args: aemu_snapshot_cargs,
    pic: true,
)
aemu_snapshot_dep = declare_dependency(
    include_directories: aemu_snapshot_inc,
    link_with: aemu_snapshot_lib,
    sources: aemu_snapshot_src,
)
