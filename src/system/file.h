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

class File {
public:
    explicit File(std::string);
    ~File()                          = default;
    File(const File&);
    File& operator=(const File&)     = delete;
    File(File&&) noexcept;
    File& operator=(File&&) noexcept = delete;

    // Check the descriptor current status (if the file is open)
    explicit operator bool() const;

    std::string GetPath() const;
    bool        Exists() const;
    void        Close();
    void        Delete();

    bool        IsAbsolute() const;
    FileType    GetFileType() const;
    std::string GetParentPath() const;
    std::string GetName() const;
    std::string GetStem() const;
    std::string GetExtension() const;
    uint64_t    GetSize() const;
    bool        ReplaceName(const std::string&);
    bool        ReplaceExtension(const std::string&);
    bool        Copy(const std::string&);

    int32_t     OpenForReadOnly();
    int32_t     Create();
    bool        CreateDirectory();
    int64_t     Write(const std::string&, uint64_t);

private:
    std::filesystem::path path_;
    int32_t               descriptor_;

    int32_t Open(int32_t, uint32_t);
};

#endif //AUTOFUZZ_FILE_H
