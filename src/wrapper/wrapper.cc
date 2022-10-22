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

#include "wrapper.h"

Wrapper::Wrapper(Options options) : options_(options) {}

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

SourceEntity Wrapper::FindDeclarationsPerSource(
        const std::string& source_path,
        const StandaloneFunctions& standalone_functions) {

    SourceEntity source_entity;
    File source_file(source_path);

    // Open the file
    int32_t source_descriptor = source_file.OpenForReadOnly();
    if (source_descriptor == -1) {
        // Can not open file
        return source_entity;
    }

    // Get file size to mmap into memory
    auto source_file_size = static_cast<int32_t>(source_file.GetSize());

    if (source_file_size == 0) {
        // The file is empty
        return source_entity;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Open source file '"
                            << source_path << "':";
        LOG(LOG_LEVEL_INFO) << "\tfd   = " << source_descriptor;
        LOG(LOG_LEVEL_INFO) << "\tsize = " << source_file_size << " bytes";
    }

    // Load the file into memory
    VirtualMapper memory;
    if (!memory.AllocateReadMap(source_descriptor, source_file_size)) {
        // Can not load file into memory
        return source_entity;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Load source file content '"
                            << source_path << "' into memory.";
    }

    // Get file content
    std::string source_content = memory.GetMapping();

    if (source_content.empty()) {
        // The file is empty
        return source_entity;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Launch parser.";
    }

    // Launch source parser
    clang::tooling::runToolOnCode(
                       std::make_unique<FullSourceParser>(
                               standalone_functions,
                               source_entity),
                       source_content);

    if (LOGGER_ON) {
        uint64_t found_declarations = source_entity.size();

        if (found_declarations == 1) {
            LOG(LOG_LEVEL_INFO) << "1 declaration found in '"
                                << source_path << "'.";
        } else {
            LOG(LOG_LEVEL_INFO) << found_declarations
                                << " declarations found in '"
                                << source_path << "'.";
        }
    }

    // Explicitly unload the file from memory (the destructor will get
    // it anyway)
    memory.Unmap();
    // Close the file descriptor (the destructor will NOT get it)
    source_file.Close();

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Unload '" << source_path
                            << "' from memory.";
        LOG(LOG_LEVEL_INFO) << "Close fd = " << source_descriptor << ".";
    }

    return source_entity;
}

bool Wrapper::CreateDirectory(const std::string& directory_path,
                                    bool override) {
    File directory(directory_path);
    if (!override) {
        if (directory.Exists()) {
            // A directory exists
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_WARNING) << "Directory '"
                                       << directory_path
                                       << "' exists.";
            }

            return false;
        }
        // A directory does not exist. Then it is OK and create it.
    }

    if (!directory.CreateDirectory()) {
        // Something went wrong
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "Something went wrong. Directory '"
                                 << directory_path
                                 << "' was not created.";
        }

        return false;
    }

    // A directory successfully created
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Directory '" << directory_path
                            << "' created.";
    }

    return true;
}

