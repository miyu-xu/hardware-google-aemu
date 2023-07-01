# aemu-base overrides:
#   - aemu-base-srcs: aemu-base source files
#   - aemu-base-test-srcs: aemu-base test source files
set(BUILD_STANDALONE OFF PARENT_SCOPE)
set(GFXSTREAM_BASE_LIB aemu-vkcereal-base PARENT_SCOPE)
set(GFXSTREAM_HOST_COMMON_LIB aemu-vkcereal-host-common PARENT_SCOPE)
# aemu-host-common overrides:
#   - BUILD_HOST_COMMON_SUBSET: Enables aemu-host-common-subset library to be built, using a subset
#                               of host-common sources files (aemu-host-common-srcs).
#   - aemu-host-common-srcs: host-common source files
set(BUILD_HOST_COMMON_SUBSET ON PARENT_SCOPE)
set(aemu-host-common-srcs
    address_space_graphics.cpp PARENT_SCOPE)
add_library(lz4_static ALIAS lz4)
