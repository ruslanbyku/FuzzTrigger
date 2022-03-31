#include "file.h"

File::File(std::string path)
: path_(std::move(path)), descriptor_(-1) {}

File::File(const File& file)
: path_(file.path_), descriptor_(file.descriptor_) {}

File& File::operator=(const File& file) {
    if (this == &file) {
        return *this;
    }

    path_       = file.path_;
    descriptor_ = file.descriptor_;

    return *this;
}

File::File(File&& file) noexcept : descriptor_(-1) {
    // Swap existing data and default initialized data
    std::swap(path_, file.path_);
    std::swap(descriptor_, file.descriptor_);
}

File& File::operator=(File&& file) noexcept {
    path_       = file.path_;
    descriptor_ = file.descriptor_;

    file.path_.clear();
    file.descriptor_ = -1;

    return *this;
}

File::operator bool() const noexcept {
    if (descriptor_ > -1) {
        return true; // Set
    }

    return false; // Unset
}

std::string File::GetPath() const noexcept {
    return path_.string();
}

bool File::IsAbsolute() const noexcept {
    if (!path_.empty()) {
        return path_.is_absolute();
    }

    return false;
}

void File::Close() noexcept {
    // Check if the file descriptor is set
    if (*this) {
        // The file descriptor is set, close it
        if (close(descriptor_) == -1) {
            // The file descriptor is somehow closed, be it so
        }

        // The file is closed, update the status of the descriptor
        descriptor_  = -1;
    }
}

void File::Delete() noexcept {
    Close();

    if (!path_.empty()) {
        std::error_code error_code;
        bool result = std::filesystem::remove(path_, error_code);

        if (!result) {
            // An error occurred when deleting a file, probably there is
            // already no such file, not a problem
        }

        // Update file path, zero it
        path_.clear();
    }
}

bool File::Exists() const noexcept {
    std::error_code error_code;
    bool result = std::filesystem::exists(path_, error_code);

    if (static_cast<bool>(error_code)) { // Error
        return false;
    }

    return result;
}

FileType File::GetFileType() const noexcept {
    std::error_code error_code;
    std::filesystem::file_type type =
            std::filesystem::status(path_, error_code).type();

    if (static_cast<bool>(error_code)) { // Error
        return FILETYPE_UNKNOWN;
    }

    switch (type) {
        default:
            return FILETYPE_UNKNOWN;
        case std::filesystem::file_type::regular:
            return FILETYPE_REGULAR;
        case std::filesystem::file_type::directory:
            return FILETYPE_DIRECTORY;

    }
}

std::string File::GetAbsolute() const noexcept {
    if (!path_.empty()) {
        if (IsAbsolute()) {
            return path_.string();
        }

        std::error_code error_code;
        std::string absolute(std::filesystem::absolute(path_, error_code));

        if (!static_cast<bool>(error_code)) { // Success
            return absolute;
        }
    }

    return {};
}

std::string File::GetParentPath() const noexcept {
    if (!path_.empty()) {
        if (path_.has_parent_path()) {
            return path_.parent_path().string();
        }
    }

    return {};
}

std::string File::GetName() const noexcept {
    if (!path_.empty()) {
        if (path_.has_filename()) {
            return path_.filename().string();
        }
    }

    return {};
}

std::string File::GetStem() const noexcept {
    if (!path_.empty()) {
        if (path_.has_stem()) {
            return path_.stem().string();
        }
    }

    return {};
}

std::string File::GetExtension() const noexcept {
    if (!path_.empty()) {
        if (path_.has_extension()) {
            return path_.extension().string();
        }
    }

    return {};
}

uint64_t File::GetSize() const noexcept {
    if (!path_.empty()) {
        std::error_code error_code;
        uint64_t size = std::filesystem::file_size(path_, error_code);

        if (!static_cast<bool>(error_code)) { // Success
            return size;
        }
    }

    return 0;
}

bool File::ReplaceName(const std::string& new_name) noexcept {
    if (!path_.empty()) {
        path_.replace_filename(new_name);

        return true;
    }

    return false;
}

bool File::ReplaceExtension(const std::string& extension) noexcept {
    if (!path_.empty()) {
        path_.replace_extension(extension);

        return true;
    }

    return false;
}

bool File::Copy(const std::string& new_path, bool override) const noexcept {
    if (!path_.empty()) {
        std::error_code error_code;
        bool result;

        if (override) {
            auto override_option =
                    std::filesystem::copy_options::overwrite_existing;
            result = std::filesystem::copy_file(path_, new_path,
                                                override_option, error_code);
        } else {
            result = std::filesystem::copy_file(path_, new_path, error_code);
        }

        if (!static_cast<bool>(error_code)) { // Success
            return result;
        }
    }

    return false;
}

int32_t File::OpenForReadOnly() {
    return Open(O_RDONLY, 0);
}

// If a file does not exist -> create
// If a file exists         -> truncate (override) the existing one
int32_t File::OpenForWrite() {
    int32_t flags = O_WRONLY | O_CREAT | O_TRUNC;

    return Create(flags);
}

// If a file does not exist -> create
// If a file exists         -> append to the existing one
int32_t File::OpenForAppend() {
    int32_t flags = O_WRONLY | O_CREAT | O_APPEND;

    return Create(flags);
}


int32_t File::Create(int32_t flags) {
    // 0644 mode bits for a regular file creation
    uint32_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    return Open(flags, mode);
}

int32_t File::Open(int32_t flags, uint32_t mode) {
    if (flags < 0) {
        return -1;
    }

    if (*this) {
        return descriptor_;
    }

    int32_t file_descriptor = open(path_.c_str(), flags, mode);

    if (file_descriptor == -1) {
        return -1;
    }

    descriptor_ = file_descriptor;

    return file_descriptor;
}

// If a directory does not exist -> create
// If a directory exists         -> do nothing
bool File::CreateDirectory() const noexcept {
    if (!path_.empty()) {
        std::error_code error_code;
        // Create all parent directories that do not exist
        bool result = std::filesystem::create_directories(path_, error_code);

        if (!static_cast<bool>(error_code)) { // Success
            return result;
        }
    }

    return false;
}

int64_t File::Write(const std::string& data, uint64_t size) const {
    // If the file descriptor is closed
    if (!*this) {
        return -1;
    }

    // If the size of data to write is zero
    if (size == 0) {
        return -1;
    }

    // Return number of data written to the file
    int64_t bytes = write(descriptor_, data.c_str(), size);

    return bytes;
}

