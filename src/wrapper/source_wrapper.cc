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

#include "source_wrapper.h"

SourceWrapper::SourceWrapper(std::string source_file_path, Options options)
: Wrapper(options),
  source_file_(std::move(source_file_path)),
  ir_source_file_(source_file_),
  working_directory_(source_file_.GetParentPath()),
  module_dump_(std::make_unique<Module>()) {
    InitializeState();
}

SourceWrapper::~SourceWrapper() {
    EmptyGarbage(options_.auto_deletion_);
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
        std::string random_parameter   =
                              options_.random_on_      ? "true" : "false";
        std::string deletion_parameter =
                              options_.auto_deletion_  ? "true" : "false";
        std::string override_parameter =
                              options_.override_       ? "true" : "false";

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

    if (!Compiler::CompileToIR(source_file_, ir_source_file_)) {
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

    // ---------------------------------------------------------------------- //
    //                           Analysis Process                             //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "";
        LOG(LOG_LEVEL_INFO) << "Start analysis of '"
                            << ir_source_file_.GetPath() << "'.";
    }

    // Make analysis
    if (!PerformAnalysis()) {
        // Some errors occurred
        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Analysis went successful.";
        LOG(LOG_LEVEL_INFO) << "";
    }

    // --------------------------------------------------------------------- //
    //   Find declarations for all standalone functions in the source file   //
    // --------------------------------------------------------------------- //
    source_entity_ =
            FindDeclarationsPerSource(
                    source_file_.GetPath(),
                    module_dump_->standalone_functions_);

    if (source_entity_.empty()) {
        // No declarations were found
        return false;
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
                                                    source_file_,
                                                    options_.random_on_);

    if (result_directory_path_.empty()) {
        // Can not construct a global directory path
        // working_directory is empty
        return false;
    }

    if (!CreateDirectory(result_directory_path_, options_.override_)) {
        // Something went wrong
        return false;
    }

    // ---------------------------------------------------------------------- //
    //                            Generation Process                          //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "";
        LOG(LOG_LEVEL_INFO) << "Start fuzzer generation process.";
    }

    uint64_t successful_fuzzer_counter = 0;
    // Analysis is ready, module_dump has been filled
    for (const auto& function_dump: module_dump_->standalone_functions_) {
        // ------------------------------------------------------------------ //
        //                      Prepare for generation                        //
        // ------------------------------------------------------------------ //
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
        //                      Get function declaration                      //
        // ------------------------------------------------------------------ //
        // If a standalone function has no declaration, it will be impossible
        // to create a fuzzer
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Find declaration for '"
                                << function_dump->name_ << "'.";
        }

        std::string function_declaration(GetDeclaration(function_dump->name_));

        if (function_declaration.empty()) {
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "No declaration for '"
                                    << function_dump->name_
                                    << "' was found.";
            }

            continue;
        }

        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Declaration for '"
                                << function_dump->name_ << "' was found.";
        }

        // ------------------------------------------------------------------ //
        //        Create a directory to store data about the function         //
        // ------------------------------------------------------------------ //
        LOG(LOG_LEVEL_INFO) << "Create a directory to store results about '"
                            << function_dump->name_ << "'.";

        std::string function_directory_path;

        function_directory_path =
                ConstructFunctionDirectoryPath(
                        result_directory_path_,
                        function_dump->name_,
                        options_.random_on_);

        if (function_directory_path.empty()) {
            // Can not construct a function directory path
            // Either result_directory_path or function name is empty
            return false;
        }

        if (!CreateDirectory(function_directory_path, options_.override_)) {
            // Something went wrong
            return false;
        }

        // ------------------------------------------------------------------ //
        //                         Start generation                           //
        // ------------------------------------------------------------------ //
        if (!PerformGeneration(function_directory_path,
                               function_dump,
                               function_declaration)) {
            // 1) Something bad has happened
            // 2) No error occurred, just could not continue to
            // generate a fuzzer

            // In case fuzzer generation process aborts at some point further,
            // delete its directory
            if (LOGGER_ON) {
                LOG(LOG_LEVEL_INFO) << "Delete '"
                                    << function_directory_path << "'.";
            }

            File function_directory(function_directory_path);
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

bool SourceWrapper::PerformAnalysis() {
    PassLauncher pass_on_source(ir_source_file_.GetPath());

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

bool SourceWrapper::PerformGeneration(
                          std::string function_directory_path,
                          const std::shared_ptr<Function>& function_dump,
                          std::string function_declaration) {
    // --------------------------------------------------------------------- //
    //             Create a separate IR file for the function                //
    // --------------------------------------------------------------------- //
    std::string ir_function_path;
    ir_function_path += function_directory_path;
    ir_function_path += function_dump->name_;
    ir_function_path += ir_extension;

    if (!ir_source_file_.Copy(ir_function_path, options_.override_)) {
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

    if (!pass_on_function_ir.LaunchOnFunction<Sanitizer>(function_dump)) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_WARNING) << "Sanitization of '" << ir_function_path
                                   << "' went unsuccessful.";
        }

        return false;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Sanitization of '" << ir_function_path
                            << "' went successful.";
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
                                  fuzzer_content, options_.override_)) {
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

    if (!pass_on_fuzzer.LaunchOnFunction<NameCorrector>(function_dump)) {
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

std::string SourceWrapper::GetDeclaration(
                                 const std::string& function_name) const {
    std::string function_declaration;

    for (const auto& function_entity: source_entity_) {
        if (function_entity.name_ == function_name) {

            //
            // Found our target function
            //

            function_declaration = function_entity.declaration_;

            return function_declaration;
        }
    }

    return function_declaration;
}
