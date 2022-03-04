#ifndef AUTOFUZZ_PASS_LAUNCHER_H
#define AUTOFUZZ_PASS_LAUNCHER_H

#include "analysis.h"

#include <llvm/Pass.h>
#include <llvm/PassRegistry.h>
#include <llvm/InitializePasses.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

class PassLauncher {
public:
    // --------------------------------------------------------------------- //
    //                              Load IR file                             //
    // --------------------------------------------------------------------- //
    static void LaunchOnIRModule(const std::string& IR_module,
                                        std::unique_ptr<Module>& module_dump) {
        llvm::SMDiagnostic error;
        llvm::LLVMContext context;

        // Load IR text representation into memory
        std::unique_ptr<llvm::Module> module(
                llvm::parseIRFile(IR_module, error, context));
        if (!module) {
            fprintf(stderr, "Could not open [%s]\n", IR_module.c_str());
            return;
        }

        // Tell the pass that analysis operations (CallGraph) will be
        // done further
        llvm::PassRegistry* passReg = llvm::PassRegistry::getPassRegistry();
        llvm::initializeAnalysis(*passReg);

        // Register the pass and run it
        llvm::legacy::PassManager pass_manager;
        pass_manager.add(new Analysis(module_dump));
        pass_manager.run(*module);
    }
};


#endif //AUTOFUZZ_PASS_LAUNCHER_H
