#include "pass_launcher.h"
#include "module.h"
#include "file.h"
#include "compiler.h"
#include "fuzzer_generator.h"

#include <cstdio>
#include <random>
#include <sstream>

inline bool IsStartUpValid(int argc) { return argc == 2; }
// https://stackoverflow.com/a/13445752
using engine = std::mt19937;
inline uint32_t GenerateRandom() {
    std::random_device device;
    engine generator(device());
    std::uniform_int_distribution<engine::result_type> distribution(1,
                                                                    UINT32_MAX);

    return distribution(generator);
}

inline std::string GenerateHash(uint32_t random) {
    std::string random_string = std::to_string(random);
    uint64_t hash = std::hash<std::string>{}(random_string);

    std::stringstream stream;
    stream << std::hex << hash;

    return stream.str();
}

inline std::string ReturnShortenedHash(uint32_t random, uint16_t length = 8) {
    std::string hash = GenerateHash(random);

    if (hash.length() < length) {
        return hash;
    }

    return hash.substr(0, length);
}

bool PerformAnalysis(const File& ir_source_file,
                                         std::unique_ptr<Module>& module_dump) {
    PassLauncher pass_on_source(ir_source_file.GetPath());

    if (!pass_on_source.LaunchAnalysis(module_dump)) {
        return EXIT_FAILURE;
    }

    if (!*module_dump) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

bool PerformGeneration() {

}

int32_t LaunchRoutine(File& source_file) {
    if (!source_file.Exists()) {
        fprintf(stderr, "File [%s] does not exist\n",
                source_file.GetPath().c_str());

        return EXIT_FAILURE;
    }

    if (!Compiler::IsCompilable(source_file)) {
        fprintf(stderr, "File [%s] can not be compiled\n",
                source_file.GetPath().c_str());

        return EXIT_FAILURE;
    }
    File ir_source_file(source_file);
    ir_source_file.ReplaceExtension(ir_extension);
    bool result = Compiler::CompileToIR(source_file, ir_source_file);
    if (!result) {
        return EXIT_FAILURE;
    }
    std::unique_ptr<Module> module_dump = std::make_unique<Module>();
    bool analysis_result = PerformAnalysis(ir_source_file, module_dump);
    if (analysis_result) {
        return EXIT_FAILURE;
    }

    // Create a new directory for the results
    std::string result_directory;
    result_directory += source_file.GetParentPath();
    result_directory += "/";
    result_directory += source_file.GetStem();
    result_directory += "_fuzz_results_";
    result_directory += ReturnShortenedHash(GenerateRandom());
    result_directory += "/";
    File directory(result_directory);
    if (directory.Exists()) {
        return EXIT_FAILURE;
    }
    directory.CreateDirectory();

    // Generate fuzzer for each found function
    for (auto& function: module_dump->functions_) {
        if (function->name_ != "sanitize_cookie_path") {
            continue;
        }

        // Create a function_directory for a function
        std::string function_directory_path;
        function_directory_path += result_directory;
        function_directory_path += function->name_;
        function_directory_path += "_";
        function_directory_path += ReturnShortenedHash(GenerateRandom());
        function_directory_path += "/";
        File function_directory(function_directory_path);
        if (function_directory.Exists()) {
            return EXIT_FAILURE;
        }
        function_directory.CreateDirectory();

        // Copy source IR to a new IR of a function
        std::string function_ir;
        function_ir += function_directory_path;
        function_ir += function->name_;
        function_ir += ir_extension;

        if (!ir_source_file.Copy(function_ir)) {
            return EXIT_FAILURE;
        }

        // Sanitize function IR file
        PassLauncher pass_on_function_ir(function_ir);
        pass_on_function_ir.LaunchSanitizer(function);

        //
        //
        //
        //

        // Generate fuzzer stub content
        std::string fuzzer;
        std::string declaration("static char* sanitize_cookie_path(const char* cookie_path);");
        FuzzerGenerator fuzzer_generator(declaration, function);
        fuzzer_generator.Generate();
        fuzzer = fuzzer_generator.GetFuzzer();

        // Create the fuzzer stub file and write fuzzer data to it
        std::string fuzzer_stub;
        fuzzer_stub += function_directory_path;
        fuzzer_stub += "fuzz_";
        fuzzer_stub += function->name_;
        fuzzer_stub += ".cc";
        File fuzzer_stub_file(fuzzer_stub);
        if (fuzzer_stub_file.Exists()) {
            // So far do nothing
            continue;
        }
        int32_t fuzzer_stub_descriptor = fuzzer_stub_file.Create();
        if (fuzzer_stub_descriptor == -1) {
            // So far do nothing
            continue;
        }
        int64_t bytes = fuzzer_stub_file.Write(fuzzer, fuzzer.size());
        if (bytes == -1) {
            // So far do nothing
            continue;
        }
        fuzzer_stub_file.Close();

        // Compile fuzzer stub file to IR
        if (!Compiler::IsCompilable(fuzzer_stub_file)) {
            fprintf(stderr, "File [%s] can not be compiled\n",
                    fuzzer_stub_file.GetPath().c_str());

            return EXIT_FAILURE;
        }
        File ir_fuzzer_stub(fuzzer_stub);
        ir_fuzzer_stub.ReplaceExtension(ir_extension);
        result = Compiler::CompileToIR(fuzzer_stub_file, ir_fuzzer_stub);
        if (!result) {
            return EXIT_FAILURE;
        }

        // Modify fuzzer IR
        PassLauncher pass_on_fuzzer(ir_fuzzer_stub.GetPath());
        pass_on_fuzzer.LaunchNameCorrector(function);

        // Compile both IR
        std::string final;
        final += function_directory_path;
        final += "fuzzer";
        File final_file(final);
        File function_ir_file(function_ir);
        Compiler::CompileToFuzzer(function_ir_file, ir_fuzzer_stub, final_file);

        /*
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
         */
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
