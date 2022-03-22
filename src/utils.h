#ifndef AUTOFUZZ_UTILS_H
#define AUTOFUZZ_UTILS_H

#include <random>
#include <sstream>

#define SHORT_HASH_LENGTH 8

using engine = std::mt19937;

namespace Utils {
    // https://stackoverflow.com/a/13445752
    uint32_t GenerateRandom();
    std::string GenerateHash(uint32_t);
    std::string ReturnShortenedHash(uint32_t,
                                    uint16_t length = SHORT_HASH_LENGTH);
}

#endif //AUTOFUZZ_UTILS_H
