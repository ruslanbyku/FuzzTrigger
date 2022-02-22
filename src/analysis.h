#ifndef AUTOFUZZ_ANALYSIS_H
#define AUTOFUZZ_ANALYSIS_H

#include "module.h"
#include "cfg.h"

#include <queue>
#include <map>

#include <llvm/Pass.h>
#include <llvm/PassRegistry.h>
#include <llvm/InitializePasses.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/TypeFinder.h>
#include <llvm/IR/DataLayout.h>
// #include <llvm/IR/Function.h>
// #include <llvm/IR/Instruction.h>
// #include <llvm/IR/Constants.h>
// #include <llvm/IR/Dominators.h>
// #include <llvm/IR/User.h>

// #include <llvm/Analysis/LoopInfo.h>
// #include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/CallGraph.h>

#include <llvm/Support/SourceMgr.h>
// #include <llvm/Support/raw_ostream.h>

#include <llvm/IRReader/IRReader.h>

//
// Make a full analysis of an IR module
//
class Analysis : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    // Receive an IR module file path (.ll)
    explicit Analysis(std::string);

    explicit operator bool() const;
    std::unique_ptr<Module> GetModuleDump();

    // --------------------------------------------------------------------- //
    //                     LLVM specific methods                             //
    // --------------------------------------------------------------------- //
    // Explicitly specify what kind of analysis has to be done on the run.
    void getAnalysisUsage(llvm::AnalysisUsage&) const override;
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    // Set this flag if the pass has worked out
    bool                              success_;
    // llvm main module
    llvm::Module*                     module_;
    // Module auxiliary information
    std::unique_ptr<llvm::DataLayout> data_layout_;
    // An object for dump of an IR from the memory
    std::unique_ptr<Module>           module_dump_;
    // A function that calls every function in the module
    const llvm::Function*             root_function_;
    // CFG of module functions
    // The order of objects in the container is the order of how functions
    // are called in the module
    std::vector<std::unique_ptr<CFG>> module_cfg_;
    std::set<const llvm::Function*>   standalone_functions_;

    void LaunchPassOnIRModule(std::string&&);
    void DumpModuleStructs();
    std::unique_ptr<Function> DumpModuleFunction(const llvm::Function&);
    std::unique_ptr<Argument> DumpFunctionArgument(const llvm::Argument&);

    std::unique_ptr<Type> ResolveValueType(llvm::Type*);
    std::unique_ptr<Type> ResolveIntegerType(llvm::Type*, llvm::Type*);
    std::unique_ptr<Type> ResolveStructType(llvm::Type*, llvm::Type*);

    const llvm::Function* GetRootFunction() const;
    void MakeControlFlowGraph(const llvm::Function&);
    void FindStandaloneFunctions();
};

#endif //AUTOFUZZ_ANALYSIS_H
