#include "source_wrapper.h"

SourceWrapper::SourceWrapper(std::string source_file_path, bool auto_deletion,
                             bool random_on, bool override)
: source_file_(std::move(source_file_path)),
  ir_source_file_(source_file_),
  working_directory_(source_file_.GetParentPath()),
  module_dump_(std::make_unique<Module>()),
  auto_deletion_(auto_deletion),
  random_on_(random_on),
  override_(override) {
    InitializeState();
}

SourceWrapper::~SourceWrapper() {
    EmptyGarbage();
}

void SourceWrapper::InitializeState() {
    if (!source_file_.IsAbsolute()) {
        std::string exception_message;

        exception_message += "File path of '";
        exception_message += source_file_.GetPath();
        exception_message += "' is relative.";

        throw std::runtime_error(exception_message);
    }

    // Check if the file exists in the system
    if (!source_file_.Exists()) {
        std::string exception_message;

        exception_message += "File '";
        exception_message += source_file_.GetPath();
        exception_message += "' does not exist.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << source_file_.GetPath() << "' found.";
        //
        LOG(LOG_LEVEL_INFO) << "Current working directory '"
                            << working_directory_ << "'.";
        //
        LOG(LOG_LEVEL_INFO) << "Check whether '" << source_file_.GetPath()
                            << "' can be compiled.";
    }

    // Check if the file is a source file and can be compiled
    if (!Compiler::IsCompilable(source_file_)) {
        std::string exception_message;

        exception_message += "File '";
        exception_message += source_file_.GetPath();
        exception_message += "' can not be compiled.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << source_file_.GetPath()
                            << "' can be compiled.";
    }

    // The source file is valid
    // Create a corresponding IR file for the source file
    // file_name[.c] -> file_name[.ll]
    ir_source_file_.ReplaceExtension(ir_extension);

    if (LOGGER_ON) {
        std::string random_parameter   = random_on_      ? "true" : "false";
        std::string deletion_parameter = auto_deletion_  ? "true" : "false";
        std::string override_parameter = override_       ? "true" : "false";

        LOG(LOG_LEVEL_INFO) << "Additional parameters are used:";
        LOG(LOG_LEVEL_INFO) << "file name randomization = " << random_parameter;
        LOG(LOG_LEVEL_INFO) << "tmp file auto deletion  = "
                            << deletion_parameter;
        LOG(LOG_LEVEL_INFO) << "file override           = "
                            << override_parameter;
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Environment configuration is done. "
                               "Ready to proceed.";
    }
}

bool SourceWrapper::LaunchRoutine() {
    // --------------------------------------------------------------------- //
    //                        Compiling source to IR                         //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Compiling '" << source_file_.GetPath()
                            << "' to IR.";
    }
    bool compile_result = Compiler::CompileToIR(source_file_, ir_source_file_);
    if (!compile_result) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "An error occurred when compiling '"
                                 << ir_source_file_.GetPath()
                                 << "' to IR.";
        }

        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_source_file_.GetPath()
                            << "' created.";
    }

    // Save a file for further deletion
    PlaceIntoGarbage(ir_source_file_);

    // --------------------------------------------------------------------- //
    //                           Start analysis                              //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Start analysis of '"
                            << ir_source_file_.GetPath() << "'.";
    }

    // Make analysis
    bool analysis_result = PerformAnalysis();
    if (!analysis_result) {
        // Analysis resulted unsuccessful
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Analysis went successful.";
    }

    // --------------------------------------------------------------------- //
    //               Find number of found standalone functions               //
    // --------------------------------------------------------------------- //
    if (module_dump_->standalone_funcs_number_ == 0) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "There are no standalone functions. Abort.";
        }

        return true;
    } else {
        if (LOGGER_ON) {
            if (module_dump_->standalone_funcs_number_ == 1) {
                LOG(LOG_LEVEL_INFO) << "There is 1 standalone function:";
            } else {
                LOG(LOG_LEVEL_INFO) << "There are "
                                    << module_dump_->standalone_funcs_number_
                                    << " standalone functions:";
            }
        }
    }

    if (LOGGER_ON) {
        for (const auto& function: module_dump_->functions_) {
            if (!function->is_standalone_) {
                continue;
            }

            LOG(LOG_LEVEL_INFO) << "\t" << function->name_;
        }

    }

    // --------------------------------------------------------------------- //
    //   Open the source file in READ mode and load its content into memory  //
    // --------------------------------------------------------------------- //
    int32_t source_descriptor = source_file_.OpenForReadOnly();
    if (source_descriptor == -1) {
        // Can not open file
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Open source file '"
                            << source_file_.GetPath() << "' with fd = "
                            << source_descriptor << ".";
    }

    auto source_file_size = static_cast<int32_t>(source_file_.GetSize());
    bool mapper_result = memory_.AllocateReadMap(
            source_descriptor,
            source_file_size
            );
    if (!mapper_result) {
        // Can not load file into memory
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Load source file content '"
                            << source_file_.GetPath() << "' into memory.";
    }

    // --------------------------------------------------------------------- //
    //             Create a directory to store program results               //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Create a directory to store program results.";
    }

    // Create a directory to store results
    ConstructResultDirectoryPath();
    bool directory_create_result = CreateDirectory(result_directory_path_);
    if (!directory_create_result) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "Directory '"
                                 << result_directory_path_
                                 << "' was not created.";
        }

        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Directory '" << result_directory_path_
                            << "' created.";
        LOG(LOG_LEVEL_INFO) << "Start fuzzer generation process.";
    }

    // --------------------------------------------------------------------- //
    //                            Start generation                           //
    // --------------------------------------------------------------------- //
    uint64_t successful_fuzzer_counter = 0;
    // Analysis is ready, module_dump has been filled
    for (auto& function: module_dump_->functions_) {
        if (!function->is_standalone_) {
            continue;
        }

        bool generation_result = PerformGeneration(function);
        if (!generation_result) {
            // 1) Something bad has happened
            // 2) No error occurred, just could not continue to
            // generate a fuzzer
            continue;
        }

        ++successful_fuzzer_counter;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "End of fuzzer generation process.";
        if (successful_fuzzer_counter == 1) {
            LOG(LOG_LEVEL_INFO) << "1 fuzzer generated.";
        } else {
            LOG(LOG_LEVEL_INFO) << successful_fuzzer_counter
                                << " fuzzers generated.";
        }
    }

    // --------------------------------------------------------------------- //
    //         Unload the source from memory and close the file              //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Unload '" << source_file_.GetPath()
                            << "' from memory.";
        LOG(LOG_LEVEL_INFO) << "Close fd = " << source_descriptor << ".";
    }

    memory_.Unmap();
    source_file_.Close();

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
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Generate fuzzer for '"
                            << function_dump->name_ << "'.";
    }

    // --------------------------------------------------------------------- //
    //     Check whether a standalone function is qualified to be fuzzed     //
    // --------------------------------------------------------------------- //
    // If a standalone function has no argument for input, there is no point in
    // making a fuzzer for it
    if (function_dump->arguments_number_ == 0) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Function '"
                                << function_dump->name_
                                << "' has no input arguments. Abort.";
        }

        return false;
    }

    // --------------------------------------------------------------------- //
    //        Create a directory to store data about the function            //
    // --------------------------------------------------------------------- //
    //
    bool result;
    std::string function_directory_path;

    ConstructFunctionDirectoryPath(
            function_dump->name_, function_directory_path);
    result = CreateDirectory(function_directory_path);
    if (!result) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Directory '" << function_directory_path
                                            << "' created.";
    }

    // --------------------------------------------------------------------- //
    //             Create a separate IR file for the function                //
    // --------------------------------------------------------------------- //
    std::string ir_function_path;
    ir_function_path += function_directory_path;
    ir_function_path += function_dump->name_;
    ir_function_path += ir_extension;

    if (!ir_source_file_.Copy(ir_function_path, override_)) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_function_path << "' created.";
    }

    File ir_function_file(ir_function_path);
    // Put the file into the garbage right away after the creation
    PlaceIntoGarbage(ir_function_file);

    // --------------------------------------------------------------------- //
    //                     Sanitize the IR function file                     //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Launch sanitization for '"
                            << ir_function_path << "'.";
    }

    PassLauncher pass_on_function_ir(ir_function_path);
    result = pass_on_function_ir.LaunchSanitizer(function_dump);
    if (!result) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_function_path
                                       << "' has been sanitized.";
    }

    // --------------------------------------------------------------------- //
    //            Find function declaration in the source file               //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Find declaration for '"
                            << function_dump->name_ << "' in '"
                            << source_file_.GetPath() << "'.";
    }

    FunctionLocation function_location(function_dump->name_);
    if (!source_file_) {
        // The source file is not open
        return false;
    }

    if (!memory_) {
        // The file content is not loaded into memory
        return false;
    }

    std::string source_content = memory_.GetMapping();
    clang::tooling::runToolOnCode(
            std::make_unique<FrontendAction>(function_location),
            source_content
            );

    if (!function_location.is_filled_) {
        // Function was not found in the source file
        return false;
    }

    if (function_location.entity_.empty()) {
        // Function declaration was not set
        return false;
    }

    std::string function_declaration(function_location.entity_);

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Declaration for '"
                            << function_dump->name_ << "' is found.";
    }

    // --------------------------------------------------------------------- //
    //                       Generate fuzzer stub code                       //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Generate fuzzer stub data for '"
                            << function_dump->name_ << "'.";
    }

    std::string fuzzer_content;
    FuzzerGenerator fuzzer_generator(function_declaration, function_dump);
    fuzzer_generator.Generate();
    fuzzer_content = fuzzer_generator.GetFuzzer();

    if (fuzzer_content.empty()) {
        LOG(LOG_LEVEL_WARNING) << "Fuzzer stub data for '"
                               << function_dump->name_
                               << "' has not been generated.";
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Fuzzer stub data for '" << function_dump->name_
                            << "' has been generated.";
    }

    // --------------------------------------------------------------------- //
    //     Create the fuzzer stub (.cc file) and write fuzzer code to it     //
    // --------------------------------------------------------------------- //
    std::string fuzzer_stub_path;
    ConstructFuzzerStubPath(function_dump->name_,
                            function_directory_path, fuzzer_stub_path);
    File fuzzer_stub_file(fuzzer_stub_path);
    result = WriteFuzzerContentToFile(fuzzer_stub_file, fuzzer_content);
    if (!result) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Fuzzer stub file '" << fuzzer_stub_path
                            << "' has been created.";
    }

    // Put the file into the garbage right away after the creation
    PlaceIntoGarbage(fuzzer_stub_file);

    // --------------------------------------------------------------------- //
    //                   Compile fuzzer stub file to IR                      //
    // --------------------------------------------------------------------- //
    if (!Compiler::IsCompilable(fuzzer_stub_file)) {
        // Can not compile the file
        return false;
    }
    File ir_fuzzer_stub_file(fuzzer_stub_path);
    ir_fuzzer_stub_file.ReplaceExtension(ir_extension);

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Compiling '" << fuzzer_stub_path << "' to IR.";
    }

    result = Compiler::CompileToIR(fuzzer_stub_file, ir_fuzzer_stub_file);
    if (!result) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_fuzzer_stub_file.GetPath()
                            << "' created.";
    }
    // Put the file into the garbage right away after the creation
    PlaceIntoGarbage(ir_fuzzer_stub_file);

    // --------------------------------------------------------------------- //
    //          Resolve name mangling in the fuzzer stub IR file             //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Resolve name mangling in '"
                            << ir_fuzzer_stub_file.GetPath() << "'.";
    }

    // Modify IR fuzzer stub file (make suitable for separate compilation)
    PassLauncher pass_on_fuzzer(ir_fuzzer_stub_file.GetPath());
    result = pass_on_fuzzer.LaunchNameCorrector(function_dump);
    if (!result) {
        return false;
    }

    // --------------------------------------------------------------------- //
    //                 Launch separate compilation of 2 IRs                  //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Compiling separately 2 IRs:";
        LOG(LOG_LEVEL_INFO) << "\t" << ir_function_file.GetPath();
        LOG(LOG_LEVEL_INFO) << "\t" << ir_fuzzer_stub_file.GetPath();
    }

    // Compile both IR
    std::string fuzzer_executable_path;
    ConstructFuzzerExecutablePath(function_directory_path,
                                  fuzzer_executable_path);
    File final_executable_file(fuzzer_executable_path);

    // Compile all bits and pieces
    result = Compiler::CompileToFuzzer(
            ir_function_file,
            ir_fuzzer_stub_file,
            final_executable_file
            );

    if (!result) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Executable '"
                            << fuzzer_executable_path << "' created.";
        LOG(LOG_LEVEL_INFO) << "Fuzzer generation for '"
                            << function_dump->name_ << "' went successful.";
    }

    return true;
}

void SourceWrapper::ConstructResultDirectoryPath() {
    result_directory_path_ += source_file_.GetParentPath(); //TODO: May be working_directory_?
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
        const std::string& function_name, std::string& path) {
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
        const std::string& parent_directory, std::string& path) {
    path += parent_directory;
    path += "fuzz_";
    path += function_name;
    path += ".cc";
}

void SourceWrapper::ConstructFuzzerExecutablePath(
        const std::string& parent_directory, std::string& path) {
    path += parent_directory;
    path += "fuzzer";
}


bool SourceWrapper::WriteFuzzerContentToFile(
        File& file, const std::string& fuzzer_content) {

    if (fuzzer_content.empty()) {
        return false;
    }

    if (!override_) {
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

bool SourceWrapper::CreateDirectory(const std::string& path) {
    File directory(path);

    if (!override_) {
        if (directory.Exists()) {
            return false;
        }
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
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "Delete '" << file.GetPath() << "'.";
            }

            file.Delete();
        });
    }
}
