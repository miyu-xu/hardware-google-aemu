#ifndef BASE_TESTING_TESTUTILS_H_
#define BASE_TESTING_TESTUTILS_H_

#include <gmock/gmock.h>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <random>
#include <regex>
#include <sstream>
#include <string>

#include "base/Compiler.h"

// The original gtest MatchesRegex will use different regex implementation on different platforms,
// e.g. on Windows gtest's limited regex engine is used while on Linux Posix ERE is used, which
// could result incompatible syntaxes. See
// https://github.com/google/googletest/blob/main/docs/advanced.md#regular-expression-syntax for
// details. std::regex will by default use the ECMAScript syntax for all platforms, which could fix
// this issue.
MATCHER_P(MatchesStdRegex, regStr, std::string("contains regular expression: ") + regStr) {
    std::regex reg(regStr);
    return std::regex_search(arg, reg);
}

class ScopedTempFile {
   public:
    static std::unique_ptr<ScopedTempFile> create(const char* prefix, const std::string& content) {
        static std::random_device sDev;
        static std::mt19937 sRandomEngine(sDev());
        std::uniform_int_distribution<int> uniformDistribution(0, 10'000);
        std::stringstream ss;
        ss << prefix << "-" << std::setfill('0') << std::setw(4)
           << uniformDistribution(sRandomEngine);
        std::string filePath = ss.str();
        if (std::filesystem::exists(filePath)) {
            ADD_FAILURE() << "File " << filePath << " already exists.";
            return nullptr;
        }
        std::fstream stream(filePath, std::ios::binary | std::ios::out);
        if (stream.fail()) {
            ADD_FAILURE() << "Failed to open " << filePath << ".";
            return nullptr;
        }
        stream << content;
        if (stream.fail()) {
            ADD_FAILURE() << "Failed to write contents to " << filePath << ".";
            return nullptr;
        }
        return std::unique_ptr<ScopedTempFile>(new ScopedTempFile(std::move(filePath)));
    }
    ~ScopedTempFile() {
        if (std::filesystem::remove(mFilePath) != 1) {
            ADD_FAILURE() << "Failed to remove " << mFilePath << ".";
        }
    }
    const std::string mFilePath;

    DISALLOW_COPY_ASSIGN_AND_MOVE(ScopedTempFile);

   private:
    ScopedTempFile(std::string filePath) : mFilePath(std::move(filePath)) {}
};

#endif  // BASE_TESTING_TESTUTILS_H_
