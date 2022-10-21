cmake_minimum_required(VERSION 3.18)

include(CMake/Version.cmake)

project(AEMUCommon
        DESCRIPTION "Android emulation utilities library"
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
        LANGUAGES CXX C)

set(AEMU_COMMON_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
# TODO(joshuaduong): There are still some header dependencies in vulkan-cereal/include. Need to
# decouple this if possible.
set(GFXSTREAM_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../device/generic/vulkan-cereal)

option(AEMU_COMMON_GEN_PKGCONFIG
       "Generate install files used by pkg-config."
       OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Set AEMU_BUILD_CONFIG_NAME to use a custom cmake build script located in
# build-config/$AEMU_BUILD_CONFIG_NAME. If AEMU_BUILD_CONFIG_NAME is unset,
# it will default to building everything. See build-config/gfxstream/CMakeLists.txt
# for an example.
if (NOT ${AEMU_COMMON_BUILD_CONFIG} STREQUAL "")
    if (NOT EXISTS "${AEMU_COMMON_REPO_ROOT}/build-config/${AEMU_COMMON_BUILD_CONFIG}")
        message(FATAL_ERROR "build-config/${AEMU_COMMON_BUILD_CONFIG} does not exist")
    endif()
    message(STATUS "aemu-common: Using custom build script in \
${AEMU_COMMON_REPO_ROOT}/build-config/${AEMU_COMMON_BUILD_CONFIG}")
    add_subdirectory(build-config/${AEMU_COMMON_BUILD_CONFIG})
endif()

add_subdirectory(base)
add_subdirectory(snapshot)
add_subdirectory(host-common)
add_subdirectory(third-party)

add_library(aemu_common INTERFACE)
target_link_libraries(
    aemu_common
    INTERFACE
    aemu-base
    aemu-base.headers
    aemu-host-common
    aemu-host-common.headers)

if(AEMU_COMMON_GEN_PKGCONFIG)
    configure_file(CMake/aemu_common.pc.in aemu_common.pc @ONLY)
    install(DIRECTORY base/include/aemu DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(DIRECTORY host-common/include/host-common DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS aemu-base aemu-host-common
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/aemu_common.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
