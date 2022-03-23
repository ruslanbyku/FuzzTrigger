#include "name_corrector.h"

char NameCorrector::ID = 0;

NameCorrector::NameCorrector(const std::unique_ptr<Function>& function_dump)
: llvm::ModulePass(ID), function_dump_(function_dump) {}

llvm::StringRef NameCorrector::getPassName() const {
    return "FunctionNameCorrector";
}

bool NameCorrector::runOnModule(llvm::Module& module) {
    llvm::Function* target_function = nullptr;

    for (llvm::Function& function: module) {
        // Assuming there must be a declaration
        if (function.isDeclaration()) {
            std::string function_name = function.getName().str();
            if (function_name.find(function_dump_->name_)
                                                         != std::string::npos) {
                target_function = &function;
            }
        }
    }

    if (!target_function) {
        // Something went wrong, abort
        return false;
    }

    for (llvm::User* user: target_function->users()) {
        if (auto caller = llvm::dyn_cast<llvm::CallInst>(user)) {
            llvm::Function* callee = caller->getCalledFunction();

            callee->setName(function_dump_->name_);
        }
    }
    target_function->setName(function_dump_->name_);

    UpdateIRModule(module);

    return true;
}

void NameCorrector::UpdateIRModule(llvm::Module& module) {
    const std::string& path = module.getModuleIdentifier();
    std::error_code error_code;

    llvm::raw_fd_ostream file(path, error_code);
    module.print(file, nullptr);
}

void NameCorrector::Debug(llvm::Module& module) {
    llvm::raw_ostream& stdout_ = llvm::outs();
    module.print(stdout_, nullptr);
}
