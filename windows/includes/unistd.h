// Copyright 2021 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// A minimal set of functions found in unistd.h
#if !defined(_AEMU_UNISTD_H_) && !defined(_MSVC_UNISTD_H)
#define _AEMU_UNISTD_H_
#define _MSVC_UNISTD_H

#include "compat_compiler.h"
#include <stddef.h>
#include <stdint.h>
#include <process.h>

ANDROID_BEGIN_HEADER

#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>

typedef long long ssize_t;
typedef long off_t;
typedef int64_t off64_t;
typedef int mode_t;

typedef char assert_sizeof_ssize_t[(sizeof(ssize_t) == sizeof(size_t)) ? 1 : -1];

#undef fstat
#define fstat _fstat64

#define lseek(a, b, c) _lseek(a, b, c)
#define lseek64 _lseeki64

// Define for convenience only in mingw. This is
// convenient for the _access function in Windows.
#if !defined(F_OK)
#define F_OK 0 /* Check for file existence */
#endif
#if !defined(X_OK)
#define X_OK 1 /* Check for execute permission (not supported in Windows) */
#endif
#if !defined(W_OK)
#define W_OK 2 /* Check for write permission */
#endif
#if !defined(R_OK)
#define R_OK 4 /* Check for read permission */
#endif

#define STDIN_FILENO _fileno(stdin)
#define STDOUT_FILENO _fileno(stdout)
#define STDERR_FILENO _fileno(stderr)
ssize_t pread(int fd, void *buf, size_t count, off_t offset);

int usleep(long usec);
unsigned int sleep(unsigned int seconds);

// Qemu will redefine this if it can.
int _ftruncate(int fd, off_t length);
#define ftruncate _ftruncate


#define __try1(x) __try
#define __except1 __except (EXCEPTION_EXECUTE_HANDLER)

ANDROID_END_HEADER
#endif	/* Not _AEMU_UNISTD_H_ */
