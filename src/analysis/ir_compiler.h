#ifndef AUTOFUZZ_IR_COMPILER_H
#define AUTOFUZZ_IR_COMPILER_H

#include "file.h"

#include <cstdlib>
#include <string>
#include <filesystem>

const char* const compiler     = "clang";
const char* const ir_extension = ".ll";

class IRCompiler {
public:
    explicit IRCompiler(std::string);
    ~IRCompiler();
    IRCompiler(const IRCompiler&)                = delete;
    IRCompiler& operator=(const IRCompiler&)     = delete;
    IRCompiler(IRCompiler&&) noexcept            = delete;
    IRCompiler& operator=(IRCompiler&&) noexcept = delete;

    bool Compile();
    void Delete();
    const std::string& GetIRFilePath() const;

private:
    std::string source_path_;
    std::string ir_path_;
};


#endif //AUTOFUZZ_IR_COMPILER_H
