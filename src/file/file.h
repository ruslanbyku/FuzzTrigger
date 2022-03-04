#ifndef AUTOFUZZ_FILE_H
#define AUTOFUZZ_FILE_H

#include "virtual_mapper.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <filesystem>
#include <algorithm>
#include <array>

const std::array<const char* const, 8> cxx_extensions = {".C", ".c",
                                                         ".cc", ".cxx",
                                                         ".cpp", ".CPP",
                                                         ".c++", ".cp"};

class File {
public:
    explicit File(std::string);
    ~File();
    File(const File&)                = delete;
    File& operator=(const File&)     = delete;
    File(File&&) noexcept            = delete;
    File& operator=(File&&) noexcept = delete;

    explicit operator bool() const;

    bool    Exists();
    int32_t OpenForReadOnly();
    int32_t Create();
    void    Close();

    bool    IsCompilable();

    const char* LoadIntoMemory(int32_t);

    const std::string& GetPath() const;

private:
    std::string   path_;
    struct stat   statistics_;
    int32_t       descriptor_;

    VirtualMapper mapper_;

    int32_t Open(int32_t, uint32_t);
};


#endif //AUTOFUZZ_FILE_H
