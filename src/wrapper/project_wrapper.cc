#include "project_wrapper.h"

ProjectWrapper::ProjectWrapper(std::string ir_project,
                               std::string sources, bool auto_deletion,
                               bool random_on, bool override) noexcept(false)
: ir_project_(std::move(ir_project)),
  sources_(std::move(sources)),
  working_directory_(ir_project_.GetParentPath()),
  module_dump_(std::make_unique<Module>()),
  auto_deletion_(auto_deletion),
  random_on_(random_on),
  override_(override) {
    InitializeState();
}

ProjectWrapper::~ProjectWrapper() {
    EmptyGarbage(auto_deletion_);
}

void ProjectWrapper::InitializeState() {
    if (!ir_project_.IsAbsolute()) {
        std::string exception_message;

        exception_message += "File path of '";
        exception_message += ir_project_.GetPath();
        exception_message += "' is relative.";

        throw std::runtime_error(exception_message);
    }

    if (!ir_project_.Exists()) {
        std::string exception_message;

        exception_message += "File '";
        exception_message += ir_project_.GetPath();
        exception_message += "' does not exist.";

        throw std::runtime_error(exception_message);
    }

    if (!sources_.IsAbsolute()) {
        std::string exception_message;

        exception_message += "File path of '";
        exception_message += sources_.GetPath();
        exception_message += "' is relative.";

        throw std::runtime_error(exception_message);
    }

    if (!sources_.Exists()) {
        std::string exception_message;

        exception_message += "File '";
        exception_message += sources_.GetPath();
        exception_message += "' does not exist.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_project_.GetPath() << "' found.";
        LOG(LOG_LEVEL_INFO) << "File '" << sources_.GetPath() << "' found.";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Current working directory '"
                            << working_directory_ << "'.";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Check whether '" << ir_project_.GetPath()
                            << "' looks like an IR file.";
    }

    if (!Compiler::IsCompilable(ir_project_)) {
        std::string exception_message;

        exception_message += "File '";
        exception_message += ir_project_.GetPath();
        exception_message += "' does look like an IR file.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_project_.GetPath()
                            << "' looks like an IR file.";
        //
        //
        //
        LOG(LOG_LEVEL_INFO) << "Extract source paths from '"
                            << sources_.GetPath() << "':";
    }

    // If a source path is not valid, proceed anyway.
    uint64_t lines = 0;
    std::ifstream sources_file(sources_.GetPath());
    std::string single_source_path;
    while (std::getline(sources_file, single_source_path)) {
        ++lines;

        // \n is deleted automatically
        File source(single_source_path);

        if (source.IsAbsolute() &&
            source.Exists() &&
            Compiler::IsCompilable(source)) {

            source_paths_.insert(single_source_path);

            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "'" << single_source_path << "' found.";
            }
        } else {
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_WARNING) << "Can not recognize '"
                                       << single_source_path << "'.";
            }
        }
    }

    if (source_paths_.empty()) {
        std::string exception_message;

        exception_message += "No source paths were found in '";
        exception_message += sources_.GetPath();
        exception_message += "'.";

        throw std::runtime_error(exception_message);
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "[recognized files/total lines]: ["
                            << source_paths_.size()
                            << "/"
                            << lines
                            << "]";
        //
        //
        //
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

bool ProjectWrapper::LaunchRoutine() {
    // ---------------------------------------------------------------------- //
    //                           Analysis Process                             //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "";
        LOG(LOG_LEVEL_INFO) << "Start analysis of '" << ir_project_.GetPath()
                            << "'.";
    }

    // Make analysis
    bool analysis_result = PerformAnalysis();
    if (!analysis_result) {
        // Some errors occurred
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Analysis went successful.";
        LOG(LOG_LEVEL_INFO) << "";
    }

    // ---------------------------------------------------------------------- //
    //           Find all function declarations in every source file          //
    // ---------------------------------------------------------------------- //
    for (const std::string& source_path: source_paths_) {
        SourceEntity source_entity;

        // Open the file
        File source_file(source_path);
        int32_t source_descriptor = source_file.OpenForReadOnly();
        if (source_descriptor == -1) {
            // Can not open file
            return false;
        }

        auto source_file_size = static_cast<int32_t>(source_file.GetSize());

        // Load file into memory
        VirtualMapper memory;
        if (!memory.AllocateReadMap(source_descriptor, source_file_size)) {
            // Can not load file into memory
            return false;
        }

        // Get file content
        std::string source_content = memory.GetMapping();

        // Find all function declaration for a single source file
        clang::tooling::runToolOnCode(
                std::make_unique<FullSourceParser>(source_entity),
                source_content);

        // Save function declarations
        function_declarations_[source_path] = source_entity;

        // Explicitly unload the file from memory (the destructor will get
        // it anyway)
        memory.Unmap();
        // Close the file descriptor (the destructor will NOT get it)
        source_file.Close();
    }

    // ---------------------------------------------------------------------- //
    //           Create a global directory to store program results           //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Create a directory to store program results.";
    }

    // Construct a directory path to store results
    result_directory_path_ = ConstructResultDirectoryPath(
                                                    working_directory_,
                                                    ir_project_,
                                                    random_on_);

    if (result_directory_path_.empty()) {
        // Can not construct a global directory
        // working_directory is empty
        return false;
    }

    if (!CreateDirectory(result_directory_path_, override_)) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "Directory '"
                                 << result_directory_path_
                                 << "' exists.";
        }

        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Directory '" << result_directory_path_
                            << "' created.";
    }

    // ---------------------------------------------------------------------- //
    //                            Generation Process                          //
    // ---------------------------------------------------------------------- //
    uint64_t successful_fuzzer_counter = 0;
    // Analysis is ready, module_dump has been filled
    for (auto& function_dump: module_dump_->functions_) {
        if (!function_dump->is_standalone_) {
            continue;
        }

        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Generate fuzzer for '"
                                << function_dump->name_ << "'.";
        }

        // ------------------------------------------------------------------ //
        //    Check whether a standalone function is qualified to be fuzzed   //
        // ------------------------------------------------------------------ //
        // If a standalone function has no argument for input, there is
        // no point in making a fuzzer for it
        if (function_dump->arguments_number_ == 0) {
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "Function '"
                                    << function_dump->name_
                                    << "' has no input arguments.";
            }

            continue;
        }

        // ------------------------------------------------------------------ //
        //        Create a directory to store data about the function         //
        // ------------------------------------------------------------------ //
        std::string function_directory_path;

        function_directory_path =
                ConstructFunctionDirectoryPath(
                        result_directory_path_,
                        function_dump->name_,
                        random_on_);

        if (function_directory_path.empty()) {
            // Can not construct a function directory path
            // Either result_directory_path or function name is empty
            return false;
        }

        if (!CreateDirectory(function_directory_path, override_)) {
            // An error occurred while creating a function directory, but
            // this error does not impede the further program execution, so
            // leave it.
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_WARNING) << "Directory '"
                                       << function_directory_path
                                       << "' exists.";
            }

            continue;
        }

        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Directory '" << function_directory_path
                                << "' created.";
        }

        // ------------------------------------------------------------------ //
        //                         Start generation                           //
        // ------------------------------------------------------------------ //
        if (!PerformGeneration(function_directory_path, function_dump)) {
            // 1) Something bad has happened
            // 2) No error occurred, just could not continue to
            // generate a fuzzer

            // In case fuzzer generation process aborts at some point further,
            // delete its directory
            File function_directory(function_directory_path);

            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "Delete '"
                                    << function_directory_path << "'.";
            }

            function_directory.Delete();

            continue;
        }

        ++successful_fuzzer_counter;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "End of fuzzer generation process.";
        LOG(LOG_LEVEL_INFO) << "";

        if (successful_fuzzer_counter == 1) {
            LOG(LOG_LEVEL_INFO) << "1 fuzzer generated.";
        } else {
            LOG(LOG_LEVEL_INFO) << successful_fuzzer_counter
                                << " fuzzers generated.";
        }
    }

    return true;
}

bool ProjectWrapper::PerformAnalysis() {
                             PassLauncher pass_on_source(ir_project_.GetPath());

    if (!pass_on_source.LaunchAnalysis(module_dump_)) {
        // Can not launch analysis
        return false;
    }

    if (!*module_dump_) {
        // Analysis resulted unsuccessful or there are no standalone functions
        return false;
    }

    return true;
}

bool ProjectWrapper::PerformGeneration(
                           std::string function_directory_path,
                           const std::unique_ptr<Function>& function_dump) {
    // --------------------------------------------------------------------- //
    //             Create a separate IR file for the function                //
    // --------------------------------------------------------------------- //
    std::string ir_function_path;
    ir_function_path += function_directory_path;
    ir_function_path += function_dump->name_;
    ir_function_path += ir_extension;

    if (!ir_project_.Copy(ir_function_path, override_)) {
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

    if (!pass_on_function_ir.LaunchSanitizer(function_dump)) {
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "File '" << ir_function_path
                            << "' has been sanitized.";
    }

    // --------------------------------------------------------------------- //
    //                      Get the function declaration                     //
    // --------------------------------------------------------------------- //
    std::string function_declaration;

    for (const auto& pair: function_declarations_) {
        SourceEntity source_entity = pair.second;

        if (source_entity.empty()) {
            // TODO: Deal with it later
            // No function declarations
            continue;
        }

        for (const auto& function_entity: source_entity) {
            if (function_entity.name_ != function_dump->name_) {
                continue;
            }

            //
            // Found our target function
            //

            if (function_entity.declaration_.empty()) {
                // Function declaration was not found
                return false;
            }

            function_declaration = function_entity.declaration_;
        }
    }

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
    fuzzer_stub_path = ConstructFuzzerStubPath(
            function_directory_path,
            function_dump->name_);

    if (fuzzer_stub_path.empty()) {
        // Can not construct a fuzzer stub path
        // Either function_directory_path or function name is empty
        return false;
    }

    File fuzzer_stub_file(fuzzer_stub_path);

    if (!WriteFuzzerContentToFile(fuzzer_stub_file,
                                  fuzzer_content, override_)) {
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

    if (!Compiler::CompileToIR(fuzzer_stub_file, ir_fuzzer_stub_file)) {
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

    if (!pass_on_fuzzer.LaunchNameCorrector(function_dump)) {
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
    fuzzer_executable_path =
            ConstructFuzzerExecutablePath(function_directory_path);

    if (fuzzer_executable_path.empty()) {
        // Can not construct a fuzzer executable path
        // function_directory_path is empty
        return false;
    }

    File final_executable_file(fuzzer_executable_path);

    // Compile all bits and pieces
    bool final_compilation_result = Compiler::CompileToFuzzer(
            ir_function_file,
            ir_fuzzer_stub_file,
            final_executable_file);

    if (!final_compilation_result) {
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
