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
