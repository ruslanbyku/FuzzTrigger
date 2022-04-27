#ifndef AUTOFUZZ_ANALYSIS_H
#define AUTOFUZZ_ANALYSIS_H

#include "module.h"
#include "cfg.h"

#include <algorithm>
#include <queue>
#include <map>

#include <llvm/Pass.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/TypeFinder.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/Analysis/CallGraph.h>

using FunctionCFGPtr   = std::unique_ptr<CFG<llvm::Function>>;
using BasicBlockCFGPtr = std::unique_ptr<CFG<llvm::BasicBlock>>;

//
// Make a full analysis of an IR module
//
class Analysis : public llvm::ModulePass {
public:
    static char ID;  // Declare for Pass internal operations

    // Receive an IR module file path (.ll)
    explicit Analysis(std::unique_ptr<Module>&);

    // --------------------------------------------------------------------- //
    //                     LLVM specific methods                             //
    // --------------------------------------------------------------------- //
    // Explicitly specify what kind of analysis has to be done on the run.
    llvm::StringRef getPassName() const override;
    // Process the module on invocation
    bool runOnModule(llvm::Module&) override;

private:
    // An object for dump of an IR from the memory
    std::unique_ptr<Module>&          module_dump_;

    std::unique_ptr<llvm::DataLayout> data_layout_;

    std::set<std::string>             visited_structs_;

    // CFG of all functions in the module
    std::vector<FunctionCFGPtr>       module_cfg_;
    // CFG of all basic blocks for each function (in the module)
    std::vector<BasicBlockCFGPtr>     function_cfg_;

    std::set<const llvm::Function*>   standalone_functions_;

    // Main function
    void DumpModule(llvm::Module&);

    // Helpers to the main function
    bool IsModuleLegit(const std::string&, uint64_t) const;
    bool TraverseModule(llvm::Module&);
    bool DumpModuleStructs(llvm::Module&);
    bool DumpModuleFunctions(llvm::Module&, const std::vector<FunctionCFGPtr>&);
    std::unique_ptr<Function> DumpSingleFunction(const llvm::Function&);
    bool DumpFunctionArguments(const llvm::Function&,
                               std::unique_ptr<Function>&);
    std::unique_ptr<Argument> DumpSingleArgument(const llvm::Argument&);

    FunctionLinkage GetFunctionLinkage(llvm::GlobalValue::LinkageTypes) const;

    // CFG related stuff
    std::vector<const llvm::Function*> GetRootFunctions(llvm::Module&) const;
    uint32_t MakeControlFlowGraph(const llvm::Function&,
                                  const llvm::Function* = nullptr,
                                  uint32_t = 0);
    void PrintCFG() const;

    // Type variable stuff
    std::unique_ptr<Type> ResolveValueType(llvm::Type*);
    std::unique_ptr<Type> ResolveIntegerType(llvm::Type*, llvm::Type*);
    std::unique_ptr<Type> ResolveStructType(llvm::Type*, llvm::Type*);

    // Discover standalone functions
    std::vector<const llvm::GlobalVariable*> GetLocalGlobals(llvm::Module&);
    std::set<const llvm::Function*> FindStandaloneFunctions(
                                           llvm::Module&, const AdjacencyList&);
    bool IsStandalone(const llvm::Function&);
};

#endif //AUTOFUZZ_ANALYSIS_H
