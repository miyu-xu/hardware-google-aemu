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
#include <chrono>
#include <cinttypes>
#include <cstdarg>
#include <cstring>
#include <sstream>
#include <thread>

#include "absl/log/log.h"
#include "logging.h"
namespace {
bool sEnableVerbose = false;
}  // namespace

void set_gfxstream_logger(gfxstream_logger_t f) {}
void set_gfxstream_fine_logger(gfxstream_logger_t f) {}
void set_gfxstream_enable_log_colors() {}
void set_gfxstream_enable_verbose_logs() { sEnableVerbose = true; }

static void GetLogSeverityAndVerbosity(char severity, absl::LogSeverity& logSeverity,
                                       int& verbosity) {
    switch (severity) {
        case 'V':
            logSeverity = absl::LogSeverity::kInfo;
            verbosity = -1;
            break;
        case 'I':
            logSeverity = absl::LogSeverity::kInfo;
            verbosity = 0;
            break;
        case 'W':
            logSeverity = absl::LogSeverity::kWarning;
            verbosity = 0;
            break;
        case 'E':
            logSeverity = absl::LogSeverity::kError;
            verbosity = 0;
            break;
        case 'F':
            logSeverity = absl::LogSeverity::kFatal;
            verbosity = 0;
            break;
        default:
            logSeverity = absl::LogSeverity::kWarning;
            verbosity = 0;
            LOG(WARNING) << "Gfxstream is using an unknown severity level: " << severity;
            break;
    }
}

static void LogMessageHelper(const char* file, unsigned int line, absl::LogSeverity severity,
                             int64_t timestamp_us, const char* buffer, int verbosity = 0) {
    absl::log_internal::LogMessage(file, line, severity)
            .WithTimestamp(absl::FromUnixMicros(timestamp_us))
            .WithVerbosity(verbosity)
        << buffer;
}

void OutputLog(FILE* stream, char severity, const char* file, unsigned int line,
               int64_t timestamp_us, const char* format, ...) {
    if (severity == 'V' && !sEnableVerbose) {
        return;
    }

    constexpr int bufferSize = 4096;
    char buffer[bufferSize];
    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
    if (size >= bufferSize) {}

    absl::LogSeverity logSeverity;
    int verbosity = 0;

    GetLogSeverityAndVerbosity(severity, logSeverity, verbosity);
    LogMessageHelper(file, line, logSeverity, timestamp_us, buffer, verbosity);
}
