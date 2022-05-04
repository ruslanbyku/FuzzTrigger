#include "sanitizer.h"

char Sanitizer::ID = 0;

Sanitizer::Sanitizer(const std::shared_ptr<Function>& function_dump,
                     bool& operation_status)
: llvm::ModulePass(ID), function_dump_(function_dump),
target_function_(nullptr), success_(operation_status) {}

llvm::StringRef Sanitizer::getPassName() const {
    return "ModuleSanitizer";
}

bool Sanitizer::runOnModule(llvm::Module& module) {
    // Preparation
    // Find the corresponding function in the module beforehand
    for (llvm::Function& function: module) {
        std::string function_name = function.getName().str();

        if (function_name == function_dump_->name_) {
            target_function_ = &function;
            break;
        }
    }

    if (!target_function_) {
        // The target function was not found in the module
        success_ = false;

        // The module has not been modified, then return false
        return false;
    }

    // !IMPORTANT!
    // Check if the sanitization process went successful
    // Segmentation fault might interrupt the program
    success_ = SanitizeModule(module);
    if (!success_) {
        // Sanitization failed
        // A new module seems invalid after sanitization
        //
        // The module has not been modified yet, then return false
        return false;
    }

    UpdateIRModule(module);
    //Debug(module);

    // The module has been modified, then return true
    return true;
}

bool Sanitizer::SanitizeModule(llvm::Module& module) {
    // ---------------------------------------------------------------------- //
    //                      Search non-dependant globals                      //
    // ---------------------------------------------------------------------- //
    std::set<llvm::GlobalVariable*> global_garbage;

    for (llvm::GlobalVariable& global: module.getGlobalList()) {

        bool is_droppable = true;
        for (llvm::User* global_user: global.users()) {
            if (!IsDroppable(global_user)) {
                is_droppable = false;
                break;
            }
        }

        if (is_droppable) {
            global_garbage.insert(&global);
        }
    }

    // ---------------------------------------------------------------------- //
    //                     Search non-dependant functions                     //
    // ---------------------------------------------------------------------- //
    std::set<llvm::Function*> function_garbage;

    for (llvm::Function& function: module) {

        if (&function == target_function_) {
            continue;
        }

        bool is_droppable = true;
        for (llvm::User* function_user: function.users()) {
            if (!IsDroppable(function_user)) {
                is_droppable = false;
                break;
            }
        }

        if (is_droppable) {
            function_garbage.insert(&function);
        }
    }

    // ---------------------------------------------------------------------- //
    //                            Sanitize module                             //
    // ---------------------------------------------------------------------- //
    for (auto* global: global_garbage) {
        global->dropAllReferences();
        global->removeFromParent();
    }

    for (auto* function: function_garbage) {
        function->dropAllReferences();
        function->removeFromParent();
    }

    // ---------------------------------------------------------------------- //
    //                 Target function visibility resolution                  //
    // ---------------------------------------------------------------------- //
    if (!target_function_->hasDefaultVisibility()) {
        target_function_->setVisibility(Visibility::DefaultVisibility);
    }

    // ---------------------------------------------------------------------- //
    //                  Target function linkage resolution                    //
    // ---------------------------------------------------------------------- //
    if (!target_function_->hasExternalLinkage()) {
        // Internally visible function (static)
        target_function_->setLinkage(Linkage::ExternalLinkage);
    }

    // true  - An error is present
    // false - The module is okay
    return !llvm::verifyModule(module);
}

bool Sanitizer::IsDroppable(llvm::User* user) {
    llvm::Instruction* instruction;
    llvm::Constant*    constant;

    if (llvm::isa<llvm::Instruction>(user)) {
        instruction = llvm::cast<llvm::Instruction>(user);

        return !IsMemberOfTargetFunction(*instruction);
    }

    if (llvm::isa<llvm::Constant>(user)) {
        constant = llvm::cast<llvm::Constant>(user);

        if (visited_constants_.contains(constant)) {
            // Already encountered a constant, no need to proceed, return the
            // result
            return visited_constants_[constant];
        }

        for (llvm::User* constant_user: constant->users()) {
            bool is_droppable            = IsDroppable(constant_user);
            visited_constants_[constant] = is_droppable;

            if (!is_droppable) {
                // A constant must be preserved
                return false;
            }
        }
    }

    // These classes remain unaccounted
    // llvm::DerivedUser
    // llvm::Operator

    return true;
}

bool Sanitizer::IsMemberOfTargetFunction(const llvm::Instruction& instruction) {
    const llvm::Function* parent_function = instruction.getFunction();
    std::string parent_name               = parent_function->getName().str();

    if (parent_name == function_dump_->name_) {
        return true;
    }

    return false;
}

void Sanitizer::UpdateIRModule(llvm::Module& module) {
    const std::string& path = module.getModuleIdentifier();
    std::error_code error_code;

    llvm::raw_fd_ostream file(path, error_code);
    module.print(file, nullptr);
}

void Sanitizer::Debug(llvm::Module& module) {
    llvm::raw_ostream& stdout_ = llvm::outs();
    module.print(stdout_, nullptr);
}
