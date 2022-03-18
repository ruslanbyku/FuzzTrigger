#include "sanitizer.h"

char Sanitizer::ID = 0;

Sanitizer::Sanitizer(const std::unique_ptr<Function>& function_dump)
: llvm::ModulePass(ID), function_dump_(function_dump),
target_function_(nullptr) {}

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

    UpdateIRModule(module);

    return true;
}

void Sanitizer::SanitizeModule(llvm::Module& module) {
    std::set<llvm::GlobalVariable*> global_dumpster;
    std::set<llvm::Function*>       function_dumpster;

    FindGlobalsToDelete(module, global_dumpster);
    for (llvm::GlobalVariable* global: global_dumpster) {
        global->eraseFromParent();
    }

    FindFunctionsToDelete(module, function_dumpster);
    for (llvm::Function* function: function_dumpster) {
        function->eraseFromParent();
    }

    //Debug(module);

    ResolveLinkage();
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
                if (const llvm::Instruction* instruction =
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
    std::set<const llvm::GlobalVariable*> native_string_literals;
    FindStringLiterals(*target_function_, native_string_literals);


    llvm::SymbolTableList<llvm::GlobalVariable>& global_list =
                                                         module.getGlobalList();
    for (llvm::GlobalVariable& global: global_list) {
        bool native = false;

        // Global variable is NOT a string literal
        if (!global.isConstant()) {
            for (const llvm::User* user: global.users()) {
                if (const llvm::Instruction* instruction =
                                      llvm::dyn_cast<llvm::Instruction>(user)) {
                    // Get the function which the instruction belongs to
                    const llvm::Function* parent_function =
                            instruction->getFunction();
                    std::string parent_name = parent_function->getName().str();

                    if (parent_name == function_dump_->name_) {
                        native = true;
                    }
                } else { // Can not identify a global object
                    // Do not delete, lest to break the dependency
                    native = true;
                }
            }
        } else { // Global variable is a string literal
            if (native_string_literals.contains(&global)) {
                native = true;
            }
        }

        if (!native) {
            global_dumpster.insert(&global);
        }
    }
}

void Sanitizer::FindStringLiterals(const llvm::Function& function,
                       std::set<const llvm::GlobalVariable*>& string_literals) {
    for (const llvm::BasicBlock& basic_block: function) {
        for (const llvm::Instruction& instruction: basic_block) {
            for (const llvm::Use& operand: instruction.operands()) {

                // Check if an operand is GetElementPtr
                if (llvm::GEPOperator* gep_operand =
                        llvm::dyn_cast<llvm::GEPOperator>(operand)) {

                    if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(
                            gep_operand->getPointerOperand())) {
                        string_literals.insert(global);
                    }

                    for (auto ii = gep_operand->idx_begin();
                                           ii != gep_operand->idx_end(); ++ii) {
                        if (auto* global =
                                llvm::dyn_cast<llvm::GlobalVariable>(*ii)) {
                            string_literals.insert(global);
                        }
                    }

                }
            }
        }
    }
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
