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

using Visibility = llvm::GlobalValue::VisibilityTypes;
using Linkage    = llvm::GlobalValue::LinkageTypes;

class Sanitizer : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    explicit Sanitizer(const std::shared_ptr<Function>&, bool&);

    // Explicitly specify what kind of pass has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    const std::shared_ptr<Function>& function_dump_;
    llvm::Function*                  target_function_;

    bool&                            success_;

    bool SanitizeModule(llvm::Module&);
    bool IsDroppable(llvm::User*);
    bool IsMemberOfTargetFunction(const llvm::Instruction&);

    void UpdateIRModule(llvm::Module&);
    void Debug(llvm::Module&);
};


#endif //AUTOFUZZ_SANITIZER_H
