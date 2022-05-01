#include "name_corrector.h"

char NameCorrector::ID = 0;

NameCorrector::NameCorrector(const std::shared_ptr<Function>& function_dump,
                             bool& status)
: llvm::ModulePass(ID), function_dump_(function_dump), success_(status) {}

llvm::StringRef NameCorrector::getPassName() const {
    return "FunctionNameCorrector";
}

bool NameCorrector::runOnModule(llvm::Module& module) {
    llvm::Function* target_function   = nullptr;
    std::string     original_function(function_dump_->name_);

    // Find the equivalent of the original function in the module
    for (llvm::Function& function: module) {
        // Assuming there must be a declaration
        if (function.isDeclaration()) {
            std::string function_name = function.getName().str();

            if (function_name.find(original_function) != std::string::npos) {
                target_function = &function;
            }
        }
    }

    if (!target_function) {
        // // The equivalent function can not be found, abort.
        return false;
    }

    // There are only 2 places where substitution needs to be done.
    // 1) Inside the body - caller instruction
    // 2) Declaration
    //
    // Replace name in the caller instruction
    for (llvm::User* user: target_function->users()) {
        if (auto caller = llvm::dyn_cast<llvm::CallInst>(user)) {
            llvm::Function* callee = caller->getCalledFunction();

            callee->setName(original_function);
        }
    }

    // Replace name in the declaration
    target_function->setName(original_function);

    // !IMPORTANT!
    // Check if the name mangling process went successful
    // Segmentation fault might interrupt the program
    success_ = IsModuleValid(module);
    if (!success_) {
        return false;
    }

    UpdateIRModule(module);

    return true;
}

// true  - An error is present
// false - The module is okay
bool NameCorrector::IsModuleValid(llvm::Module& module) {
    return !llvm::verifyModule(module);
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
