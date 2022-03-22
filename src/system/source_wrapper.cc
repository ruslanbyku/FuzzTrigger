#include "source_wrapper.h"

SourceWrapper::SourceWrapper(std::string source_file_path, bool auto_deletion,
                             bool random_on)
: source_file_(std::move(source_file_path)),
working_directory_(source_file_.GetParentPath()),
module_dump_(std::make_unique<Module>()), ir_source_file_(source_file_),
auto_deletion_(auto_deletion), random_on_(random_on) {

    // Check if the file exists in the system
    if (!source_file_.Exists()) {
        std::string exception_message;

        exception_message += "File [";
        exception_message += source_file_.GetPath();
        exception_message += "] does not exist\n";

        throw std::runtime_error(exception_message);
    }

    // Check if the file is a source file and can be compiled
    if (!Compiler::IsCompilable(source_file_)) {
        std::string exception_message;

        exception_message += "File [";
        exception_message += source_file_.GetPath();
        exception_message += "] can not be compiled\n";

        throw std::runtime_error(exception_message);
    }

    // The source file is valid
    // Create a corresponding IR file for the source file
    ir_source_file_.ReplaceExtension(ir_extension);

    // Create a directory to store results
    ConstructResultDirectoryPath();
    bool result = CreateDirectory(result_directory_path_);
    if (!result) {
        std::string exception_message;

        exception_message += "Directory [";
        exception_message += result_directory_path_;
        exception_message += "] can not be created\n";

        throw std::runtime_error(exception_message);
    }
}

SourceWrapper::~SourceWrapper() {
    EmptyGarbage();
}

bool SourceWrapper::LaunchRoutine() {
    // Save a file for further deletion
    PlaceIntoGarbage(ir_source_file_);

    // Make analysis
    bool analysis_result = PerformAnalysis();
    if (!analysis_result) {
        return false;
    }

    // Analysis is ready, module_dump has been filled
    for (auto& function: module_dump_->functions_) {
        if (function->name_ != "sanitize_cookie_path") {
            continue;
        }

        bool generation_result = PerformGeneration(function);
        if (!generation_result) {
            // Do not know what to do so far
            return false;
        }
    }

    return true;
}

bool SourceWrapper::PerformAnalysis() {
    PassLauncher pass_on_source(ir_source_file_.GetPath());

    if (!pass_on_source.LaunchAnalysis(module_dump_)) {
        return false;
    }

    if (!*module_dump_) {
        return false;
    }

    return true;
}

bool SourceWrapper::PerformGeneration(
                              const std::unique_ptr<Function>& function_dump) {
    // Create a directory to store data about the function
    bool result;
    std::string function_directory_path;

    ConstructFunctionDirectoryPath(
            function_dump->name_, function_directory_path);
    result = CreateDirectory(function_directory_path);
    if (!result) {
        return false;
    }

    // Create a separate IR file for the function
    std::string ir_function_path;
    ir_function_path += function_directory_path;
    ir_function_path += function_dump->name_;
    ir_function_path += ir_extension;
    if (!ir_source_file_.Copy(ir_function_path)) {
        return false;
    }
    File ir_function_file(ir_function_path);

    // Sanitize the IR function file
    PassLauncher pass_on_function_ir(ir_function_path);
    pass_on_function_ir.LaunchSanitizer(function_dump);

    // Generate fuzzer_content stub content
    std::string fuzzer_content;
    std::string declaration("static char* sanitize_cookie_path(const char* cookie_path);");
    FuzzerGenerator fuzzer_generator(declaration, function_dump);
    fuzzer_generator.Generate();
    fuzzer_content = fuzzer_generator.GetFuzzer();

    // Create the fuzzer stub file and write fuzzer data to it
    std::string fuzzer_stub_path;
    ConstructFuzzerStubPath(function_dump->name_,
                            function_directory_path, fuzzer_stub_path);
    File fuzzer_stub_file(fuzzer_stub_path);
    result = WriteFuzzerContentToFile(fuzzer_stub_file, fuzzer_content);
    if (!result) {
        return false;
    }

    // Compile fuzzer stub file to IR
    if (!Compiler::IsCompilable(fuzzer_stub_file)) {
        fprintf(stderr, "File [%s] can not be compiled\n",
                fuzzer_stub_file.GetPath().c_str());

        return false;
    }
    File ir_fuzzer_stub_file(fuzzer_stub_path);
    ir_fuzzer_stub_file.ReplaceExtension(ir_extension);
    result = Compiler::CompileToIR(fuzzer_stub_file, ir_fuzzer_stub_file);
    if (!result) {
        return false;
    }

    // Modify IR fuzzer stub file (make suitable for separate compilation)
    PassLauncher pass_on_fuzzer(fuzzer_stub_path);
    pass_on_fuzzer.LaunchNameCorrector(function_dump);

    // Compile both IR
    std::string fuzzer_executable_path;
    ConstructFuzzerExecutablePath(function_directory_path,
                                  fuzzer_executable_path);
    File final_executable_file(fuzzer_executable_path);

    // Compile all bits and pieces
    Compiler::CompileToFuzzer(
            ir_function_file,
            ir_fuzzer_stub_file,
            final_executable_file
            );

    // Save files for further deletion
    PlaceIntoGarbage(ir_function_file);
    PlaceIntoGarbage(fuzzer_stub_file);
    PlaceIntoGarbage(ir_fuzzer_stub_file);

    return true;
}

void SourceWrapper::ConstructResultDirectoryPath() {
    result_directory_path_ += source_file_.GetParentPath();
    result_directory_path_ += "/";
    result_directory_path_ += source_file_.GetStem();
    result_directory_path_ += "_fuzz_results";

    if (random_on_) {
        result_directory_path_ += "_";
        result_directory_path_ +=
                Utils::ReturnShortenedHash(Utils::GenerateRandom());
    }

    result_directory_path_ += "/";
}

void SourceWrapper::ConstructFunctionDirectoryPath(
        const std::string& function_name, std::string path) {
    path += result_directory_path_;
    path += function_name;

    if (random_on_) {
        path += "_";
        path +=
                Utils::ReturnShortenedHash(Utils::GenerateRandom());
    }

    path += "/";
}

void SourceWrapper::ConstructFuzzerStubPath(
        const std::string& function_name,
        const std::string& parent_directory, std::string path) {
    path += parent_directory;
    path += "fuzz_";
    path += function_name;
    path += ".cc";
}

void SourceWrapper::ConstructFuzzerExecutablePath(
        const std::string& parent_directory, std::string path) {
    path += parent_directory;
    path += "fuzzer";
}


bool SourceWrapper::WriteFuzzerContentToFile(
        File& file, const std::string& fuzzer_content) {
    if (file.Exists()) {
        // So far do nothing
        return false;
    }

    int32_t file_descriptor = file.Create();
    if (file_descriptor == -1) {
        // So far do nothing
        return false;
    }

    int64_t bytes = file.Write(fuzzer_content, fuzzer_content.size());
    if (bytes == -1) {
        // So far do nothing
        return false;
    }

    file.Close();

    return true;
}

bool SourceWrapper::CreateDirectory(const std::string& path) {
    File directory(path);
    if (directory.Exists()) {
        return false;
    }
    directory.CreateDirectory();

    return true;
}

void SourceWrapper::PlaceIntoGarbage(File& path) {
    garbage_.push_back(path);
}

void SourceWrapper::EmptyGarbage() {
    if (auto_deletion_) {
        std::for_each(garbage_.begin(), garbage_.end(), [](File& file) {
            file.Delete();
        });
    }
}
