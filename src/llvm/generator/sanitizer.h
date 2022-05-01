#ifndef AUTOFUZZ_SANITIZER_H
#define AUTOFUZZ_SANITIZER_H

#include "module.h"

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Constants.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Verifier.h>

class Sanitizer : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    explicit Sanitizer(const std::shared_ptr<Function>&,
                                                       bool&, bool deep = true);

    // Explicitly specify what kind of pass has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    const std::shared_ptr<Function>& function_dump_;
    llvm::Function*                  target_function_;

    bool&                            success_;
    bool                             deep_;

    void SanitizeModule(llvm::Module&);
    void FindGlobalsToDelete(llvm::Module&, std::set<llvm::GlobalVariable*>&);
    void FindFunctionsToDelete(llvm::Module&, std::set<llvm::Function*>&);

    bool IsNative(const llvm::GlobalVariable&);
    bool DigIntoConstant(const llvm::ConstantExpr*);
    bool IsFunctionMember(const llvm::Instruction&);

    void ResolveLinkage();

    bool IsModuleValid(llvm::Module&);
    void UpdateIRModule(llvm::Module&);
    void Debug(llvm::Module&);
};


#endif //AUTOFUZZ_SANITIZER_H
