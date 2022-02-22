#include "analysis.h"

char Analysis::ID = 0;

Analysis::Analysis(std::string ir_module)
: llvm::ModulePass(ID), success_(false), module_(nullptr),
module_dump_(nullptr) {
    LaunchPassOnIRModule(std::move(ir_module));
}

Analysis::operator bool() const {
    return success_;
}

std::unique_ptr<Module> Analysis::GetModuleDump() {
    if (!module_dump_) {
        return nullptr;
    }

    return std::move(module_dump_);
}

// ------------------------------------------------------------------------- //
//                              Load IR file                                 //
// ------------------------------------------------------------------------- //
void Analysis::LaunchPassOnIRModule(std::string&& ir_module) {
    llvm::SMDiagnostic error;
    llvm::LLVMContext context;

    // Load IR text representation into memory
    std::unique_ptr<llvm::Module> module(
            llvm::parseIRFile(ir_module, error, context));
    if (!module) {
        fprintf(stderr, "Could not open [%s]\n", ir_module.c_str());
        success_ = false;
        return;
    }

    // Tell the pass that analysis operations (CallGraph) will be done further
    llvm::PassRegistry* passReg = llvm::PassRegistry::getPassRegistry();
    llvm::initializeAnalysis(*passReg);

    // Register the pass and run it
    llvm::legacy::PassManager pass_manager;
    pass_manager.add(this);
    pass_manager.run(*module);

    success_ = true;
}

void Analysis::getAnalysisUsage(llvm::AnalysisUsage& analysis_usage) const {}

llvm::StringRef Analysis::getPassName() const {
    return "ModuleAnalysis";
}

// ------------------------------------------------------------------------- //
//                              Run Pass manager                             //
// ------------------------------------------------------------------------- //
bool Analysis::runOnModule(llvm::Module& module) {
    // --------------------------------------------------------------------- //
    //                 Initialize class attributes (important)               //
    // --------------------------------------------------------------------- //
    // Initialize global module for further application
    module_ = &module;
    data_layout_ = std::make_unique<llvm::DataLayout>(&module);

    // --------------------------------------------------------------------- //
    //                        Dump module (important)                        //
    // --------------------------------------------------------------------- //
    // Start dumping the module
    module_dump_ = std::make_unique<Module>();
    // Copy module name
    module_dump_->name_ = module.getModuleIdentifier();
    // Copy module corresponding source file
    module_dump_->source_name_ = module.getSourceFileName();
    // Get number of functions
    module_dump_->functions_number_ = module.getFunctionList().size();

    // Check if there are functions in the module
    if (module_dump_->functions_number_ == 0) {
        // There are no functions in the module, quit
        success_ = false;

        return false;
    }

    // --------------------------------------------------------------------- //
    //                            Construct CFGs                             //
    // --------------------------------------------------------------------- //
    root_function_ = GetRootFunction();
    if (!root_function_) {
        success_ = false;

        return false;
    }

    MakeControlFlowGraph(*root_function_);
    FindStandaloneFunctions();

    // --------------------------------------------------------------------- //
    //                          Dump global structs                          //
    // --------------------------------------------------------------------- //
    //DumpModuleStructs();

    // --------------------------------------------------------------------- //
    //                         Dump module functions                         //
    // --------------------------------------------------------------------- //

    /*
    // Allocate space in vector for all functions
    module_dump_->functions_.reserve(module_dump_->functions_number_);
    // Iterate over module's functions
    for (llvm::Module::const_iterator ii = module.begin();
         ii != module.end(); ++ii) {
        const llvm::Function& function = *ii;

        std::unique_ptr<Function> function_dump = DumpModuleFunction(function);

        // Append the dumped function to the vector of module functions
        module_dump_->functions_.push_back(std::move(function_dump));
    }
    */

    // The module has not been modified, then return false
    return false;
}

void Analysis::DumpModuleStructs() {
    // Find all module structs
    std::vector<llvm::StructType*> module_structs =
            module_->getIdentifiedStructTypes();
    std::ranges::reverse(module_structs);

    // Allocate space in vector for all structs
    uint64_t structs_number = module_structs.size();
    module_dump_->structs_.reserve(structs_number);

    // Iterate over module structs in reverse order
    for (auto struct_type : module_structs) {

        // Dump a struct
        std::unique_ptr<Type> type_dump =
                ResolveValueType(struct_type);

        // Cast Type to StructType as we are sure, that there is a StructType
        // object beneath.
        auto _struct_ = static_cast<StructType*>(type_dump.release());
        std::shared_ptr<StructType> struct_dump(_struct_);

        // Append the dumped struct to the vector of module structs
        module_dump_->structs_.push_back(std::move(struct_dump));

    }
}

std::unique_ptr<Function>
        Analysis::DumpModuleFunction(const llvm::Function& function) {
    llvm::Type* return_type;

    std::unique_ptr<Function> function_dump = std::make_unique<Function>();

    // Fetch general function data
    function_dump->name_ = function.getName().str();
    function_dump->arguments_number_ =
            static_cast<uint16_t>(function.arg_size());
    function_dump->arguments_fixed_ = function.isVarArg();
    return_type = function.getReturnType();
    function_dump->return_type_ = ResolveValueType(return_type);
    function_dump->is_local_ = !function.isDeclaration();

    // --------------------------------------------------------------------- //
    //                       Dump function arguments                         //
    // --------------------------------------------------------------------- //
    // Allocate space in vector for function arguments
    function_dump->arguments_.reserve(function_dump->arguments_number_);
    // Iterate over function's arguments
    for (llvm::Function::const_arg_iterator ii = function.arg_begin();
                                            ii != function.arg_end(); ++ii) {
        const llvm::Argument& argument = *ii;
        std::unique_ptr<Argument> argument_dump = DumpFunctionArgument(argument);

        // Append the dumped argument to the vector of function arguments
        function_dump->arguments_.push_back(std::move(argument_dump));
    }

    // Iterate over function's instructions
    for (const llvm::Instruction& instruction :
                                          llvm::instructions(&function)) {
        if (auto ret_instruction =
                llvm::dyn_cast<llvm::ReturnInst>(&instruction)) {

        }
        break;
    }

    return function_dump;
}

std::unique_ptr<Argument>
        Analysis::DumpFunctionArgument(const llvm::Argument& argument) {
    std::unique_ptr<Argument> argument_dump = std::make_unique<Argument>();
    llvm::Type* argument_type;

    argument_dump->index_ = static_cast<int16_t>(argument.getArgNo());
    argument_type = argument.getType();
    argument_dump->type_ = ResolveValueType(argument_type);

    return argument_dump;
}

std::unique_ptr<Type> Analysis::ResolveValueType(llvm::Type* data_type) {
    llvm::Type* base_type = data_type;
    std::unique_ptr<Type> type_dump;
    uint8_t pointer_depth = 0;

    // If there are pointers, then bring up a base type
    while (base_type->isPointerTy()) {
        ++pointer_depth;
        base_type = llvm::dyn_cast<llvm::PointerType>
                (base_type)->getElementType();
    }

    // Find base type
    if (base_type->isVoidTy()) {

        type_dump = std::make_unique<Type>();
        type_dump->base_type_ = TYPE_VOID;
        type_dump->allocation_size_ =
                data_layout_->getTypeAllocSize(data_type);

    } else if (base_type->isIntegerTy()) {

        type_dump = ResolveIntegerType(data_type, base_type);

    } else if (base_type->isFloatTy()) {

        type_dump = std::make_unique<Type>();
        type_dump->base_type_ = TYPE_FLOAT;
        type_dump->allocation_size_ =
                data_layout_->getTypeAllocSize(data_type);

    } else if (base_type->isDoubleTy()) {

        type_dump = std::make_unique<Type>();
        type_dump->base_type_ = TYPE_DOUBLE;
        type_dump->allocation_size_ =
                data_layout_->getTypeAllocSize(data_type);

    } else if (base_type->isStructTy()) {

        type_dump = ResolveStructType(data_type, base_type);

    } else if (base_type->isFunctionTy()) {

        //TODO: Resolve function type
        type_dump = std::make_unique<FunctionType>();
        auto function_dump = static_cast<FunctionType*>(type_dump.get());
        function_dump->base_type_ = TYPE_FUNC;

    } else if (base_type->isArrayTy()) {

        //TODO: Resolve array type
        type_dump = std::make_unique<Type>();
        type_dump->base_type_ = TYPE_ARRAY;

    } else {
        type_dump = std::make_unique<Type>();
        type_dump->base_type_ = TYPE_UNKNOWN;
    }

    // Append found pointer to a Type struct
    if (pointer_depth > 0) {
        type_dump->pointer_depth_ = pointer_depth;
    }

    return type_dump;
}

std::unique_ptr<Type> Analysis::ResolveIntegerType(llvm::Type* data_type,
                                                   llvm::Type* base_type) {
    std::unique_ptr<Type> type_dump = std::make_unique<Type>();
    type_dump->allocation_size_ = data_layout_->getTypeAllocSize(data_type);
    switch (base_type->getIntegerBitWidth()) {
        case 1:
            type_dump->base_type_ = TYPE_BOOL;
            break;
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

//
// How this method works:
// In the llvm IR representation the below struct will be:
// struct A {
//    struct B {
//        long*  j;
//        char** k;
//        bool   l;
//        struct C {
//            char n;
//        } m;
//    } a;
//    int*               c;
//    unsigned short int d;
//    float              e;
//    double             f;
//    long               h;
//    int                i;
//};
// %struct.A = type { %struct.B, i32*, i16, float, double, i64, i32 }
// %struct.B = type { i64*, i8**, i8, %struct.C }
// %struct.C = type { i8 }
//
// The method iterates over the IR structs in reverse order and stores it
// in memory in this order:
// %struct.C = type { i8 }
// %struct.B = type { i64*, i8**, i8, [address_to_struct.C] }
// %struct.A = type { [address_to_struct.B], i32*, i16, float, double, i64, i32 }
//
std::unique_ptr<Type> Analysis::ResolveStructType(llvm::Type* data_type,
                                                  llvm::Type* base_type) {
    // Create an object where to dump found data
    std::unique_ptr<Type> type_dump = std::make_unique<StructType>();
    auto struct_dump = static_cast<StructType*>(type_dump.get());
    // Get actual data about the found struct
    auto struct_type =
            llvm::dyn_cast<llvm::StructType>(base_type);

    // Identify struct name
    struct_dump->name_ = struct_type->getStructName();

    // --------------------------------------------------------------------- //
    //                       Resolve a struct pointer                        //
    // --------------------------------------------------------------------- //
    // If the data type is a pointer
    if (data_type != base_type) {
        // Fetch general data about the struct
        struct_dump->is_original_ = false;
        struct_dump->base_type_ = TYPE_STRUCT;
        struct_dump->allocation_size_ =
                data_layout_->getTypeAllocSize(data_type);

        return type_dump;
    }

    // --------------------------------------------------------------------- //
    //                        Resolve a hollow struct                        //
    // --------------------------------------------------------------------- //
    // Determine whether the struct has been already registered
    if (module_dump_->struct_list_.contains(struct_dump->name_)) {
        // The struct already exists
        // The current struct will be a link to the original one (with the
        // body/definition)
        struct_dump->is_original_ = false;
        for (auto& registered_struct: module_dump_->structs_) {
            // Find previously registered struct
            if (registered_struct->name_ == struct_dump->name_) {
                // The registered struct is found, check if it is original, i.e.
                // has a body (definition)
                if (registered_struct->is_original_) {
                    // The registered struct has a body, store the address of
                    // the found struct address to the current analyzing struct
                    struct_dump->base_type_ = TYPE_STRUCT;
                    struct_dump->allocation_size_ =
                            data_layout_->getTypeAllocSize(data_type);
                    struct_dump->content_ = registered_struct;
                }
            }
        }

        return type_dump;

    } else {  // This struct does not exist, register a new entry
        module_dump_->struct_list_.insert(struct_dump->name_);
    }

    // --------------------------------------------------------------------- //
    //                        Resolve a solid struct                         //
    // --------------------------------------------------------------------- //
    // Fetch general data about the struct
    struct_dump->base_type_ = TYPE_STRUCT;

    // Create the body/definition of the struct
    std::unique_ptr<Body> struct_body = std::make_unique<Body>();
    struct_dump->is_original_ = true;

    if (!struct_type->isOpaque()) { // Struct has the body, dig into

        struct_dump->allocation_size_ =
                data_layout_->getTypeAllocSize(data_type);

        // Auxiliary data to the main struct info, it provides:
        // 1. Size of the struct in bytes
        // 2. Alignment of the struct fields (8 bytes encountered so far)
        // 3. Offset of each field in the struct
        const llvm::StructLayout* struct_layout =
                data_layout_->getStructLayout(struct_type);
        struct_body->number_of_fields_ = struct_type->getNumElements();
        struct_body->size_ = struct_layout->getSizeInBytes();
        struct_body->alignment_ =
                static_cast<uint16_t>(struct_layout->getAlignment().value());

        // Allocate space for struct fields
        struct_body->fields_.reserve(struct_body->number_of_fields_);
        // Iterate over the struct fields
        uint64_t offset = 0;
        llvm::Type* field_type;
        for (uint32_t idx = 0; idx < struct_body->number_of_fields_; ++idx) {
            offset = struct_layout->getElementOffset(idx);
            field_type = struct_type->getElementType(idx);

            // A new field inside the current struct was found, fetch the
            // type of the field.
            std::unique_ptr<Type> field_type_dump =
                    ResolveValueType(field_type);

            // Insert into the vector a pair of:
            // 1. the offset of the field in the struct
            // 2. the type of the field
            struct_body->fields_.emplace_back(
                    std::make_pair(
                            offset,
                            std::move(field_type_dump)
                    ));
        }
    } else { // Struct definition is opaque, nothing to discover
        // Explicitly reassign default value, just to make sure
        struct_body->number_of_fields_ = 0;
    }

    struct_dump->content_ = std::move(struct_body);

    return type_dump;
}

//
// Root function is a function that calls every other function in the module
//
const llvm::Function* Analysis::GetRootFunction() const {
    llvm::CallGraph call_graph(*module_);

    // Enumerate all functions in the module
    for (auto& ii: call_graph) {
        llvm::CallGraphNode* call_graph_node = ii.second.get();
        // Find out the number of times the function was encountered in the
        // module
        if (call_graph_node->getNumReferences() == 1) {
            const llvm::Function* function = ii.first;
            // Find out if the function has a definition
            if (!function->isDeclaration()) {
                return function;
            }
        }
    }

    return nullptr;
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
void Analysis::MakeControlFlowGraph(const llvm::Function& function) {
    // Check if the function has a body
    if (function.empty()) {
        // It does not
        return;
    }

    const llvm::BasicBlock* entry_block   = &function.getEntryBlock();
    std::queue<const llvm::BasicBlock*>     bb_queue;
    std::map<const llvm::BasicBlock*, bool> visited;
    uint32_t                                block_id = 0;

    // Add new CFG object for further analysis
    module_cfg_.emplace_back(std::make_unique<CFG>(function));
    CFG* cfg = module_cfg_.back().get();

    // Create/initialize all vertices for the adjacency list
    for (auto& basic_block: function.getBasicBlockList()) {
        cfg->AddVertex(block_id, &basic_block);
        ++block_id;
    }

    // Add the entry block to the queue
    bb_queue.push(entry_block);
    visited[entry_block] = true;

    // Start processing vertices until there is no left
    while (!bb_queue.empty()) {
        const llvm::BasicBlock* block = bb_queue.front();
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
                MakeControlFlowGraph(*callee);
            }
        }

        // Find all the neighbors (successors) for the basic block
        const llvm::Instruction* terminator = block->getTerminator();
        for (uint32_t idx = 0; idx < terminator->getNumSuccessors(); ++idx) {
            const llvm::BasicBlock* successor = terminator->getSuccessor(idx);

            // A neighboring vertex is found, it can form an edge
            cfg->AddEdge(block, successor);

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
}

void Analysis::FindStandaloneFunctions() {
    std::set<const llvm::Function*> dependant_functions;

    // Find all dependant functions
    // A dependant function is a function that calls other in-module functions
    // inside itself
    for (std::unique_ptr<CFG>& cfg: module_cfg_) {
        const llvm::Function* function = &cfg->GetFunction();

        // Find all instructions in the module that call the function
        for (const llvm::User* user: function->users()) {
            if (const llvm::Instruction* instruction =
                    llvm::dyn_cast<llvm::Instruction>(user)) {
                // Get the function which the instruction belongs to
                const llvm::Function* parent = instruction->getFunction();

                if (dependant_functions.contains(parent)) {
                    continue;
                }

                dependant_functions.insert(parent);
            }
        }
    }

    // Filter standalone functions
    for (std::unique_ptr<CFG>& cfg: module_cfg_) {
        const llvm::Function* function = &cfg->GetFunction();
        bool result = dependant_functions.contains(function);

        if (!result) {
            standalone_functions_.insert(function);
        }
    }

}
