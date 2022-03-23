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

    std::string GetCurrentProgramPath() {
        char buffer[MAX_PATH];

        int64_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (length == -1) {
            // An error has occurred, return empty string
            return {};
        }

        buffer[length] = '\0';

        return buffer;
    }

    std::string GetTimeStamp() {
        char timestamp_buffer[MAX_TIMESTAMP];
        time_t time          = std::time(nullptr);
        struct tm* time_info = std::localtime(&time);

        // Write current timestamp
        auto timestamp_length = static_cast<uint8_t>(std::strftime(
                timestamp_buffer,
                MAX_TIMESTAMP,
                timestamp_pattern,
                time_info
                ));

        // Check if the timestamp length did not exceed the TIMESTAMP_MAX
        if (timestamp_length == 0) {
            // The timestamp length has exceeded the TIMESTAMP_MAX, error
            return {};
        }

        // Count milliseconds
        auto now_time = std::chrono::system_clock::now();
        auto milliseconds =
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                        now_time);

        // Add milliseconds to the timestamp
        auto updated_timestamp = static_cast<int8_t>(std::snprintf(
                timestamp_buffer + timestamp_length,
                MAX_TIMESTAMP - timestamp_length,
                milliseconds_pattern,
                milliseconds.time_since_epoch().count() % 1000
                ));

        // Check if the timestamp length did not exceed its max value
        if (updated_timestamp < 0) {
            // The timestamp length has exceeded its max value, error
            return {};
        }

        return timestamp_buffer;
    }
} // end Utils