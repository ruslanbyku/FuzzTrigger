#ifndef AUTOFUZZ_VIRTUAL_MAPPER_H
#define AUTOFUZZ_VIRTUAL_MAPPER_H

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
    inline bool IsFileOffsetValid(uint64_t offset) {
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

};

#endif //AUTOFUZZ_VIRTUAL_MAPPER_H
