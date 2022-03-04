#include "file.h"

File::File(std::string path)
: path_(std::move(path)), statistics_{}, descriptor_(-1) {}

File::~File() {
    Close();
}

File::operator bool() const {
    // Check the descriptor current status
    if (descriptor_ > -1) {
        return true; // Set
    }

    return false; // Unset
}

bool File::Exists() {
    return stat(path_.c_str(), &statistics_) == 0;
}

int32_t File::OpenForReadOnly() {
    return Open(O_RDONLY, 0);
}

int32_t File::Create() {
    int32_t flags = O_WRONLY | O_CREAT;
    // 0666 mode bits for a regular file creation
    // Given the default umask of 022, the resulting file will be 0644
    // 0666 & ~022 = 0644, i.e. rw-r--r--
    uint32_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    return Open(flags, mode);
}

void File::Close() {
    if (mapper_) {
        mapper_.Unmap();
    }

    // Check if the file descriptor is set
    if (*this) {
        // The file descriptor is set, close it
        if (close(descriptor_) == -1) {
            // The file descriptor is somehow closed, be it so
        }

        // The file is closed, update the status of the descriptor
        descriptor_  = -1;
        statistics_  = {};
    }
}

int32_t File::Open(int32_t flags, uint32_t mode) {
    if (flags < 0) {
        return -1;
    }

    int32_t file_descriptor = open(path_.c_str(), flags, mode);

    if (file_descriptor == -1) {
        fprintf(stderr, "open() failed on [%s]: %s\n",
                path_.c_str(),
                strerror(errno));

        return -1;
    }

    descriptor_ = file_descriptor;

    return file_descriptor;
}

bool File::IsCompilable() {
    std::filesystem::path file(path_);

    if (!file.has_extension()) {
        return false;
    }

    std::string file_extension(file.extension());

    bool found = std::ranges::any_of(
            cxx_extensions,
            [&file_extension](const char* const extension) {
                return file_extension == extension;
            });

    return found;
}

const std::string& File::GetPath() const {
    return path_;
}

const char* File::LoadIntoMemory(int32_t descriptor) {
    int64_t file_size = statistics_.st_size;

    bool result = mapper_.AllocateReadMap(descriptor, file_size);
    if (!result) {
        return nullptr;
    }

    return mapper_.GetMapping();
}
