if(TARGET msvc-posix-compat)
  return()
endif()

if(INCLUDE_ANDROID_CMAKE)
  # This is a posix wrapper for windows-msvc build.
  android_nasm_compile(TARGET setjmp_asm_lib LICENSE Apache-2.0 SRC
                       src/setjmp.asm)
  android_add_library(TARGET msvc-posix-compat LICENSE Apache-2.0 SRC "")
  target_link_libraries(msvc-posix-compat PRIVATE setjmp_asm_lib)
else()
  add_library(msvc-posix-compat)
endif()

target_sources(
  msvc-posix-compat
  PRIVATE src/asprintf.c
          src/divti3.c
          src/files.cpp
          src/getopt.c
          src/int_endianness.h
          src/int_lib.h
          src/int_types.h
          src/int_util.h
          src/modti3.c
          src/msvc-posix.c
          src/pread.cpp
          src/time.cpp
          src/udivmodti4.c
          src/udivti3.c
          src/umodti3.c)
target_link_libraries(msvc-posix-compat PUBLIC aemu-base.headers)

# Msvc redefines macro's to inject compatibility.
target_compile_options(
  msvc-posix-compat
  PUBLIC "-Wno-macro-redefined" "-Wno-deprecated-declarations" # A lot of the
                                                               # POSIX names are
                                                               # deprecated..
)
target_include_directories(msvc-posix-compat PUBLIC includes)

add_subdirectory(tests)