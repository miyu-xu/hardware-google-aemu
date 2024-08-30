// Copyright 2023 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "aemu/base/windows/includes/bits/socket.h"

#ifdef _MSC_VER

#include <windows.h>

// #include <direct.h>
// #include <fcntl.h>
#include <io.h>
#include <stdint.h>
#include "aemu/base/windows/includes/sys/cdefs.h"

__BEGIN_DECLS

#ifndef AEMU_WIN_COMPAT
#include "aemu/base/windows/includes/unistd.h"
#include "aemu/base/windows/includes/sys/types.h"
#include "aemu/base/windows/includes/fcntl.h"
#include "aemu/base/windows/includes/sys/cdefs.h"
#include "aemu/base/windows/includes/limits.h"
#include "aemu/base/windows/includes/sys/time.h"
#include "aemu/base/windows/includes/sys/stat.h"
#include "aemu/base/windows/includes/strings.h"
#include "aemu/base/windows/includes/stdlib.h"
#else
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/cdefs.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdlib.h>
#endif


#ifndef fseeko
#define fseeko _fseeki64
#endif

#ifndef ftello
#define ftello _ftelli64
#endif


extern SystemTime getSystemTime;
extern int asprintf(char** buf, const char* format, ...);
extern int vasprintf(char** buf, const char* format, va_list args);

__END_DECLS

#endif