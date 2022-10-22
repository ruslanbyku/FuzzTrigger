/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
