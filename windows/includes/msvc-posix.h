// Copyright 2018 The Android Open Source Project
//
// This software is licensed under the terms of the GNU General Public
// License version 2, as published by the Free Software Foundation, and
// may be copied, distributed, and modified under those terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#pragma once

#ifndef _AEMU_BITS_SOCKET_H_
#ifndef __linux__
#ifndef __QNX__
// Make sure these are defined and don't change anything if used.
enum {
    SOCK_CLOEXEC = 0,
#ifndef __APPLE__
    O_CLOEXEC = 0
#endif
};
#define _AEMU_BITS_SOCKET_H_
#endif  // !__QNX__
#endif  // !__linux__
#endif

#ifdef _MSC_VER

#include <windows.h>

#include <io.h>
#include <stdint.h>

#include "sys/cdefs.h"

__BEGIN_DECLS

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef fseeko
#define fseeko _fseeki64
#endif

#ifndef ftello
#define ftello _ftelli64
#endif

extern int asprintf(char** buf, const char* format, ...);
extern int vasprintf(char** buf, const char* format, va_list args);

__END_DECLS

#endif
