#include <llvm/Pass.h>
#include "llvm/PassRegistry.h"
#include "llvm/InitializePasses.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"

#include <llvm/IR/InstIterator.h> // Allows to skip BB
#include <llvm/IR/CFG.h>
#include <llvm/IR/User.h>

// https://stackoverflow.com/questions/33356876/try-to-use-llvm-looppass-to-find-number-of-loops-in-program
// https://stackoverflow.com/questions/26546332/llvm-storing-loop-in-stdvector
// https://stackoverflow.com/questions/50138271/detect-if-basicblock-is-in-a-loop
// https://stackoverflow.com/questions/69361884/how-can-i-check-if-a-llvm-instruction-is-inside-a-loop
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/PostDominators.h>
#include "llvm/Analysis/CallGraph.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IRReader/IRReader.h"

#include <cstdio>
#include <memory>
#include <cstdint>
#include <map>

// https://eli.thegreenplace.net/2013/09/16/analyzing-function-cfgs-with-llvm

class EnumFunctions : public llvm::ModulePass {
public:
    static char ID;                                 // pass ID

    EnumFunctions() : llvm::ModulePass(ID) {
        fprintf(stdout, "[1] EnumFunctions module started.\n");
    }

    virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const override {
        AU.setPreservesAll(); // preserves all previous passes
        AU.addRequired<llvm::CallGraphWrapperPass>();
    }

    bool runOnModule(llvm::Module& M) override {
        // getModuleIdentifier() - module name
        // getSourceFileName() - module source file
        // getNumNamedValues() - return the num of global values
        // getFunction(StringRef) - return Function* by its name
        // getNamedValue(StringRef Name) - return GlobalVariable* by its name
        // getGlobalList() - return const GlobalListType&
        // getFunctionList() - return const FunctionListType&
        fprintf(stdout, "[2] Module name: %s\n", M.getName().data());
        fprintf(stdout, "[3] Module source file name: %s\n\n", M.getSourceFileName().data());

        /*
        llvm::CallGraph& callGraph = llvm::Pass::getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();
        // Search the call graph for root functions
        for (llvm::CallGraph::iterator ii = callGraph.begin(); ii != callGraph.end(); ++ii) {
            // Iterator's type is FunctionMapTy: map<const Function*, unique_ptr<CallGraphNode>>
            llvm::CallGraphNode* callGraphNode = ii->second.get();
            llvm::Function* func = callGraphNode->getFunction();

            if (callGraphNode->getNumReferences() > 1 || func == nullptr) {
                if (func) {
                    // All skipped functions have at least 2 references in the file, which basically denotes that
                    // they are mentioned 2 times in the file.
                    fprintf(stdout, "Skipping function -> %s{%d}\n", func->getName().data(), callGraphNode->getNumReferences());
                    fprintf(stdout, "External linkage: %s\n\n", func->hasExternalLinkage() ? "yes" : "no");
                }
                continue; // keep only "root" nodes
            }

            // main() is the ONLY function with 1 number of reference, supposing that it is the root
            fprintf(stdout, "= Function name: %s{%d}\n", func->getName().data(), callGraphNode->getNumReferences());
            // All functions in the file have an external linkage
            fprintf(stdout, "External linkage: %s=\n\n", func->hasExternalLinkage() ? "yes" : "no");
        }
         */

        // Get a Function object and iterate over it
        for (llvm::Module::const_iterator ii = M.begin(); ii != M.end(); ++ii) {
            // GlobalValue
            //      |
            // GlobalObject
            //      |
            // Function
            // Make sure a function body has a body
            if (!ii->isDeclaration()) {
                // Make sure that is externally visible
                if (ii->getLinkage() == llvm::GlobalValue::LinkageTypes::ExternalLinkage) {
                    const llvm::Function& function = *ii;
                    // analyzeFunction(function);
                    // fprintf(stdout, "\tBasic blocks:\n");
                    for (const llvm::BasicBlock& basic_block: *ii) {

                        /*
                        // Watches previous basic blocks
                        for (const llvm::BasicBlock* predecessor: llvm::predecessors(&basic_block)) {
                            llvm::errs() << " +=+=+ predecessor" << *predecessor << " +=+=+\n";
                        }

                        // Watches future basic blocks
                        for (const llvm::BasicBlock* successors: llvm::successors(&basic_block)) {
                            llvm::errs() << " !!!!!! successors" << *successors << " !!!!!!\n";
                        }
                         */

                        // fprintf(stdout, "\t\t# of IR instr: %ld\n", basic_block.size());
                        for (const llvm::Instruction& instruction: basic_block) {

                            // Count opcodes
                            if (opcode_counter_.find(instruction.getOpcodeName()) == opcode_counter_.end()) {
                                opcode_counter_[instruction.getOpcodeName()] = 1;
                            } else {
                                ++opcode_counter_[instruction.getOpcodeName()];
                            }

                            /*
                            fprintf(stdout, "\t\t\tOpcode name: %s\n", instruction.getOpcodeName());
                            fprintf(stdout, "\t\t\tNumber of operands: %d\n", instruction.getNumOperands());
                            if (instruction.getNumOperands() > 0) {
                                fprintf(stdout, "\t\t\tOperand value: ");
                                for (std::uint32_t i = 0; i < instruction.getNumOperands(); ++i) {
                                    fprintf(stdout, "%s ", instruction.getOperand(i)->getName().data());
                                }
                                fprintf(stdout, "\n");
                            }

                            if (const llvm::CallInst* call_instruction = llvm::dyn_cast<llvm::CallInst>(&instruction)) {
                                const llvm::Function* callee = call_instruction->getCalledFunction();
                                fprintf(stdout, "\t\t\t\tCalled function: %s\n", callee->getName().data()); // root.cpp:134
                            }
                             */

                            /*
                            //
                            // User Value
                            // What is this instruction using?
                            // Print Value (function) name from the User (instruction)
                            // Iterates over the operands of an instruction
                            for (llvm::User::const_op_iterator kk = instruction.op_begin(); kk != instruction.op_end(); ++kk) {
                                llvm::Value* operand = *kk;
                                // ...
                                // ***exit
                                // ***
                                // ...
                                // ***un_init
                                // ...
                                llvm::errs()<< "***" << operand->getName()<< "\n";

                                if (const llvm::ConstantInt* ci = llvm::dyn_cast<llvm::ConstantInt>(kk)) {
                                    llvm::errs() << *ci << "\n";
                                }
                            }
                            // ==
                            */

                            /*
                            for (const llvm::Use& U: instruction.operands()) {
                                if (const llvm::Instruction* nxt = llvm::dyn_cast<llvm::Instruction>(U)) {
                                    llvm::errs() << "+ " << *nxt << "\n";
                                }
                            }

                            // Do not know what it does
                            for (llvm::Value::const_user_iterator kk = instruction.user_begin(); kk != instruction.user_end(); ++kk) {
                                if (const llvm::Instruction* nxt = llvm::dyn_cast<llvm::Instruction>(*kk)) {
                                    llvm::errs() << "{} " << *nxt << "\n";
                                }
                            }
                             */
                            //
                            //
                        }
                    }
                }
            }
        }

        std::map<std::string, uint64_t>::iterator start = opcode_counter_.begin();
        std::map<std::string, uint64_t>::iterator end = opcode_counter_.end();

        while (start != end) {
            llvm::outs() << start->first << ":" << start->second << "\n";
            ++start;
        }
        opcode_counter_.clear();

        // we didn't modify the module, so return false
        return false;
    }

    virtual llvm::StringRef getPassName() const override {
        return "EnumFunctions";
    }

private:
    std::map<std::string, uint64_t> opcode_counter_;

    void analyzeFunction(const llvm::Function& function) {
        //+ const Function & 	getFunction () const
        //+ Type* getReturnType()
        //+ bool isVarArg() - return true if a func has a variable num of args
        //+ const BasicBlock       &getEntryBlock() const
        //+ size_t arg_size()
        //+ getType()/isVoidTy()/getTypeAllocSize/dataLayout.getTypeAllocSize/getTypeID
        //+ isPointerTy/dyn_cast<PointerType>(structTy)->getElementType()/
        //+ getLinkage()/hasExternalLinkage()
        //+ isDeclaration
        //+ getParent
        //
        //- VisibilityTypes getVisibility()
        //- bool arg_empty()
        //- unsigned  getInstructionCount() - number of instructions in func
        //- FunctionType* getFunctionType()
        //- AttributeList getAttributes() - return the attribute list ???
        //- ProfileCount getEntryCount(bool AllowSynthetic = false) const - return the number
        //of times a function was executed
        //- bool doesNotAccessMemory() const
        //- bool onlyReadsMemory()
        // bool doesNotReadMemory() const - write to memory
        // bool onlyAccessesArgMemory() const - access memory only using pointers based
        //on its arguments
        // bool onlyAccessesInaccessibleMemory() const
        // bool onlyAccessesInaccessibleMemOrArgMem() const
        //- bool doesNotReturn() const
        //- bool doesNotRecurse() const
        //- bool willReturn() const
        //- bool hasStructRetAttr() const
        //- const BasicBlockListType &getBasicBlockList() const

        // 1)
        fprintf(stdout, "Function found: %s\n", function.getName().data());
        // 2)
        fprintf(stdout, "\tArgument number (%): %ld\n", function.arg_size());
        // 3)
        fprintf(stdout, "\tHas variadic arguments: %s\n", function.isVarArg() ? "yes" : "no");
        // 4)
        {
            std::string return_value_string;
            llvm::raw_string_ostream raw(return_value_string);
            llvm::Type* type = function.getReturnType();
            type->print(raw);
            fprintf(stdout, "\tReturn type: %s\n", raw.str().data());
        }
        // 5)
        // What is using this instruction?
        // function is a Value and has its Users (instructions)
        // users() returns iterator_range< const_user_iterator >
        // Iterates over uses of a value
        for (const llvm::User* U: function.users()) {
            if (const llvm::Instruction* Inst = llvm::dyn_cast<llvm::Instruction>(U)) {
                // Value User
                // {is_valid} {%8 = call zeroext i1 @is_valid(i8* %7)"}
                // Ex "The is_valid is used/called in instruction:   %8 = call zeroext i1 @is_valid(i8* %7)"
                llvm::errs() << "The " << function.getName() << " is used/called in instruction: " << *Inst << "\n";
            }
        }

        for (llvm::Function::const_arg_iterator jj = function.arg_begin(); jj != function.arg_end(); ++jj) {
            const llvm::Argument& argument = *jj;

            fprintf(stdout, "\tArgument name: %s\n", argument.getName().data());
            fprintf(stdout, "\tArgument number: %d\n", argument.getArgNo());
            {
                std::string return_value_string;
                llvm::raw_string_ostream raw(return_value_string);
                llvm::Type* type = argument.getType();
                type->print(raw);
                fprintf(stdout, "\tArgument type: %s\n", raw.str().data());
                //auto struct_type = llvm::dyn_cast<llvm::StructType>(&data_type);
                //auto array_type = llvm::dyn_cast<llvm::ArrayType>(&data_type);
            }
            std::string function_name = argument.getParent()->getName().data();
            fprintf(stdout, "\tArgument belongs to: %s\n", function_name.data());
            // Then common.h is messing with arguments (dig.cpp:501)
        }
    }

};

char EnumFunctions::ID = 0;

// Code written here uses so-called Legacy Passes (old version)
// When the code analyzes an IR file it goes strictly from top to bottom (bear that in mind)
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <.ll>\n", argv[0]);
        return 1;
    }

    llvm::SMDiagnostic Err;
    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> module(llvm::parseIRFile(argv[1], Err, Context));
    if (!module) {
        Err.print(argv[0], llvm::errs());
        return 1;
    }

    llvm::PassRegistry &passReg = *llvm::PassRegistry::getPassRegistry();
    // llvm::initializeCore(passReg);
    llvm::initializeAnalysis(passReg); // Analysis module needs it, particularly CallGraphWrapperPass

    llvm::legacy::PassManager pass_manager;
    pass_manager.add(new EnumFunctions());
    pass_manager.run(*module);

    return 0;
}

// InstIterator
// for (Instruction& I: instructions(F)) {
//      switch(I.getOpcode()) {
// case Instruction::BinaryOperator:
// case Instruction::Return: OR use dyn_cast for ReturnInst
// }

// Generate CFG
// $ opt -dot-cfg -disable-output foo.ll
// Display CFG
// dot -Tpng foo.cfg.dot > foo.cfg.png

// Topological order traversal (p239)
// Depth-first traversal (DFS)
// Breadth-first traversal (BFS)
// SSC traversal

// Example of iterating through Users of a Value
//void vulnerable() {
//    v = get_password();
//    ...
//    exploit(v);
//}
// +
//User* find_leakage(CallInst* call_instr) {
//    for (auto* User: call_instr->users()) {
//        if (isa<CallInst>(User)) {
//            return User;
//        }
//    }
//}
// The find_leakage takes a CallInst argument which represents a get_password() function call (in IR). Then I cast
// the CallInst as a Value and iterate through all its Users in the further code. It helps to find the Instruction -
// exploit(v) that calls my Value of a password.

// dyn_cast = isa + cast

// p124
// llvm::TrackingVH class to track value replacements
/*
struct BasicBlockDef {
    llvm::DenseMap<Decl*, llvm::TrackingVH<llvm::Value>> defs;
    llvm::DenseMap<llvm::PHINode*, Decl*> incomplete_phis;
    unsigned sealed = 1;
};
// Where Decl is a declaration of the AST (a variable or a formal parameter)

llvm::DenseMap<llvm::BasicBlock*, BasicBlockDef> current_def;
 */

//uint64_t alloc_size = data_layout.getTypeAllocSize(field_type);
//uint64_t offset = struct_layout->getElementOffset(idx);

/*
 * When working with globals
 *
 * // Definitions have initializers, declarations do not
        if (!var.hasInitializer()) {
            continue;
        }
//llvm::outs() << var.getName() << "\n";
        const llvm::Constant* constant = var.getInitializer();
        if (const llvm::ConstantInt* constInt = llvm::dyn_cast<llvm::ConstantInt>(constant)) {
            value = constInt->getSExtValue();
        }
 *
 *
 *
    llvm::TypeFinder type_finder;
    type_finder.run(module, true);

    // Find global structs
    int n = 0;
    std::string str;
    for (llvm::StructType* type_fnd: type_finder) {
        str = type_fnd->getName().data();
        n = type_fnd->getNumContainedTypes();
        if (str == "struct.A") {
            for (llvm::Type* type: type_fnd->elements()) {
                if (type->isIntegerTy()) {
                    ;
                }
            }
        }
    }

    // These headers were not used
    // #include <llvm/IR/Function.h>
// #include <llvm/IR/Instruction.h>
// #include <llvm/IR/Constants.h>
// #include <llvm/IR/Dominators.h>
// #include <llvm/Analysis/LoopInfo.h>
// #include <llvm/Analysis/PostDominators.h>
// #include <llvm/Support/raw_ostream.h>

 // https://stackoverflow.com/questions/1545080/c-code-file-extension-what-is-the-difference-between-cc-and-cpp
// https://en.cppreference.com/w/cpp/filesystem/path/extension
// https://retrocomputing.stackexchange.com/questions/20281/why-didnt-c-specify-filename-extensions

  int32_t pid = fork();
    if (pid == 0) {
        // Exec command in the child process
        // The terminated pointer must be cast (char *) NULL
        execlp(
                compiler,
                "-emit-llvm",
                source_path_.c_str(),
                "-S",
                "-o",
                ir_path_.c_str(),
                "> /dev/null 2>&1", // &> /dev/null also is valid
static_cast<char*>(nullptr)
);
}
// Wait for the child process to finish
int32_t status;
waitpid(pid, &status, 0);

 // Add include to the example
    const char* code = "#include <cstdint>\n"
                       "int test_func() {\n"
                       "  return 0;\n"
                       "}\n"
                       "\n"
                       "class MyClass {\n"
                       "    int foo;\n"
                       "public:\n"
                       "    void bar() {}\n"
                       "};\n"
                       "\n"
                       "MyClass\n"
                       "foobar() {\n"
                       "    MyClass a;\n"
                       "    return a;\n"
                       "  }";

 //https://stackoverflow.com/questions/19046109/how-can-i-skip-includes-using-libclang
 //https://stackoverflow.com/questions/35483901/clang-3-8-how-to-stop-clang-from-creating-ast-of-system-headers

    llvm::Twine twine(code);
    FunctionLocation location("foobar");
    llvm::tooling::runToolOnCode(std::make_unique<FrontendAction>(location), twine, "your_file_name.cc");

    //compiler.getPreprocessor().SetSuppressIncludeNotFoundError(true);
    //compiler.getDiagnostics().setSuppressSystemWarnings(true);
    //compiler.getDiagnostics().setClient(new llvm::IgnoringDiagConsumer());

    // Search string literals
    if (llvm::GlobalVariable* global = llvm::dyn_cast<llvm::GlobalVariable>(operand)) {
                        std::string p = global->getName().str();
                        llvm::outs() << "1" << p << "\n";
                    }

    */