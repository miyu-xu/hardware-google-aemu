set(AEMU_COMMON_REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

function(aemu_generate_flatbuffers_cpp_headers)
    set(oneValueArgs TARGET SCHEMA)
    cmake_parse_arguments(
        PARSE_ARGV 0 AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS ""
        "${oneValueArgs}" "")
    set(generated_target_dir
        "${CMAKE_CURRENT_BINARY_DIR}/${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_TARGET}")
    cmake_path(GET AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_SCHEMA STEM LAST_ONLY filename)
    set(generated_header "${generated_target_dir}/${filename}_generated.h")
    set(FLATC_TARGET "")
    if(TARGET flatc)
        set(FLATC_TARGET flatc)
    endif()
    add_custom_command(
        OUTPUT ${generated_header}
        COMMAND flatc
        ARGS
        --cpp --gen-object-api --scoped-enums --gen-mutable
        -o "${generated_target_dir}"
        "${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_SCHEMA}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${FLATC_TARGET} "${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_SCHEMA}"
        VERBATIM
    )
    add_library(${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_TARGET} INTERFACE)
    target_include_directories(
        ${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_TARGET}
        INTERFACE "${generated_target_dir}")
    target_sources(
        ${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_TARGET}
        INTERFACE ${generated_header})
    add_dependencies(
        ${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_TARGET}
        "${AEMU_GENERATE_FLATBUFFERS_CPP_HEADERS_SCHEMA}"
        ${FLATC_TARGET}
    )
endfunction()

add_subdirectory(base)
add_subdirectory(snapshot)
add_subdirectory(host-common)

add_library(aemu_common INTERFACE)
target_link_libraries(
    aemu_common
    INTERFACE
    gfxstream-base
    gfxstream-base.headers
    gfxstream-host-common
    gfxstream-host-common.headers)
