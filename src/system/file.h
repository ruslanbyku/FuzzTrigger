#ifndef AUTOFUZZ_FILE_H
#define AUTOFUZZ_FILE_H

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <string>
#include <filesystem>

enum FileType: uint8_t {
    FILETYPE_UNKNOWN,
    FILETYPE_REGULAR,
    FILETYPE_DIRECTORY
};

// Additional resources for error handling:
// https://stackoverflow.com/a/48614155
// http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-2.html
//

class File {
public:
    explicit File(std::string);
    ~File()                          = default;
    File(const File&);
    File& operator=(const File&);
    File(File&&) noexcept;
    File& operator=(File&&) noexcept;

    // Check the descriptor current status (if the file is open)
    explicit operator bool() const noexcept;

    std::string GetPath() const noexcept;
    bool        Exists() const noexcept;
    void        Close() noexcept;
    void        Delete() noexcept;

    bool        IsAbsolute() const noexcept;
    FileType    GetFileType() const noexcept;
    std::string GetAbsolute() const noexcept;
    std::string GetParentPath() const noexcept;
    std::string GetName() const noexcept;
    std::string GetStem() const noexcept;
    std::string GetExtension() const noexcept;
    uint64_t    GetSize() const noexcept;
    bool        ReplaceName(const std::string&) noexcept;
    bool        ReplaceExtension(const std::string&) noexcept;
    bool        Copy(const std::string&, bool override = false) const noexcept;

    int32_t     OpenForReadOnly();
    int32_t     OpenForWrite();
    int32_t     OpenForAppend();

    bool        CreateDirectory() const noexcept;
    int64_t     Write(const std::string&, uint64_t) const;

private:
    std::filesystem::path path_;
    int32_t               descriptor_;

    int32_t Create(int32_t);
    int32_t Open(int32_t, uint32_t);
};

#endif //AUTOFUZZ_FILE_H
