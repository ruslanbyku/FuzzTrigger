// Copyright 2022 Ruslan Byku
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "analysis.h"

char Analysis::ID = 0;

Analysis::Analysis(std::unique_ptr<Module>& module_dump)
: llvm::ModulePass(ID), module_dump_(module_dump){}


llvm::StringRef Analysis::getPassName() const {
    return "ModuleAnalysis";
}

// ------------------------------------------------------------------------- //
//                              Run Pass manager                             //
// ------------------------------------------------------------------------- //
bool Analysis::runOnModule(llvm::Module& module) {
    // Explicitly redefine the success flag
    module_dump_->success_ = false;

    // --------------------------------------------------------------------- //
    //                 Prepare module's auxiliary information                //
    // --------------------------------------------------------------------- //
    // Need to resolve data types
    data_layout_     = std::make_unique<llvm::DataLayout>(&module);
    // Need to resolve standalone functions
    special_globals_ = GetModuleSpecialGlobals(module);

    if (!data_layout_) {
        // Some data was not initialized, abort
        return false;
    }

    // --------------------------------------------------------------------- //
    //                        Dump the whole module                          //
    // --------------------------------------------------------------------- //
    DumpModule(module);

    // The module has not been modified, then return false
    return false;
}

std::vector<const llvm::GlobalVariable*>
Analysis::GetModuleSpecialGlobals(llvm::Module& module) {

    std::vector<const llvm::GlobalVariable*> special_globals;
    const llvm::SymbolTableList<llvm::GlobalVariable>& global_list =
                                                         module.getGlobalList();

    // By my observations there are 3 main groups of globals:
    // 1) String literals             (-)
    // 2) dso_local globals           (+)
    // 3) rest: stdout, stdin, stderr (-)
    for (const llvm::GlobalVariable& global: global_list) {

        if (global.isConstant()) {
            // String literal, leave it
            continue;
        }

        // Not sure about the accuracy of this filter
        if (global.isDSOLocal()) {
            // dso_local
            special_globals.push_back(&global);
        }
    }

    return special_globals;
}

void Analysis::DumpModule(llvm::Module& module) {
    bool result;

    // --------------------------------------------------------------------- //
    //                         General module data                           //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Extract general information.";
    }

    // Dump module name
    module_dump_->name_             = module.getModuleIdentifier();
    // Dump module corresponding source file
    module_dump_->source_name_      = module.getSourceFileName();
    // Dump number of functions (internal + external)
    module_dump_->functions_number_ = module.getFunctionList().size();

    // --------------------------------------------------------------------- //
    //                      Check module's legitimacy                        //
    // --------------------------------------------------------------------- //
    result = IsModuleLegit(module_dump_->source_name_,
                                               module_dump_->functions_number_);
    if (!result) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "The analyzing module seems "
                                    "invalid. Abort.";
        }

        // The module structure is not acceptable
        module_dump_->success_ = false;

        return;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "The analyzing module seems valid. Proceed.";
    }

    // --------------------------------------------------------------------- //
    //                            Construct CFGs                             //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Construct CFGs for the module.";
    }

    result = TraverseModule(module);
    if (!result) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "Can not construct CFGs for the "
                                    "module. Abort.";
        }

        // Can not traverse the module
        module_dump_->success_ = false;

        return;
    }

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "CFGs constructed.";
    }

    // ---------------------------------------------------------------------- //
    //                 Find module's standalone functions                     //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Find standalone functions.";
    }
    standalone_functions_ = FindModuleStandaloneFunctions(module_cfg_);

    // Save the number of standalone functions
    module_dump_->standalone_funcs_number_ = standalone_functions_.size();

    if (standalone_functions_.empty()) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_WARNING) << "There are no standalone functions "
                                      "in the module. Abort.";
        }

        // If there are no standalone functions in the module, there is no
        // use continuing the program execution
        module_dump_->success_ = false;

        return;
    }

    // At this point there are standalone functions

    if (LOGGER_ON) {
        if (module_dump_->standalone_funcs_number_ == 1) {
            LOG(LOG_LEVEL_INFO) << "There is 1 standalone function:";
        } else {
            LOG(LOG_LEVEL_INFO) << "There are "
                                << module_dump_->standalone_funcs_number_
                                << " standalone functions:";
        }

        for (const auto* function: standalone_functions_) {
            LOG(LOG_LEVEL_INFO) << "\t" << function->getName().str();
        }
    }

    // ---------------------------------------------------------------------- //
    //                     Dig into module's functions                        //
    // ---------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Dump module functions.";
    }

    // Dump local functions
    module_dump_->functions_ = DumpModuleFunctions(module_cfg_);

    if (module_dump_->functions_.empty()) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "An error occurred while "
                                    "dumping module's functions. Abort.";
        }

        // An error occurred while dumping module's functions. At this point
        // they must be, but who knows. This is a contingency branch.
        module_dump_->success_ = false;

        return;
    }

    // Create a separate storage dump for just only standalone functions
    for (auto& function_dump: module_dump_->functions_) {
        if (!function_dump->is_standalone_) {
            continue;
        }

        module_dump_->standalone_functions_.push_back(function_dump);
    }

    uint64_t standalone_functions_found =
            module_dump_->standalone_funcs_number_;
    uint64_t standalone_functions_in_storage =
            module_dump_->standalone_functions_.size();

    if (standalone_functions_found != standalone_functions_in_storage) {
        // Something went wrong
        //
        // The number of found standalone functions does not match with the
        // number of dumps of standalone function
        module_dump_->success_ = false;

        return;
    }

    if (LOGGER_ON) {
        uint64_t internal_functions_number = module_dump_->functions_.size();

        LOG(LOG_LEVEL_INFO) << internal_functions_number
                            << "/"
                            << module_dump_->functions_number_
                            << "(local-internal/all) functions found.";
    }

    // --------------------------------------------------------------------- //
    //                          Dump module structs                          //
    // --------------------------------------------------------------------- //
    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Dump module structures.";
    }

    module_dump_->structs_ = DumpModuleStructs(module);

    if (LOGGER_ON) {
        if (module_dump_->structs_.empty()) {
            // Not a problem
            LOG(LOG_LEVEL_INFO) << "No structures found.";
        } else {
            uint64_t structs_number = module_dump_->structs_.size();

            if (structs_number == 1) {
                LOG(LOG_LEVEL_INFO) << "1 structure found.";
            } else {
                LOG(LOG_LEVEL_INFO) << structs_number << " structures found.";
            }
        }
    }

    // --------------------------------------------------------------------- //
    //                          Dump module globals                          //
    // --------------------------------------------------------------------- //
    //TODO: Dump module globals

    // Analysis went successful, module dump is complete
    module_dump_->success_ = true;
}

bool Analysis::IsModuleLegit(const std::string& source_name,
                                             uint64_t functions_number) const {
    // Check if the IR module has a header, specifically the
    // 'source_filename' field
    if (source_name.empty()) {
        // Can not identify the corresponding 'source_filename'
        //
        // Actually, do not know if it is right to rely upon this field
        return false;
    }

    // Check if there are functions in the module
    if (functions_number == 0) {
        // There are no functions in the module, quit
        return false;
    }

    return true;
}

bool Analysis::TraverseModule(llvm::Module& module) {
    // A root function calls every function in the module, it is an entry
    // point to the program. There might be many root functions in the module.
    std::vector<const llvm::Function*>
            root_functions = GetRootFunctions(module);

    if (root_functions.empty()) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "No root functions found. Abort.";
        }

        // No root function, do not know how to traverse the module, quit
        return false;
    }

    if (LOGGER_ON) {
        if (root_functions.size() == 1) {
            LOG(LOG_LEVEL_INFO) << "1 root function found.";
        } else {
            LOG(LOG_LEVEL_INFO) << root_functions.size()
                                << " root functions found.";
        }
    }

    for (const auto* root_function: root_functions) {
        MakeControlFlowGraph(*root_function, nullptr, 0);
    }

    //PrintCFG();

    if (module_cfg_.empty() || function_cfg_.empty()) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << "Something went wrong during making CFGs "
                                    "for the module. Abort.";
        }

        // Something went wrong during making CFG, and the containers are empty
        return false;
    }

    return true;
}


//
// Root function is a function that calls every other function in the module
//
std::vector<const llvm::Function*> Analysis::GetRootFunctions(
        llvm::Module& module) const {
    const llvm::CallGraph call_graph(module);
    std::vector<const llvm::Function*>    root_functions;
    std::map<const llvm::Function*, bool> cross_references_to;
    std::set<const llvm::Function*>       function_pointers;

    // Get additional support information about how functions in the module
    // are called
    // Enumerate all functions in the module (the first lap) and find all
    // cross-references to a function and function pointers
    for (const llvm::Function& function: module) {

        if (!cross_references_to.contains(&function)) {
            // If nobody has called this function yet, mark it as a root for now
            cross_references_to[&function] = false;
        }

        for (const auto& instruction: llvm::instructions(function)) {
            // Find call instructions for cross-references
            if (auto caller = llvm::dyn_cast<llvm::CallInst>(&instruction)) {
                // A call instruction is found, take the callee
                const llvm::Function* callee = caller->getCalledFunction();

                if (callee) {  // There is a valid callee in the caller
                    // The current function calls another function (callee)
                    // Mark the callee that it can be already a root
                    cross_references_to[callee] = true;
                }

                // Find store instruction for function pointers
            } else if (auto store_instruction =
                    llvm::dyn_cast<llvm::StoreInst>(&instruction)) {
                // A store instruction is found, take the first operand
                const llvm::Value* operand = store_instruction->getOperand(0);
                const llvm::Type* type     = operand->getType();

                if (!type->isPointerTy() ||
                    !llvm::isa<llvm::PointerType>(type)) {
                    // The operand is not a pointer
                    continue;
                }

                const auto* pointer = llvm::cast<llvm::PointerType>(type);
                if (!pointer->getElementType()->isFunctionTy() ||
                    !llvm::isa<llvm::Function>(operand)) {
                    // A pointer is not a function pointer
                    continue;
                }

                // A function pointer has been found
                if (!operand->getName().empty()) {
                    // The value of the pointer is known at compile time
                    const auto* callee = llvm::cast<llvm::Function>(operand);

                    if (callee) {
                        function_pointers.insert(callee);
                    }
                }
            }
        }
    }

    // Enumerate all functions in the module (the second lap)
    // pair - (llvm::Function*, llvm::CallGraphNode*)
    for (const auto& pair: call_graph) {

        // The first pair is null (nullptr, 0).
        if (!pair.first) {
            continue;
        }

        const llvm::CallGraphNode* call_graph_node = pair.second.get();
        // Get the number of times the function was encountered in the module
        uint32_t references_to_function = call_graph_node->getNumReferences();

        if (references_to_function == 1) {
            // Looks like one root is found, checking it further
            const llvm::Function* function = pair.first;

            // -------------------------------------------------------------- //
            //                           Filter                               //
            // -------------------------------------------------------------- //

            // Find out if the function has a definition
            if (function->isDeclaration()) {
                // The function does not have a definition (body)
                continue;
            }

            // static functions (internal linkage functions) might be falsely
            // recognized as root functions
            if (!function->hasExternalLinkage()) {
                continue;
            }

            // Employ the additional check
            // Find if the function was previously marked as a non-root
            if (cross_references_to[function]) {
                // The function was indeed marked as a non-root
                continue;
            }

            if (function_pointers.contains(function)) {
                // The function is used as a function pointer
                continue;
            }

            root_functions.push_back(function);
        }
    }

    return root_functions;
}

//
// The method works recursively for every function in the module.
// 1. A function is passed to the method, ideally a root function.
// 2. When the root function is passed, a new CFG object is added to the global
// array of CFG objects.
// 3. The first basic block aka entry block of the function is passed to a
// queue for further analysis.
// 4. As the analysis of the entry block goes, new functions are discovered and
// recursively passed to the same method.
// 5. As a result the array of CFG objects is filled in the order of how
// functions are called in the module.
//
uint32_t Analysis::MakeControlFlowGraph(const llvm::Function& function,
                                        const llvm::Function* parent,
                                        uint32_t function_id) {
    // Check if the function has a body
    if (function.empty()) {
        // It does not
        return function_id;
    }

    // --------------------------------------------------------------------- //
    //                             Function CFG                              //
    // --------------------------------------------------------------------- //
    // If the current function is root (the first enter)
    if (!parent) {
        // Create a new adjacency list for a bunch of functions
        module_cfg_.emplace_back(std::make_unique<FunctionCFG>());
    }

    // Extract the most recent created adjacency list
    auto functions_cfg = static_cast<FunctionCFG*>(module_cfg_.back().get());

    // Recursion prevention:
    // 1) if the function calls itself (a -> a)
    // 2) if a vertex for a called function already exists (a -> b -> a)
    if (&function == parent || functions_cfg->GetVertexByObject(&function)) {

        // If an edge exists, then the current function is called more than
        // once within some function, do not register the same edge
        // 0 -> 1, 4, 5, 1, 1
        if (!functions_cfg->EdgeExists(parent, &function)) {
            // The edge does not exist, but the current function already exists
            // It is definitely a recursion
            // Add the edge to the adjacency list
            functions_cfg->AddEdge(parent, &function);
        }

        return function_id;
    }

    // Create a new vertex for a function and add it to the adjacency list
    functions_cfg->AddVertex(function_id, &function);
    // Keep track of each function identification number
    ++function_id;

    // If the current function is not root (the second or more enters)
    if (parent) {
        // Form a new edge (u, v), where:
        // u - a parent function
        // v - the current function
        // Then add the edge to the adjacency list
        functions_cfg->AddEdge(parent, &function);
    }

    // --------------------------------------------------------------------- //
    //                            Basic Block CFG                            //
    // --------------------------------------------------------------------- //
    const llvm::BasicBlock* entry_block   = &function.getEntryBlock();
    std::queue<const llvm::BasicBlock*>     bb_queue;
    std::map<const llvm::BasicBlock*, bool> visited;
    uint32_t                                block_id = 0;

    // Create a new adjacency list for a bunch of basic blocks of the function
    function_cfg_.emplace_back(std::make_unique<BasicBlockCFG>(function));

    // Extract the most recent created adjacency list
    auto bblocks_cfg =
            static_cast<BasicBlockCFG*>(function_cfg_.back().get());

    // Create all vertices for the adjacency list beforehand
    // Add just created vertices to the adjacency list
    for (auto& basic_block: function.getBasicBlockList()) {
        bblocks_cfg->AddVertex(block_id, &basic_block);
        ++block_id;
    }

    // Add the entry block to the queue (start from the first basic block)
    bb_queue.push(entry_block);
    visited[entry_block] = true;

    // Start processing vertices until there is no left
    while (!bb_queue.empty()) {
        const llvm::BasicBlock* block = bb_queue.front();
        if (!block) {
            continue;
        }
        bb_queue.pop();

        // Enter the basic block and search for call instructions
        for (auto ii = block->begin(); ii != block->end(); ++ii) {
            if (auto caller = llvm::dyn_cast<llvm::CallInst>(ii)) {
                // Call instruction is found, take the callee
                const llvm::Function* callee = caller->getCalledFunction();

                if (!callee) { // There is no callee in the caller
                    continue;
                }

                // Recursively construct a new CFG for a new function
                function_id =
                          MakeControlFlowGraph(*callee, &function, function_id);
            }
        }

        // Find all the neighbors (successors) for the basic block
        const llvm::Instruction* terminator = block->getTerminator();
        for (uint32_t idx = 0; idx < terminator->getNumSuccessors(); ++idx) {
            const llvm::BasicBlock* successor = terminator->getSuccessor(idx);

            // A neighboring vertex is found, it can form an edge
            bblocks_cfg->AddEdge(block, successor);

            // If all neighbors of the vertex has been previously found, then
            // do not process it again
            if (visited.find(successor) != visited.end()) {
                continue;
            }

            // Mark the vertex as visited and put it in the queue for the
            // further processing
            visited[successor] = true;
            bb_queue.push(successor);
        }
    }

    return function_id;
}

std::set<const llvm::Function*>
        Analysis::FindModuleStandaloneFunctions(
                const std::vector<FunctionCFGPtr>& module_cfg) {
    std::set<const llvm::Function*> module_standalone_functions;

    // Iterate over all adjacency lists of functions
    for (const auto& functions_cfg: module_cfg) {
        const AdjacencyList& func_adjacency_list =
                                              functions_cfg->GetAdjacencyList();

        // Local module functions (external are discarded)
        uint64_t functions_number = func_adjacency_list.size();
        // Check if the adjacency list is empty
        if (functions_number == 0) {
            // Although it is impossible, at least root function is present
            continue;
        }

        // ------------------------------------------------------------------ //
        //             Find standalone functions per adjacency list           //
        // ------------------------------------------------------------------ //
        // Find non-dependant functions (from other functions or global values)
        std::set<const llvm::Function*> standalone_functions =
                   FindStandaloneFunctionsPerAdjacencyList(func_adjacency_list);

        if (standalone_functions.empty()) {
            // There are no standalone functions in the current adjacency list
            continue;
        }

        // Merge newly found standalone functions with the main storage
        std::copy(standalone_functions.begin(),
                  standalone_functions.end(),
                  std::inserter(module_standalone_functions,
                                module_standalone_functions.begin()));
    }

    return module_standalone_functions;
}

std::set<const llvm::Function*>
                   Analysis::FindStandaloneFunctionsPerAdjacencyList(
                                          const AdjacencyList& adjacency_list) {
    std::set<const llvm::Function*> standalone_functions;

    // ---------------------------------------------------------------------- //
    //                          Filer by functions                            //
    // ---------------------------------------------------------------------- //
    // The basic filter that identifies whether a function calls other inner
    // functions (local with definition)
    for (const auto& pair: adjacency_list) {
        // // Found a pair with an empty LinkedList [vertex_id -> None]
        if (!pair.second.empty()) {
            continue;
        }

        // Found
        auto vertex = static_cast<Vertex<llvm::Function>*>(pair.first.get());
        standalone_functions.insert(vertex->object_);
    }


    // ---------------------------------------------------------------------- //
    //                           Filer by globals                             //
    // ---------------------------------------------------------------------- //
    // Some functions might be dependant to global variables which do not
    // make them standalone anymore
    for (const llvm::GlobalVariable* global: special_globals_) {

        for (const llvm::User* user: global->users()) {

            if (const llvm::Instruction* instruction =
                    llvm::dyn_cast<llvm::Instruction>(user)) {
                // Get the function which the instruction belongs to
                const llvm::Function* parent_function =
                                                     instruction->getFunction();

                if (standalone_functions.contains(parent_function)) {
                    standalone_functions.erase(parent_function);
                }
            }
        }
    }

    return standalone_functions;
}

bool Analysis::IsStandalone(const llvm::Function& function) {
    return standalone_functions_.contains(&function);
}

std::vector<std::shared_ptr<Function>>
               Analysis::DumpModuleFunctions(
                       const std::vector<FunctionCFGPtr>& module_cfg) {
    std::vector<std::shared_ptr<Function>> module_functions;
    std::set<const llvm::Function*>        registered_functions;

    // Iterate over all adjacency lists of functions
    // A function may be encountered more than twice in the chains of
    // adjacency lists
    for (const auto& functions_cfg: module_cfg) {
        const AdjacencyList& func_adjacency_list =
                                              functions_cfg->GetAdjacencyList();

        // Local module functions (external are discarded)
        uint64_t functions_number = func_adjacency_list.size();
        // Check if the adjacency list is empty
        if (functions_number == 0) {
            // Although it is impossible, at least root function is present
            continue;
        }

        // ------------------------------------------------------------------ //
        //         Dump all functions that are in the adjacency list          //
        // ------------------------------------------------------------------ //
        // Iterate over module's local functions and dump them
        for (const auto& pair: func_adjacency_list) {
            // Extract a function from the adjacency list ([u], v)
            auto vertex =
                         static_cast<Vertex<llvm::Function>*>(pair.first.get());
            const llvm::Function& function = *vertex->object_;

            // Check if the found function was previously registered
            if (registered_functions.contains(&function)) {
                continue;
            }

            // Register a new encountered function
            registered_functions.insert(&function);

            // Start the analysis process of a function
            std::shared_ptr<Function> function_dump =
                                                   DumpSingleFunction(function);

            // Append the dumped function to the vector of module functions
            module_functions.push_back(function_dump);
        }
    }

    return module_functions;
}

std::shared_ptr<Function>
        Analysis::DumpSingleFunction(const llvm::Function& function) {
    llvm::Type* return_type;
    std::shared_ptr<Function> function_dump = std::make_shared<Function>();

    // ---------------------------------------------------------------------- //
    //                     Dump general function data                         //
    // ---------------------------------------------------------------------- //
    function_dump->name_             = function.getName().str();

    function_dump->arguments_number_ =
            static_cast<uint16_t>(function.arg_size());

    function_dump->arguments_fixed_  = !function.isVarArg();

    return_type                      = function.getReturnType();
    function_dump->return_type_      = ResolveValueType(return_type);

    function_dump->is_standalone_    = IsStandalone(function);

    // All functions must be local-internal
    //function_dump->is_local_       = !function.isDeclaration();

    if (function_dump->arguments_number_ == 0) {
        // There are no function arguments, not a problem
        return function_dump;
    }

    // ---------------------------------------------------------------------- //
    //                       Dump function arguments                          //
    // ---------------------------------------------------------------------- //
    function_dump->arguments_ =
            DumpFunctionArguments(function, function_dump->arguments_number_);

    return function_dump;
}

std::vector<std::unique_ptr<Argument>>
                     Analysis::DumpFunctionArguments(
                             const llvm::Function& function,
                             uint16_t arguments_number) {
    std::vector<std::unique_ptr<Argument>> function_arguments;

    // Check if the current function has arguments
    if (arguments_number == 0) {
        // There are no arguments in the current function, not a problem
        return function_arguments;
    }

    // Allocate space in vector for function arguments
    function_arguments.reserve(arguments_number);

    // Iterate over function's arguments
    for (llvm::Function::const_arg_iterator ii = function.arg_begin();
                                            ii != function.arg_end(); ++ii) {
        const llvm::Argument& argument          = *ii;
        std::unique_ptr<Argument> argument_dump = DumpSingleArgument(argument);

        // Append the dumped argument to the vector of function arguments
        function_arguments.push_back(std::move(argument_dump));
    }

    return function_arguments;
}

std::unique_ptr<Argument>
        Analysis::DumpSingleArgument(const llvm::Argument& argument) {
    std::unique_ptr<Argument> argument_dump = std::make_unique<Argument>();
    llvm::Type* argument_type;

    argument_dump->index_ = static_cast<int16_t>(argument.getArgNo());

    argument_type         = argument.getType();

    argument_dump->type_  = ResolveValueType(argument_type);

    return argument_dump;
}

std::unique_ptr<Type> Analysis::ResolveValueType(llvm::Type* data_type) {
    llvm::Type* base_type = data_type;
    uint8_t pointer_depth = 0;
    std::unique_ptr<Type>   type_dump;

    // ---------------------------------------------------------------------- //
    //                        Dig to the base type                            //
    // ---------------------------------------------------------------------- //
    // If there are pointers, then bring up a base type
    while (base_type->isPointerTy()) {
        ++pointer_depth;
        base_type =
                 llvm::dyn_cast<llvm::PointerType>(base_type)->getElementType();
    }

    // ---------------------------------------------------------------------- //
    //                        Resolve the base type                           //
    // ---------------------------------------------------------------------- //
    if (base_type->isVoidTy()) {

        type_dump                   = std::make_unique<Type>();
        type_dump->base_type_       = TYPE_VOID;
        type_dump->allocation_size_ = 0;

    } else if (base_type->isIntegerTy()) {

        type_dump = ResolveIntegerType(data_type, base_type);

    } else if (base_type->isFloatTy()) {

        type_dump                   = std::make_unique<Type>();
        type_dump->base_type_       = TYPE_FLOAT;
        type_dump->allocation_size_ = data_layout_->getTypeAllocSize(data_type);

    } else if (base_type->isDoubleTy()) {

        type_dump                   = std::make_unique<Type>();
        type_dump->base_type_       = TYPE_DOUBLE;
        type_dump->allocation_size_ = data_layout_->getTypeAllocSize(data_type);

    } else if (base_type->isStructTy()) {

        type_dump = ResolveStructType(data_type, base_type);

    } else if (base_type->isFunctionTy()) {

        //TODO: Resolve function type
        type_dump                       = std::make_unique<FunctionType>();
        auto function_dump              =
                                    static_cast<FunctionType*>(type_dump.get());
        function_dump->base_type_       = TYPE_FUNC;
        function_dump->allocation_size_ = 0;

    } else if (base_type->isArrayTy()) {

        //TODO: Resolve array type
        type_dump                   = std::make_unique<Type>();
        type_dump->base_type_       = TYPE_ARRAY;
        type_dump->allocation_size_ = 0;

    } else {
        type_dump                   = std::make_unique<Type>();
        type_dump->base_type_       = TYPE_UNKNOWN;
        type_dump->allocation_size_ = 0;
    }

    // Save a pointer depth as well
    if (pointer_depth > 0) {
        type_dump->pointer_depth_ = pointer_depth;
    }

    return type_dump;
}

std::unique_ptr<Type> Analysis::ResolveIntegerType(llvm::Type* data_type,
                                                   llvm::Type* base_type) {
    std::unique_ptr<Type> type_dump = std::make_unique<Type>();
    type_dump->allocation_size_     = data_layout_->getTypeAllocSize(data_type);

    switch (base_type->getIntegerBitWidth()) {
        case 1:
        case 8:
            type_dump->base_type_ = TYPE_INT8;
            break;
        case 16:
            type_dump->base_type_ = TYPE_INT16;
            break;
        case 32:
            type_dump->base_type_ = TYPE_INT32;
            break;
        case 64:
            type_dump->base_type_ = TYPE_INT64;
            break;
        default:
            type_dump->base_type_ = TYPE_INT_UNKNOWN;
            break;
    }

    return type_dump;
}

std::vector<std::unique_ptr<StructType>>
                 Analysis::DumpModuleStructs(llvm::Module& module) {
    std::vector<llvm::StructType*> module_structs =
                                              module.getIdentifiedStructTypes();
    std::vector<std::unique_ptr<StructType>> dumped_structs;

    // Check if structs are present in the module
    uint64_t structs_number = module_structs.size();
    if (structs_number == 0) {
        // There are no structs in the module, not a problem
        return dumped_structs;
    }

    // Allocate space in vector for module structs
    dumped_structs.reserve(structs_number);

    // Iterate over module structs in reverse order
    for (auto struct_type: module_structs) {

        std::unique_ptr<Type> type_dump =
                                        ResolveStructType(struct_type, nullptr);

        auto struct_type_dump = static_cast<StructType*>(type_dump.release());
        std::unique_ptr<StructType> struct_dump(struct_type_dump);

        // Append the dumped struct to the vector of module structs
        dumped_structs.push_back(std::move(struct_dump));
    }

    return dumped_structs;
}

std::unique_ptr<Type> Analysis::ResolveStructType(llvm::Type* data_type,
                                                  llvm::Type* base_type) {
    // Create an object where to dump found data
    std::unique_ptr<Type> type_dump = std::make_unique<StructType>();
    auto struct_dump                = static_cast<StructType*>(type_dump.get());

    struct_dump->base_type_         = TYPE_STRUCT;

    // ---------------------------------------------------------------------- //
    //                 A single struct field declaration                      //
    // ---------------------------------------------------------------------- //
    if (base_type != nullptr) {

        // A single struct field declaration is a struct inside a struct
        // definition. Do not dig into the declaration recursively as
        // it might become tricky one day. Dump basic data. It will be enough
        // to find the corresponding definition by the declaration name.

        auto struct_type              = llvm::cast<llvm::StructType>(base_type);

        struct_dump->name_            = struct_type->getStructName();

        struct_dump->allocation_size_ =
                                      data_layout_->getTypeAllocSize(data_type);

        struct_dump->is_definition_   = false;

        // + [pointer_depth_] will be set up ahead

        struct_dump->body_            = nullptr;

        return type_dump;
    }

    // ---------------------------------------------------------------------- //
    //                          Struct definition                             //
    // ---------------------------------------------------------------------- //
    auto struct_type                  = llvm::cast<llvm::StructType>(data_type);

    struct_dump->name_                = struct_type->getStructName();

    struct_dump->pointer_depth_       = 0;

    struct_dump->is_definition_       = true;

    // Create a new body for a struct
    std::unique_ptr<Body> struct_body = std::make_unique<Body>();

    // Empty struct definition
    if (struct_type->isOpaque()) {
        // Struct definition is opaque, nothing to discover (it is empty)
        //
        // Explicitly reassign default values, just to make sure
        struct_body->number_of_fields_ = 0;

        // Size of the struct in bytes
        struct_body->size_             = 0;

        struct_body->alignment_        = 0;
        // +
        struct_dump->allocation_size_  = 0;

        struct_dump->body_             = std::move(struct_body);

        return type_dump;
    }

    struct_dump->allocation_size_           =
                                      data_layout_->getTypeAllocSize(data_type);

    const llvm::StructLayout* struct_layout =
                                     data_layout_->getStructLayout(struct_type);

    struct_body->number_of_fields_          = struct_type->getNumElements();

    // Size of a struct in bytes
    struct_body->size_                      = struct_layout->getSizeInBytes();

    // Alignment of the struct fields (8 bytes encountered so far)
    struct_body->alignment_                 =
                   static_cast<uint16_t>(struct_layout->getAlignment().value());

    // ---------------------------------------------------------------------- //
    //                        Struct fields definition                        //
    // ---------------------------------------------------------------------- //
    if (struct_body->number_of_fields_ != 0) {

        // Allocate space for struct fields
        struct_body->fields_.reserve(struct_body->number_of_fields_);

        // Offset of each field in the struct
        uint64_t offset = 0;
        llvm::Type* field_type;

        // Iterate over the struct fields
        for (uint32_t idx = 0; idx < struct_body->number_of_fields_; ++idx) {

            offset     = struct_layout->getElementOffset(idx);
            field_type = struct_type->getElementType(idx);

            // A new field inside the current struct was found, fetch the
            // type of the field.
            std::unique_ptr<Type> field_type_dump =
                                                   ResolveValueType(field_type);

            // Insert into the vector a pair of:
            // 1. the offset of the field in the struct
            // 2. the type of the field
            struct_body->fields_.emplace_back(
                    std::make_pair(offset, std::move(field_type_dump)));
        }
    }

    struct_dump->body_ = std::move(struct_body);

    return type_dump;
}

void Analysis::PrintCFG() const {
    llvm::outs() << "CFG of module functions\n";
    for (const auto& functions_cfg: module_cfg_) {
        const AdjacencyList& func_adjacency_list_ =
                                             functions_cfg->GetAdjacencyList();

        for (const auto& pair: func_adjacency_list_) {
            auto vertex =
                    static_cast<Vertex<llvm::Function>*>(pair.first.get());
            if (vertex->id_ == 0) {
                llvm::outs() << "[" << vertex->object_->getName() << "]\n";
            }
            llvm::outs() << vertex->object_->getName()
                         << "(" << vertex->id_ << ") -> ";

            if (pair.second.empty()) {
                llvm::outs() << "None\n";
                continue;
            }

            for (auto& node: pair.second) {
                auto node_data =
                        static_cast<Vertex<llvm::Function>*>(node.get());
                llvm::outs() << node_data->object_->getName()
                                     << "(" << node_data->id_ << ") ";
            }
            llvm::outs() << "\n";
        }

        llvm::outs() << "===\n";
    }

    llvm::outs() << "Going inside each function..\n";
    for (const auto& bblocks_cfg: function_cfg_) {
        const AdjacencyList& bb_adjacency_list_ =
                                              bblocks_cfg->GetAdjacencyList();
        auto bb_cfg = static_cast<BasicBlockCFG*>(bblocks_cfg.get());

        llvm::outs() << "CFG of a function: ";
        llvm::outs() << bb_cfg->GetFunction().getName() << "\n";
        for (const auto& pair: bb_adjacency_list_) {
            const auto vertex =
                    static_cast<Vertex<llvm::BasicBlock>*>(pair.first.get());
            vertex->object_->printAsOperand(llvm::outs());
            llvm::outs() << "(" << vertex->id_ << ") -> ";

            if (pair.second.empty()) {
                llvm::outs() << "None\n";
                continue;
            }

            for (const auto& node: pair.second) {
                auto node_data =
                        static_cast<Vertex<llvm::BasicBlock>*>(node.get());
                node_data->object_->printAsOperand(llvm::outs());
                llvm::outs() << "(" << node_data->id_ << ") ";
            }
            llvm::outs() << "\n";
        }
        llvm::outs() << "\n";
    }
}
