#include "sanitizer.h"

char Sanitizer::ID = 0;

Sanitizer::Sanitizer(const std::unique_ptr<Function>& function_dump,
                     bool& status, bool deep)
: llvm::ModulePass(ID), function_dump_(function_dump),
target_function_(nullptr), success_(status), deep_(deep) {}

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

    SanitizeModule(module);

    // !IMPORTANT!
    // Check if the sanitization process went successful
    // Segmentation fault might interrupt the program
    success_ = IsModuleValid(module);
    if (!success_) {
        return false;
    }

    //Debug(module);
    UpdateIRModule(module);

    return true;
}

void Sanitizer::SanitizeModule(llvm::Module& module) {
    std::set<llvm::Function*>       function_dumpster;

    if (deep_) {
        std::set<llvm::GlobalVariable*> global_dumpster;

        FindGlobalsToDelete(module, global_dumpster);
        for (llvm::GlobalVariable* global: global_dumpster) {
            global->eraseFromParent();
        }
    }

    FindFunctionsToDelete(module, function_dumpster);
    for (llvm::Function* function: function_dumpster) {
        function->eraseFromParent();
    }

    ResolveLinkage();
}

// true  - An error is present
// false - The module is okay
bool Sanitizer::IsModuleValid(llvm::Module& module) {
    return !llvm::verifyModule(module);
}

void Sanitizer::UpdateIRModule(llvm::Module& module) {
    const std::string& path = module.getModuleIdentifier();
    std::error_code error_code;

    llvm::raw_fd_ostream file(path, error_code);
    module.print(file, nullptr);
}

void Sanitizer::FindFunctionsToDelete(llvm::Module& module,
        std::set<llvm::Function*>& function_dumpster) {
    for (llvm::Function& function: module) {
        if (&function == target_function_) {
            continue;
        }

        bool native = false;
        // Not local functions, marked as "declare", some might be interesting
        if (function.isDeclaration()) {

            for (const llvm::User* user: function.users()) {
                if (const auto* instruction =
                                      llvm::dyn_cast<llvm::Instruction>(user)) {
                    // Get the function which the instruction belongs to
                    const llvm::Function* parent_function =
                            instruction->getFunction();
                    std::string parent_name = parent_function->getName().str();

                    if (parent_name == function_dump_->name_) {
                        native = true;
                    }
                }
            }
        }

        // Local functions delete by default
        if (!native) {
            function_dumpster.insert(&function);
        }
    }
}

void Sanitizer::FindGlobalsToDelete(llvm::Module& module,
                       std::set<llvm::GlobalVariable*>& global_dumpster) {
    llvm::SymbolTableList<llvm::GlobalVariable>& global_list =
                                                         module.getGlobalList();
    std::set<const llvm::GlobalVariable*> native_globals;

    for (llvm::GlobalVariable& global: global_list) {
        if (IsNative(global)) {
            native_globals.insert(&global);
        }
    }

    // Sort out native from non-native globals
    for (llvm::GlobalVariable& global: global_list) {
        if (!native_globals.contains(&global)) {

            global_dumpster.insert(&global);
        }
    }
}

bool Sanitizer::IsNative(const llvm::GlobalVariable& global) {
    for (const llvm::User* global_user: global.users()) {
        // Global variable is NOT a string literal
        if (const auto* instruction =
                               llvm::dyn_cast<llvm::Instruction>(global_user)) {
            return IsFunctionMember(*instruction);
        }

        // Global variable is a string literal
        if (const auto* constant_expression =
                              llvm::dyn_cast<llvm::ConstantExpr>(global_user)) {
            return DigIntoConstant(constant_expression);
        }
    }

    // A global variable was not identified, do not delete lest to break some
    // dependencies
    return true; // true by default
}

bool Sanitizer::DigIntoConstant(const llvm::ConstantExpr* constant_expression) {
    for (const auto* constant_expression_user: constant_expression->users()) {
        if (const auto* instruction =
                  llvm::dyn_cast<llvm::Instruction>(constant_expression_user)) {
            //
            return IsFunctionMember(*instruction);
        //
        } else if (auto* constant_body =
                     llvm::dyn_cast<llvm::Constant>(constant_expression_user)) {
            for (const llvm::User* constant_value: constant_body->users()) {
                //
                if (auto* global =
                         llvm::dyn_cast<llvm::GlobalVariable>(constant_value)) {
                    // @__const.BufferOverRead.items
                    //
                    return IsNative(*global);
                }
            }
        }
    }

    // An unexpected branch
    return true; // true by default
}

bool Sanitizer::IsFunctionMember(const llvm::Instruction& instruction) {
    const llvm::Function* parent_function = instruction.getFunction();
    std::string parent_name = parent_function->getName().str();

    if (parent_name == function_dump_->name_) {
        return true;
    }

    return false;
}


using Linkage = llvm::GlobalValue::LinkageTypes;

void Sanitizer::ResolveLinkage() {
    if (function_dump_->linkage_ != INTERNAL_LINKAGE) {
        return;
    }

    target_function_->setLinkage(Linkage::ExternalLinkage);
}

void Sanitizer::Debug(llvm::Module& module) {
    llvm::raw_ostream& stdout_ = llvm::outs();
    module.print(stdout_, nullptr);
}
