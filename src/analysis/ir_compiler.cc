#include "ir_compiler.h"

IRCompiler::IRCompiler(std::string source_path)
: source_path_(std::move(source_path)) {
    std::filesystem::path ir_path(source_path_);

    ir_path.replace_extension(ir_extension);

    ir_path_ = ir_path;
}

IRCompiler::~IRCompiler() {
    Delete();
}

bool IRCompiler::Compile() {
    //TODO: Check clang existence

    if (system(nullptr) == 0) {
        // Can not execute the command
        return false;
    }

    // Construct a command to be compiled
    std::string command;
    command += compiler;
    command += " -emit-llvm ";
    command += source_path_;
    command += " -S";
    command += " -o ";
    command += ir_path_;
    command += " > /dev/null 2>&1"; // &> /dev/null

    system(command.c_str());

    File ir_path(ir_path_);
    if (!ir_path.Exists()) {
        return false;
    }

    return true;
}

void IRCompiler::Delete() {
    if (!ir_path_.empty()) {
        if (remove(ir_path_.c_str()) != 0) {
            // An error occurred when deleting a file, probably there is
            // already no such file, not a problem
        }

        ir_path_.erase();
    }
}

const std::string &IRCompiler::GetIRFilePath() const {
    return ir_path_;
}
