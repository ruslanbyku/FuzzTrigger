#include "name_corrector.h"

char NameCorrector::ID = 0;

NameCorrector::NameCorrector(const std::shared_ptr<Function>& function_dump,
                             bool& operation_status)
: llvm::ModulePass(ID), function_dump_(function_dump),
success_(operation_status) {}

llvm::StringRef NameCorrector::getPassName() const {
    return "FunctionNameCorrector";
}

bool NameCorrector::runOnModule(llvm::Module& module) {
    // !IMPORTANT!
    // Check if the name mangling process went successful
    // Segmentation fault might interrupt the program
    success_ = DropManglingOnTargetFunction(module, function_dump_->name_);
    if (!success_) {
        // Name correction failed
        // A new module seems invalid after name correction
        //
        // The module has not been modified yet, then return false
        return false;
    }

    UpdateIRModule(module);
    //Debug(module);

    // The module has been modified, then return true
    return true;
}

bool NameCorrector::DropManglingOnTargetFunction(
        llvm::Module& module, const std::string& original_function_name) {
    llvm::Function* target_function   = nullptr;

    // Find the equivalent of the original function in the module
    for (llvm::Function& function: module) {
        // Assuming there must be a declaration
        if (function.isDeclaration()) {
            std::string function_name(function.getName().str());
            auto position = function_name.find(original_function_name);

            // For now assuming that there is only one function that
            // incorporates original_function_name
            if (position != std::string::npos) {
                target_function = &function;
            }
        }
    }

    if (!target_function) {
        // The target function was not found in the module
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

            callee->setName(original_function_name);
        }
    }

    // Replace name in the declaration
    target_function->setName(original_function_name);

    // true  - An error is present
    // false - The module is okay
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
