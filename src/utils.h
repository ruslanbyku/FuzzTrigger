#ifndef FUZZTRIGGER_UTILS_H
#define FUZZTRIGGER_UTILS_H

#include <unistd.h>
#include <random>
#include <sstream>
#include <chrono>

#define MAX_PATH          1024
#define SHORT_HASH_LENGTH 8
#define MAX_TIMESTAMP     128

const char* const timestamp_pattern    = "%H:%M:%S";
const char* const milliseconds_pattern = ".%03lu";

using engine = std::mt19937;

namespace Utils {
    // https://stackoverflow.com/a/13445752
    uint32_t GenerateRandom();
    std::string GenerateHash(uint32_t);
    std::string ReturnShortenedHash(uint32_t,
                                    uint16_t length = SHORT_HASH_LENGTH);
    std::string GetCurrentProgramPath();
    std::string GetTimeStamp();
}

#endif //FUZZTRIGGER_UTILS_H
