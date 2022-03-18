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
    IRCompiler source_to_ir(source_file.GetPath());
    bool result = source_to_ir.Compile();
    if (!result) {
        return EXIT_FAILURE;
    }

    const std::string& ir_source_path = source_to_ir.GetIRFilePath();
    PassLauncher pass_on_source(ir_source_path);
    if (!pass_on_source.LaunchAnalysis(module_dump)) {
        return EXIT_FAILURE;
    }
    if (!*module_dump) {
        return EXIT_FAILURE;
    }

    for (auto& function: module_dump->functions_) {
        if (function->name_ != "sanitize_cookie_path") {
            continue;
        }

        // Sanitize source file
        pass_on_source.LaunchSanitizer(function);

        // Generate fuzzer stub
        std::string fuzzer;
        std::string declaration("static char* sanitize_cookie_path(const char* cookie_path);");
        FuzzerGenerator fuzzer_generator(declaration, function);
        fuzzer_generator.Generate();
        fuzzer = fuzzer_generator.GetFuzzer();

        // Write fuzzer into a file
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
                fuzzer.c_str(),
                fuzzer.size()
        );
        if (bytes == -1) {
            // So far do nothing
            continue;
        }
        fuzzer_file.Close();

        // Compile fuzzer stub to IR and correct the function name
        IRCompiler fuzzer_to_ir(function_fuzzer_path);
        result = fuzzer_to_ir.Compile();
        if (!result) {
            return EXIT_FAILURE;
        }
        const std::string& ir_fuzzer_path = fuzzer_to_ir.GetIRFilePath();
        PassLauncher pass_on_fuzzer(ir_fuzzer_path);
        pass_on_fuzzer.LaunchNameCorrector(function);

        // Both IRs are ready, compile them
        std::string fuzzer_executable = source_file_path.parent_path();
        fuzzer_executable += "/fuzzer";
        std::string command;
        command += "clang++ ";
        command += "-O2 ";
        command += "-g ";
        command += "-fno-omit-frame-pointer ";
        command += "-fsanitize=address,fuzzer ";
        command += "-fsanitize-coverage=trace-cmp,trace-gep,trace-div ";
        command += ir_source_path + " ";
        command += ir_fuzzer_path + " ";
        command += "-o ";
        command += fuzzer_executable;

        system(command.c_str());
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
