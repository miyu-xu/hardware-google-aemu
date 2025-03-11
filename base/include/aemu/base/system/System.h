#pragma once

#include "aemu/base/CpuTime.h"

#include <string>

namespace android {
namespace base {

std::string getEnvironmentVariable(const std::string& key);
void setEnvironmentVariable(const std::string& key, const std::string& value);

uint64_t getUnixTimeUs();
uint64_t getHighResTimeUs();

uint64_t getUptimeMs();

std::string getProgramDirectory();
std::string getLauncherDirectory();

bool getFileSize(int fd, uint64_t* size);

void sleepMs(uint64_t ms);
void sleepUs(uint64_t us);
// Sleep to the specified time in microseconds from getHighResTimeUs().
void sleepToUs(uint64_t us);

CpuTime cpuTime();

bool queryFileVersionInfo(const char* filename, int* major, int* minor, int* build1, int* build2);

int getCpuCoreCount();

// Return the program bitness as an integer, either 32 or 64.
#if defined(__x86_64__) || defined(__aarch64__)
    static const int kProgramBitness = 64;
#else
    static const int kProgramBitness = 32;
#endif

inline int getProgramBitness() { return kProgramBitness; }

bool isRemoteSession(std::string* sessionType);

} // namespace base
} // namespace android
