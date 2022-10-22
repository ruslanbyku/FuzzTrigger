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

#ifndef FUZZTRIGGER_VIRTUAL_MAPPER_H
#define FUZZTRIGGER_VIRTUAL_MAPPER_H

#include <unistd.h>
#include <sys/mman.h>
#include <cstdint>

class VirtualMapper {
public:
    explicit VirtualMapper();
    ~VirtualMapper();
    VirtualMapper(const VirtualMapper&)                = delete;
    VirtualMapper& operator=(const VirtualMapper&)     = delete;
    VirtualMapper(VirtualMapper&&) noexcept            = delete;
    VirtualMapper& operator=(VirtualMapper&&) noexcept = delete;

    // Check the mapping current status (if virtual memory has been allotted)
    explicit operator bool() const;

    bool        AllocateReadMap(int32_t, int64_t);
    void        Unmap();
    const char* GetMapping() const;

private:
    void*    mapping_;
    uint64_t size_;
    int32_t  protection_;
    int32_t  flags_;

    bool MapIntoMemory(int32_t, int32_t, int32_t, uint64_t,
                       uint64_t, void* page_address);

    // File offset must be a multiple of the page size as returned by
    // sysconf(_SC_PAGE_SIZE)
    bool IsFileOffsetValid(uint64_t);

};

#endif //FUZZTRIGGER_VIRTUAL_MAPPER_H
