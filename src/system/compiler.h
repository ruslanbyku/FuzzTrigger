/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZTRIGGER_COMPILER_H
#define FUZZTRIGGER_COMPILER_H

#include "file.h"
#include "logger.h"

#include <string>
#include <array>
#include <algorithm>

const char* const c_compiler                          = "clang";
const char* const cpp_compiler                        = "clang++";
const std::array<const char* const, 8> cxx_extensions = {".C", ".c",
                                                         ".cp", ".cc",
                                                         ".cpp", ".CPP",
                                                         ".c++", ".cxx"};
const char* const ir_extension = ".ll";

// https://stackoverflow.com/a/112451
namespace Compiler {
    bool IsCompilable(const File&);
    bool CompileToFuzzer(const File&, const File&, const File&);
    bool CompileToIR(const File&, const File&);
}

#endif //FUZZTRIGGER_COMPILER_H
