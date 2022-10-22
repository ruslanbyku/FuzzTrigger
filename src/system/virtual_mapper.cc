// Copyright 2022 Ruslan Byku
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "virtual_mapper.h"

VirtualMapper::VirtualMapper()
: mapping_(nullptr), size_(0), protection_(-1), flags_(-1) {}

VirtualMapper::~VirtualMapper() {
    Unmap();
}

VirtualMapper::operator bool() const {
    return mapping_ != nullptr;
}

bool VirtualMapper::AllocateReadMap(int32_t file_descriptor,
                                    int64_t file_size) {

    // Check if a file descriptor and file size are invalid + file size might
    // be 0 (might have been created recently) which means that it is in no
    // need of mapping
    if (file_descriptor < 0 || file_size <= 0) {
        return false;
    }

    int32_t protection = PROT_READ;
    int32_t flags      = MAP_PRIVATE;

    bool result = MapIntoMemory(protection, flags, file_descriptor,
                                static_cast<uint64_t>(file_size), 0, nullptr);
    if (!result) {
        return false;
    }

    return true;
}

void VirtualMapper::Unmap() {
    if (*this) {

        if (munmap(mapping_, size_) == -1) {
            // File content is not in memory anymore
        }

        mapping_    = nullptr;
        size_       = 0;
        protection_ = -1;
        flags_      = -1;
    }
}

const char* VirtualMapper::GetMapping() const {
    return static_cast<const char*>(mapping_);
}

bool VirtualMapper::MapIntoMemory(int32_t protection, int32_t flags,
                                  int32_t file_descriptor, uint64_t file_size,
                                  uint64_t file_offset, void* page_address) {
    // After the first file has been mapped into memory the internal state
    // of the memory object is set to the first file. You can not load
    // into memory the second file until you unmap the first one.
    if (mapping_) {
        return true;
    }

    void* memory;

    bool result = IsFileOffsetValid(file_offset);
    if (!result) {
        return false;
    }

    memory = mmap(page_address, file_size, protection,
                  flags, file_descriptor, static_cast<int64_t>(file_offset));
    if (memory == MAP_FAILED) {
        return false;
    }

    mapping_    = memory;
    size_       = file_size;
    protection_ = protection;
    flags_      = flags;

    return true;
}

bool VirtualMapper::IsFileOffsetValid(uint64_t offset) {
    int64_t page_size = sysconf(_SC_PAGE_SIZE);

    if (page_size == -1) {
        return false;
    }

    if (offset != 0 && (page_size % offset) != 0) {
        // File offset is not multiple
        return false;
    }

    return true;
}

