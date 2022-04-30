#include "wrapper.h"

bool Wrapper::CreateDirectory(const std::string& path, bool is_override) {
    File directory(path);

    if (!is_override) {
        if (directory.Exists()) {
            return false;
        }
    }

    directory.CreateDirectory();

    return true;
}

std::string Wrapper::ConstructResultDirectoryPath(
        const std::string& working_directory,
        const File& source_file,
        bool is_random) {
    std::string result_directory_path;

    if (working_directory.empty()) {
        return result_directory_path;
    }

    result_directory_path += working_directory;
    result_directory_path += "/";
    result_directory_path += source_file.GetStem();
    result_directory_path += "_fuzz_results";

    if (is_random) {
        result_directory_path += "_";
        result_directory_path +=
                Utils::ReturnShortenedHash(Utils::GenerateRandom());
    }

    result_directory_path += "/";

    return result_directory_path;
}

std::string Wrapper::ConstructFunctionDirectoryPath(
        const std::string& result_directory_path,
        const std::string& function_name,
        bool is_random) {
    std::string function_directory_path;

    if (result_directory_path.empty() || function_name.empty()) {
        return function_directory_path;
    }

    function_directory_path += result_directory_path;
    function_directory_path += function_name;

    if (is_random) {
        function_directory_path += "_";
        function_directory_path +=
                Utils::ReturnShortenedHash(Utils::GenerateRandom());
    }

    function_directory_path += "/";

    return function_directory_path;
}

std::string Wrapper::ConstructFuzzerStubPath(
        const std::string& parent_directory,
        const std::string& function_name) {
    std::string fuzzer_stub_path;

    if (parent_directory.empty() || function_name.empty()) {
        return fuzzer_stub_path;
    }

    fuzzer_stub_path += parent_directory;
    fuzzer_stub_path += "fuzz_";
    fuzzer_stub_path += function_name;
    fuzzer_stub_path += ".cc";

    return fuzzer_stub_path;
}

std::string Wrapper::ConstructFuzzerExecutablePath(
        const std::string& parent_directory) {
    std::string fuzzer_executable_path;

    if (parent_directory.empty()) {
        return fuzzer_executable_path;
    }

    fuzzer_executable_path += parent_directory;
    fuzzer_executable_path += "fuzzer";

    return fuzzer_executable_path;
}


bool Wrapper::WriteFuzzerContentToFile(
        File& file, const std::string& fuzzer_content, bool is_override) {

    if (fuzzer_content.empty()) {
        return false;
    }

    if (!is_override) {
        if (file.Exists()) {
            // Trying overriding the existing file without due permissions on
            // the action
            return false;
        }
    }

    int32_t file_descriptor = file.OpenForWrite();
    if (file_descriptor == -1) {
        // Could not create file for fuzzer content
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << file.GetPath() << "' created.";
    }

    int64_t bytes = file.Write(fuzzer_content, fuzzer_content.size());
    if (bytes == -1) {
        // No bytes were written to the created file
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Fuzzer data has been written to '"
                            << file.GetPath() << "'.";
    }

    file.Close();

    return true;
}

void Wrapper::PlaceIntoGarbage(File& file) {
    garbage_.push_back(file);
}

void Wrapper::EmptyGarbage(bool auto_deletion) {
    if (auto_deletion) {
        std::for_each(garbage_.begin(), garbage_.end(), [](File& file) {

            if (!file.Exists()) {
                return;
            }

            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "Delete '" << file.GetPath() << "'.";
            }

            file.Delete();
        });
    }
}