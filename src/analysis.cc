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

    // Tell the pass that analysis operations will be done further
    llvm::PassRegistry* passReg = llvm::PassRegistry::getPassRegistry();
    // llvm::CallGraphWrapperPass needs it
    llvm::initializeAnalysis(*passReg);

    // Register the pass and run it
    llvm::legacy::PassManager pass_manager;
    pass_manager.add(this);
    pass_manager.run(*module);

    success_ = true;
}

void Analysis::getAnalysisUsage(llvm::AnalysisUsage& analysis_usage) const {
    // Must be executed first
    // AU.addRequired<llvm::CallGraphWrapperPass>();
}

llvm::StringRef Analysis::getPassName() const {
    return "ModuleAnalysis";
}

bool Analysis::runOnModule(llvm::Module& module) {
    // --------------------------------------------------------------------- //
    //                      Initialize class attributes                      //
    // --------------------------------------------------------------------- //
    // Initialize global module for further application
    module_ = &module;
    data_layout_ = std::make_unique<llvm::DataLayout>(&module);

    // --------------------------------------------------------------------- //
    //                             Dump module                               //
    // --------------------------------------------------------------------- //
    // Start dumping the module
    module_dump_ = std::make_unique<Module>();
    // Copy module name
    module_dump_->name_ = module.getModuleIdentifier();
    // Copy module corresponding source file
    module_dump_->source_name_ = module.getSourceFileName();

    // --------------------------------------------------------------------- //
    //                          Dump global structs                          //
    // --------------------------------------------------------------------- //
    DumpModuleStructs();

    // --------------------------------------------------------------------- //
    //                         Dump module functions                         //
    // --------------------------------------------------------------------- //
    // Get number of functions
    module_dump_->functions_number_ = module.getFunctionList().size();

    // Check if there are functions in the module
    if (module_dump_->functions_number_ == 0) {
        // There are no functions in the module, quit
        success_ = false;

        return false;
    }

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
        auto _struct_ = reinterpret_cast<StructType*>(type_dump.release());
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
    function_dump->name_ = function.getName().data();
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
        auto function_dump = reinterpret_cast<FunctionType*>(type_dump.get());
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
    auto struct_dump = reinterpret_cast<StructType*>(type_dump.get());
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
