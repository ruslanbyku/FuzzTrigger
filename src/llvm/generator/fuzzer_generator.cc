#include "fuzzer_generator.h"

namespace {
    const std::string HEADERS = R"(#include <cstdio>
#include <cstdint>
)";

    const std::string ADDITIONAL_HEADERS = R"(#include <cstring>
)";

    const std::string FILL_MEMORY_FUNCTIONS = R"(
uint8_t* CreateSpace(const uint8_t* src, size_t length, size_t number) {
    size_t index  = 0;
    uint8_t* data = new uint8_t[number];

    while (number / length > 0) {
        memcpy(data + index, src, length);
        index  += length;
        number -= length;
    }

    if (number % length > 0) {
        memcpy(data + index, src, number);
    }

    return data;
}

void FillMemory(void* dst, const uint8_t* src, size_t length, size_t number) {
    if (length >= number) {
        memcpy(dst, src, number);
    } else {
        uint8_t* data = create_space(src, length, number);
        memcpy(dst, data, number);
        delete[] data;
    }
}
)";

    const std::string FUZZER_STUB = R"(
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    $[fuzzer_body]$

    return 0;
}
)";
}

FuzzerGenerator::FuzzerGenerator(std::string function_declaration,
                                 const std::shared_ptr<Function>& function_dump)
: function_declaration_(std::move(function_declaration)),
function_dump_(function_dump) {}

const std::string &FuzzerGenerator::GetFuzzer() const {
    return fuzzer_;
}

bool FuzzerGenerator::Generate() {
    if (!IsSupported()) {
        // Some types encountered in the standalone function are not supported
        return false;
    }

    std::string body = GenerateFuzzerBody();
    if (body.empty()) {
        // An error occurred during generation
        return false;
    }

    std::string fuzzer_stub(InsertFuzzerBody(body));
    if (fuzzer_stub.empty()) {
        // Can not insert the body, nothing to insert
        return false;
    }

    //
    // Construct a fuzzer stub content
    //

    // Construct all necessary headers
    fuzzer_ += HEADERS;

    if (HasPlainTypeInArguments()) {
        // There are some plain types that need to be cared of
        fuzzer_ += ADDITIONAL_HEADERS;
        fuzzer_ += FILL_MEMORY_FUNCTIONS;
    }

    fuzzer_ += "\n";

    // Construct struct forward declarations
    if (NeedStructForwardDeclaration()) {
        // For every struct in the calling function, make a forward declaration
        fuzzer_ += MakeStructForwardDeclaration();
        fuzzer_ += "\n";
    }

    // Construct a standalone function declaration
    fuzzer_ += function_declaration_;
    fuzzer_ += "\n";

    // Construct the actual fuzzer code
    fuzzer_ += fuzzer_stub;

    return true;
}

bool FuzzerGenerator::IsSupported() {
    bool is_pointer;

    for (const auto& argument: function_dump_->arguments_) {
        BaseType argument_type = argument->type_->base_type_;
        is_pointer             = argument->type_->pointer_depth_ > 0;

        switch (argument_type) {
            case TYPE_INT8:
            case TYPE_INT16:
            case TYPE_INT32:
            case TYPE_INT64:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
                // Everything is supported by this point
                continue;
            case TYPE_VOID:
            case TYPE_STRUCT:
                if (is_pointer) {
                    // Only pointers are supported
                    continue;
                }
                // Plain void and struct are not supported
                return false;
            case TYPE_FUNC:
            case TYPE_ARRAY:
            case TYPE_INT_UNKNOWN:
            case TYPE_UNKNOWN:
            default:
                // Not supported
                return false;
        }
    }

    BaseType return_type = function_dump_->return_type_->base_type_;
    if (return_type == TYPE_STRUCT) {
        is_pointer       = function_dump_->return_type_->pointer_depth_ > 0;

        if (!is_pointer) {
            // Plain structs are not supported
            return false;
        }
    }

    return true;
}

std::string FuzzerGenerator::GenerateFuzzerBody() {
    std::string initialization;
    std::string call_arguments;

    for (const auto& argument: function_dump_->arguments_) {
        BaseType argument_type = argument->type_->base_type_;
        bool is_pointer        = argument->type_->pointer_depth_ > 0;
        uint8_t pointer_depth  = argument->type_->pointer_depth_;

        if (is_pointer) {
            // Cast from const to a regular type for compiling compatibility
            // reasons
            switch (argument_type) {
                case TYPE_VOID:
                    call_arguments += "(void";
                    break;
                case TYPE_INT8:
                    call_arguments += "(char";
                    break;
                case TYPE_INT16:
                    call_arguments += "(int16_t";
                    break;
                case TYPE_INT32:
                    call_arguments += "(int32_t";
                    break;
                case TYPE_INT64:
                    call_arguments += "(int64_t";
                    break;
                case TYPE_FLOAT:
                    call_arguments += "(float";
                    break;
                case TYPE_DOUBLE:
                    call_arguments += "(double";
                    break;
                case TYPE_STRUCT: {
                    auto struct_type =
                            static_cast<StructType*>(argument->type_.get());

                    call_arguments +=  "(struct ";
                    call_arguments += struct_type->name_;
                    break;
                }
                case TYPE_FUNC:
                case TYPE_ARRAY:
                case TYPE_INT_UNKNOWN:
                case TYPE_UNKNOWN:
                default:
                    // Unsupported type was encountered
                    return {};
            }

            call_arguments += GetStars(pointer_depth);
            call_arguments += ") data";

        } else {   // Not a pointer, plain data
            auto result =
                    CreateNumericVariable(argument_type, argument->index_);

            if (result.first.empty() || result.second.empty()) {
                // Unsupported type was encountered
                return {};
            }

            initialization += result.first + ";\n";
            initialization += "FillMemory(&";
            initialization += result.second + ", ";
            initialization += "data, size, ";
            initialization += std::to_string(argument->type_->allocation_size_);
            initialization += ");\n";
            //
            call_arguments += result.first;
        }

        call_arguments += ", ";
    }

    if (!call_arguments.empty()) {
        // If the arguments are not empty, at least one iteration was made -
        // at least one argument was created
        //
        // Delete the last space and comma
        call_arguments.erase(call_arguments.end() - 2, call_arguments.end());
    }

    //
    // Construct the actual body of the fuzzer
    //
    std::string body(initialization);

    body += "(void) ";
    body += function_dump_->name_;
    body += "(";
    body += call_arguments;
    body += ");";

    return body;
}

std::pair<std::string, std::string>
        FuzzerGenerator::CreateNumericVariable(BaseType type, uint16_t number) {
    std::string initialization;
    std::string variable;

    variable = "var_" + std::to_string(number);

    switch (type) {
        case TYPE_INT8: {
            initialization = "int8_t ";
            break;
        }
        case TYPE_INT16: {
            initialization = "int16_t ";
            break;
        }
        case TYPE_INT32: {
            initialization = "int32_t ";
            break;
        }
        case TYPE_INT64: {
            initialization = "int64_t ";
            break;
        }
        case TYPE_FLOAT: {
            initialization = "float ";
            break;
        }
        case TYPE_DOUBLE: {
            initialization = "double ";
            break;
        }
        default:
            return std::make_pair("", "");
    }

    initialization += variable;
    initialization += ";";

    return std::make_pair(initialization, variable);
}

inline std::string FuzzerGenerator::GetStars(uint8_t pointer_depth) {
    std::string stars;
    for (uint8_t idx = 0; idx < pointer_depth; ++idx, stars += "*") {}

    return stars;
}

std::string FuzzerGenerator::InsertFuzzerBody(const std::string& body) {
    std::string result;

    if (body.empty()) {
        return result;
    }

    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);

    return std::regex_replace(FUZZER_STUB, pattern, body);
}

bool FuzzerGenerator::HasPlainTypeInArguments() {
    for (const auto& argument: function_dump_->arguments_) {
        bool is_pointer = argument->type_->pointer_depth_ > 0;

        if (!is_pointer) {
            return true;
        }
    }

    return false;
}

bool FuzzerGenerator::NeedStructForwardDeclaration() {
    for (const auto& argument: function_dump_->arguments_) {
        if (argument->type_->base_type_ == TYPE_STRUCT) {
            return true;
        }
    }

    return function_dump_->return_type_->base_type_ == TYPE_STRUCT;
}

std::string FuzzerGenerator::MakeStructForwardDeclaration() {
    StructType* struct_type;
    std::string forward_declaration;

    for (const auto& argument: function_dump_->arguments_) {
        BaseType argument_type = argument->type_->base_type_;

        if (argument_type == TYPE_STRUCT) {
            struct_type = static_cast<StructType*>(argument->type_.get());

            forward_declaration += "struct ";
            forward_declaration += struct_type->name_;
            forward_declaration += ";\n";
        }
    }

    BaseType return_type = function_dump_->return_type_->base_type_;
    if (return_type == TYPE_STRUCT) {
        struct_type =
                static_cast<StructType*>(function_dump_->return_type_.get());

        forward_declaration += "struct ";
        forward_declaration += struct_type->name_;
        forward_declaration += ";\n";
    }

    return forward_declaration;
}
