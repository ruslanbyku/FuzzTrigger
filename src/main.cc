#include "pass_launcher.h"
#include "module.h"
#include "file.h"
#include "ir_compiler.h"
#include "fuzzer_generator.h"

#include <cstdio>

inline bool IsStartUpValid(int argc) { return argc == 2; }

int32_t LaunchRoutine(File& source_file) {
    if (!source_file.Exists()) {
        fprintf(stderr, "File [%s] does not exist\n",
                source_file.GetPath().c_str());
        return EXIT_FAILURE;
    }

    if (!source_file.IsCompilable()) {
        fprintf(stderr, "File [%s] can not be compiled\n",
                source_file.GetPath().c_str());
        return EXIT_FAILURE;
    }

    std::unique_ptr<Module> module_dump = std::make_unique<Module>();
    IRCompiler ir_compiler(source_file.GetPath());
    bool result = ir_compiler.Compile();
    if (!result) {
        return EXIT_FAILURE;
    }

    const std::string& ir_module = ir_compiler.GetIRFilePath();
    PassLauncher pass_launcher(ir_module);
    if (!pass_launcher.LaunchAnalysis(module_dump)) {
        return EXIT_FAILURE;
    }
    if (!*module_dump) {
        return EXIT_FAILURE;
    }

    for (auto& function: module_dump->functions_) {
        if (function->name_ != "sanitize_cookie_path") {
            continue;
        }

        pass_launcher.LaunchSanitizer(function);
    }

    /*
    for (auto& function: module_dump->functions_) {
        if (function->name_ != "sanitize_cookie_path") {
            continue;
        }

        FuzzerGenerator fuzzer_generator(source_file.GetPath(), function);
        result = fuzzer_generator.Generate();
        if (!result) {
            continue;
        }

        std::filesystem::path source_file_path = source_file.GetPath();
        std::string function_fuzzer_path = source_file_path.parent_path();
        function_fuzzer_path += "/";
        function_fuzzer_path += "fuzz_";
        function_fuzzer_path += function->name_;
        function_fuzzer_path += ".cc";
        File fuzzer_file(function_fuzzer_path);
        if (fuzzer_file.Exists()) {
            // So far do nothing
            continue;
        }
        int32_t fuzzer_file_descriptor = fuzzer_file.Create();
        if (fuzzer_file_descriptor == -1) {
            // So far do nothing
            continue;
        }
        int64_t bytes = write(
                fuzzer_file_descriptor,
                fuzzer_generator.GetFuzzer().c_str(),
                fuzzer_generator.GetFuzzer().size()
        );
        if (bytes == -1) {
            // So far do nothing
            continue;
        }
        fuzzer_file.Close();

        IRCompiler ir_compiler_2(function_fuzzer_path);
        result = ir_compiler_2.Compile();

        if (!result) {
            return EXIT_FAILURE;
        }
    }
     */


    /*
    int32_t source_file_descriptor = source_file.OpenForReadOnly();
    if (source_file_descriptor == -1) {
        return EXIT_FAILURE;
    }

    const char* source_file_content =
            source_file.LoadIntoMemory(source_file_descriptor);
    if (!source_file_content) {
        return EXIT_FAILURE;
    }

    for (auto& function: module_dump->functions_) {
        if (!function->is_standalone_) {
            continue;
        }

        // There are no arguments in the function
        if (function->arguments_.empty()) {
            continue;
        }

        FuzzerGenerator fuzzer_generator(source_file_content, function);
        bool result = fuzzer_generator.Generate();
        if (!result) {
            continue;
        }

        std::filesystem::path source_file_path = source_file.GetPath();

        std::string function_fuzzer_path = source_file_path.parent_path();
        function_fuzzer_path += "/";
        function_fuzzer_path += "fuzz_";
        function_fuzzer_path += function->name_;
        function_fuzzer_path += ".cc";

        File fuzzer_file(function_fuzzer_path);
        std::string n = fuzzer_file.GetPath();
        if (fuzzer_file.Exists()) {
            // So far do nothing
            continue;
        }

        int32_t fuzzer_file_descriptor = fuzzer_file.Create();
        if (fuzzer_file_descriptor == -1) {
            // So far do nothing
            continue;
        }

        int64_t bytes = write(
                fuzzer_file_descriptor,
                fuzzer_generator.GetFuzzer().c_str(),
                fuzzer_generator.GetFuzzer().size()
                );
        if (bytes == -1) {
            // So far do nothing
            continue;
        }
    }
     */

    return EXIT_SUCCESS;

}

// /home/chinesegranny/CLionProjects/AutoFuzz/test/llvm_pass/test_main.c
int main(int argc, char** argv) {
    if (!IsStartUpValid(argc)) {
        fprintf(stderr, "Usage: %s <.c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    File file(argv[1]);
    int32_t result = LaunchRoutine(file);
    if (result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
