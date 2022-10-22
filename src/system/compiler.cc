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
