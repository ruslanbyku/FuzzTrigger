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

class Sanitizer  : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    explicit Sanitizer(const std::unique_ptr<Function>&);

    // Explicitly specify what kind of pass has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    const std::unique_ptr<Function>& function_dump_;

    void SanitizeModule(llvm::Module&);
    void FindGlobalsToDelete(llvm::Module&, std::set<llvm::GlobalVariable*>&);
    void FindFunctionsToDelete(llvm::Module&, std::set<llvm::Function*>&);
    void FindStringLiterals(const llvm::Function&,
                            std::set<const llvm::GlobalVariable*>&);

    void UpdateIRModule(llvm::Module&);

    void Debug(llvm::Module&);
};


#endif //AUTOFUZZ_SANITIZER_H
