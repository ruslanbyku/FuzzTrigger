#include "compiler.h"

namespace Compiler {
    namespace {
        bool Compile(const std::string& command) {
            if (std::system(nullptr) == 0) {
                // Can not execute the command
                return false;
            }

            std::system(command.c_str());

            return true;
        }
    }

    bool IsCompilable(const File& file) {
        if (!file.Exists()) {
            return false;
        }

        std::string file_extension(file.GetExtension());
        if (file_extension.empty()) {
            return false;
        }

        bool is_cxx = std::ranges::any_of(
                cxx_extensions,
                [&file_extension](const char* const extension) {
                    return file_extension == extension;
                });
        if (is_cxx) {
            return true;
        }

        bool is_ir = file_extension == std::string(ir_extension);
        if (is_ir) {
            return true;
        }

        return false;
    }

    bool CompileToFuzzer(const File& source_ir,
                         const File& fuzzer_ir, const File& executable) {
        std::string command;

        if (!source_ir.Exists() || !IsCompilable(source_ir)) {
            return false;
        }

        if (!fuzzer_ir.Exists() || !IsCompilable(fuzzer_ir)) {
            return false;
        }

        // Construct a command to be compiled
        command += cpp_compiler;
        command += " -O0";
        command += " -g";
        command += " -fno-omit-frame-pointer";
        command += " -fsanitize=address,fuzzer";
        command += " -fsanitize-coverage=trace-cmp,trace-gep,trace-div ";
        command += source_ir.GetPath() + " ";
        command += fuzzer_ir.GetPath();
        command += " -o ";
        command += executable.GetPath();
        command += " > /dev/null 2>&1"; // &> /dev/null

        bool executed = Compile(command);
        if (!executed) {
            return false;
        }

        if (!executable.Exists()) {
            return false;
        }

        return true;
    }

    bool CompileToIR(const File& source, const File& ir) {
        std::string command;

        if (!source.Exists() || !IsCompilable(source)) {
            return false;
        }

        // Construct a command to be compiled
        command += c_compiler;
        command += " -O0";
        command += " -emit-llvm ";
        command += source.GetPath();
        command += " -S ";
        command += " -o ";
        command += ir.GetPath();
        command += " > /dev/null 2>&1"; // &> /dev/null

        bool executed = Compile(command);
        if (!executed) {
            return false;
        }

        if (!ir.Exists()) {
            return false;
        }

        return true;
    }

} // end Compiler
