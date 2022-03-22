#include "utils.h"

namespace Utils {
    uint32_t GenerateRandom() {
        std::random_device device;
        engine generator(device());
        std::uniform_int_distribution<engine::result_type>
                                                    distribution(1, UINT32_MAX);

        return distribution(generator);
    }

    std::string GenerateHash(uint32_t random) {
        std::string random_string(std::to_string(random));
        uint64_t hash = std::hash<std::string>{}(random_string);

        std::stringstream stream;
        stream << std::hex << hash;

        return stream.str();
    }

    std::string ReturnShortenedHash(uint32_t random, uint16_t length) {
        std::string hash = GenerateHash(random);

        if (hash.length() < length) {
            return hash;
        }

        return hash.substr(0, length);
    }
}