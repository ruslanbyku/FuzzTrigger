#ifndef AUTOFUZZ_COMPILER_H
#define AUTOFUZZ_COMPILER_H

#include "file.h"

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

/*
class Compiler {
public:
    explicit Compiler(File)                  = delete;
    ~Compiler()                              = delete;
    Compiler(const Compiler&)                = delete;
    Compiler& operator=(const Compiler&)     = delete;
    Compiler(Compiler&&) noexcept            = delete;
    Compiler& operator=(Compiler&&) noexcept = delete;

    static bool IsCompilable(const File&);
    static bool CompileToFuzzer(const File&, const File&, const File&);
    static bool CompileToIR(const File&, const File&);

private:

    static bool Compile(const std::string&);
};
*/

#endif //AUTOFUZZ_COMPILER_H
