#include "fuzzer_generator.h"

namespace {
    const std::string HEADERS = R"(#include <cstdint>
)";

    const std::string ADDITIONAL_HEADERS = R"(#include <cstring>
)";

    const std::string MEMORY_FUNCTIONS = R"(
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
    if (length == 0) {
        memset(dst, '\0', number);
    } else if (length >= number) {
        memcpy(dst, src, number);
    } else {
        uint8_t* data = CreateSpace(src, length, number);
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

//
// Fuzzer content structure
//
// [
//  headers,
//  forward declarations (for structs),
//  support-handy functions,
//  target fuzzing function declaration,
//  fuzzer stub body [
//      code before call the target,
//      call arguments
//      code after call the target
//  ]
// ]
//
bool FuzzerGenerator::Generate() {
    if (!IsSupported()) {
        // Some types encountered in the standalone function are not supported
        return false;
    }

    TargetFunctionArgumentQueue Q_args;

    if (!ProcessFunctionArguments(Q_args)) {
        // An unsupported type was encountered, but it actually should be
        return false;
    }

    std::string forward_decl;

    std::string target_func_decl;

    std::string before;
    std::string call_args;
    std::string after;

    bool additional_hdrs = false;
    bool support_funcs   = false;
    uint16_t args_num    = Q_args.size();
    uint16_t arg_pos     = 1;
    while (!Q_args.empty()) {
        TargetFunctionArgumentOptions& arg_options = Q_args.front();

        for (const auto& argument: arg_options) {
            if (!argument->forward_decl.empty()) {
                forward_decl += argument->forward_decl;
                forward_decl += "\n";
            }

            if (!argument->variable_type.empty()) {
                before += argument->variable_type;
                before += " ";
            }

            if (!argument->variable_name.empty()) {
                before += argument->variable_name;
            }

            if (!argument->variable_init.empty()) {
                before += " = ";
                before += argument->variable_init;
                before += ";\n";
            }

            if (!argument->before_call.empty()) {
                for (const auto& line: argument->before_call) {
                    before += "\t";
                    before += line;
                    before += "\n";
                }
            }

            if (!argument->call_arg.empty()) {
                call_args += argument->call_arg;

                if (arg_pos++ < args_num) {
                    call_args += ", ";
                }
            }

            if (!argument->after_call.empty()) {
                for (const auto& line: argument->after_call) {
                    after += "\t";
                    after += line;
                    after += "\n";
                }
            }

            if (argument->additional_hdrs) additional_hdrs = true;

            if (argument->support_funcs) support_funcs = true;

            //
            // For now I do not support a further cycle
            //
            break;
        }

        Q_args.pop();
    }

    // Handle the return type
    BaseType return_type = function_dump_->return_type_->base_type_;
    if (return_type == TYPE_STRUCT) {
        auto struct_type =
                static_cast<StructType*>(function_dump_->return_type_.get());

        forward_decl += "struct ";
        forward_decl += struct_type->name_;
        forward_decl += ";\n";
    }

    //
    // Construct the fuzzer content
    //

    fuzzer_ += HEADERS;
    if (additional_hdrs) fuzzer_ += ADDITIONAL_HEADERS;
    fuzzer_ += "\n";

    fuzzer_ += forward_decl;

    if (support_funcs) fuzzer_ += MEMORY_FUNCTIONS;

    fuzzer_ += function_declaration_;
    fuzzer_ += "\n";

    std::string fuzzer_stub_body(
            ConstructFuzzerStubBody(before, call_args,
                                    after, function_dump_->name_)
                                    );
    std::string fuzzer_stub(InsertFuzzerStubBody(fuzzer_stub_body));
    if (fuzzer_stub.empty()) {
        // Can not insert the body, nothing to insert
        return false;
    }
    fuzzer_ += fuzzer_stub;

    //
    // End of the fuzzer content
    //

    return true;
}

//
// INT8/INT8*     -> [+/+]
// INT16/INT16*   -> [+/+]
// INT32/INT32*   -> [+/+]
// INT64/INT64*   -> [+/+]
// FLOAT/FLOAT*   -> [+/+]
// DOUBLE/DOUBLE* -> [+/+]
// VOID/VOID*     -> [-/+]
// STRUCT/STRUCT* -> [-/+]
// FUNC           -> [-]
// ARRAY          -> [-]
// INT_UNKNOWN    -> [-]
// UNKNOWN        -> [-]
//
bool FuzzerGenerator::IsSupported() {
    bool is_pointer;

    //
    // Check target function arguments
    //
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

    //
    // Check target function return type
    //
    BaseType return_type = function_dump_->return_type_->base_type_;

    if (return_type == TYPE_STRUCT) {
        is_pointer = function_dump_->return_type_->pointer_depth_ > 0;

        if (!is_pointer) {
            // Plain structs are not supported
            return false;
        }
    }

    return true;
}

bool FuzzerGenerator::ProcessFunctionArguments(TargetFunctionArgumentQueue& Q) {
    for (const auto& argument: function_dump_->arguments_) {
        TargetFunctionArgumentOptions trgt_argument_opts;
        BaseType argument_type = argument->type_->base_type_;

        // Cast const to a regular type for compiling compatibility reasons

        switch (argument_type) {
            case TYPE_VOID:
                trgt_argument_opts = ProcessArgumentVoid(argument);
                break;
            case TYPE_INT8:
                trgt_argument_opts = ProcessArgumentInt8(argument);
                break;
            case TYPE_INT16:
                trgt_argument_opts = ProcessArgumentInt16(argument);
                break;
            case TYPE_INT32:
                trgt_argument_opts = ProcessArgumentInt32(argument);
                break;
            case TYPE_INT64:
                trgt_argument_opts = ProcessArgumentInt64(argument);
                break;
            case TYPE_FLOAT:
                trgt_argument_opts = ProcessArgumentFloat(argument);
                break;
            case TYPE_DOUBLE:
                trgt_argument_opts = ProcessArgumentDouble(argument);
                break;
            case TYPE_STRUCT:
                trgt_argument_opts = ProcessArgumentStruct(argument);
                break;
            case TYPE_FUNC:
            case TYPE_ARRAY:
            case TYPE_INT_UNKNOWN:
            case TYPE_UNKNOWN:
            default:
                // Unsupported type was encountered
                return false;
        }

        Q.push(std::move(trgt_argument_opts));
    }

    return true;
}

std::string FuzzerGenerator::ConstructFuzzerStubBody(
        const std::string& before, const std::string& call_args,
        const std::string& after, const std::string& func_name) {
    std::string fuzzer_stub_body;

    fuzzer_stub_body += before;

    fuzzer_stub_body += "\t(void) ";
    fuzzer_stub_body += func_name;
    fuzzer_stub_body += "(";
    fuzzer_stub_body += call_args;
    fuzzer_stub_body += ");\n";

    fuzzer_stub_body += after;

    return fuzzer_stub_body;
}

std::string FuzzerGenerator::InsertFuzzerStubBody(const std::string& body) {
    std::string result;

    if (body.empty()) {
        return result;
    }

    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);

    return std::regex_replace(FUZZER_STUB, pattern, body);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentVoid(
        const std::unique_ptr<Argument>& argument) {
    TargetFunctionArgumentOptions trgt_argument_opts;
    bool is_pointer       = argument->type_->pointer_depth_ > 0;
    uint8_t pointer_depth = argument->type_->pointer_depth_;

    if (is_pointer) {
        // forward_declaration -> [-]
        // variable_type       -> [-]
        // variable_name       -> [-]
        // variable_init       -> [-]
        // before_call         -> [-]
        // call_arg            -> [+]
        // after_call          -> [-]
        // additional_hdrs     -> [-]
        // support_funcs       -> [-]

        auto trgt_argument = std::make_unique<TargetFunctionArgument>();

        trgt_argument->call_arg      = "(void";
        trgt_argument->call_arg      += GetStars(pointer_depth);
        trgt_argument->call_arg      += ") data";

        trgt_argument_opts.push_back(std::move(trgt_argument));
    } else { // Plain data type
        // Should not reach this point, but who knows
    }

    return trgt_argument_opts;
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentInt8(
        const std::unique_ptr<Argument>& argument) {
    TargetFunctionArgumentOptions trgt_argument_opts;

    uint16_t position_ndx = argument->index_;
    bool is_pointer       = argument->type_->pointer_depth_ > 0;
    uint8_t pointer_depth = argument->type_->pointer_depth_;

    if (is_pointer) {
        auto trgt_argument = std::make_unique<TargetFunctionArgument>();

        // A string (char*)
        if (pointer_depth == 1) {
            // forward_declaration -> [-]
            // variable_type       -> [+]
            // variable_name       -> [+]
            // variable_init       -> [+]
            // before_call         -> [+]
            // call_arg            -> [+]
            // after_call          -> [+]
            // additional_hdrs     -> [+]
            // support_funcs       -> [-]

            trgt_argument->additional_hdrs = true;

            trgt_argument->variable_type   = "char*";

            trgt_argument->variable_name   = "string_";
            trgt_argument->variable_name   += std::to_string(position_ndx);

            trgt_argument->variable_init   = "new char[size + 1]";

            std::string line; // used with BEFORE + AFTER

            //
            // Initialize lines BEFORE the target function call
            //
            std::vector<std::string> before_call;

            // Line 1
            line = "memcpy(";
            line += trgt_argument->variable_name;
            line += ", data, size);";
            before_call.push_back(std::move(line));

            // Line 2
            line = trgt_argument->variable_name;
            line += "[size] = 0x0;";
            before_call.push_back(std::move(line));

            trgt_argument->before_call = before_call;
            //
            // End of BEFORE
            //

            trgt_argument->call_arg = trgt_argument->variable_name;

            //
            // Initialize lines AFTER the target function call
            //
            std::vector<std::string> after_call;

            // Line 1
            line = "delete[] ";
            line += trgt_argument->variable_name;
            line += ";";
            after_call.push_back(std::move(line));

            trgt_argument->after_call = after_call;
            //
            // End of AFTER
            //
        } else { // An array of strings (char**)
            // forward_declaration -> [-]
            // variable_type       -> [-]
            // variable_name       -> [-]
            // variable_init       -> [-]
            // before_call         -> [-]
            // call_arg            -> [+]
            // after_call          -> [-]
            // additional_hdrs     -> [-]
            // support_funcs       -> [-]

            trgt_argument->call_arg      = "(char";
            trgt_argument->call_arg      += GetStars(pointer_depth);
            trgt_argument->call_arg      += ") data";
        }

        trgt_argument_opts.push_back(std::move(trgt_argument));
    } else { // Plain numeric data
        trgt_argument_opts = HandleNumeric(argument);
    }

    return trgt_argument_opts;
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentInt16(
        const std::unique_ptr<Argument>& argument) {
    return HandleNumeric(argument);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentInt32(
        const std::unique_ptr<Argument>& argument) {
    return HandleNumeric(argument);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentInt64(
        const std::unique_ptr<Argument>& argument) {
    return HandleNumeric(argument);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentFloat(
        const std::unique_ptr<Argument>& argument) {
    return HandleNumeric(argument);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentDouble(
        const std::unique_ptr<Argument>& argument) {
    return HandleNumeric(argument);
}

TargetFunctionArgumentOptions
FuzzerGenerator::ProcessArgumentStruct(
        const std::unique_ptr<Argument>& argument) {
    bool is_pointer = argument->type_->pointer_depth_ > 0;

    if (!is_pointer) {
        // Plain data type
        // Should not reach this point, but who knows
        return {};
    }

    // forward_declaration -> [+]
    // variable_type       -> [-]
    // variable_name       -> [-]
    // variable_init       -> [-]
    // before_call         -> [-]
    // call_arg            -> [+]
    // after_call          -> [-]
    // additional_hdrs     -> [-]
    // support_funcs       -> [-]

    TargetFunctionArgumentOptions trgt_argument_opts;
    auto trgt_argument    = std::make_unique<TargetFunctionArgument>();
    uint8_t pointer_depth = argument->type_->pointer_depth_;
    auto struct_type      = static_cast<StructType*>(argument->type_.get());

    trgt_argument->forward_decl  = "struct ";
    trgt_argument->forward_decl  += struct_type->name_;
    trgt_argument->forward_decl  += ";";

    trgt_argument->call_arg      = "(struct ";
    trgt_argument->call_arg      += struct_type->name_;
    trgt_argument->call_arg      += GetStars(pointer_depth);
    trgt_argument->call_arg      += ") data";

    trgt_argument_opts.push_back(std::move(trgt_argument));

    return trgt_argument_opts;
}

TargetFunctionArgumentOptions
FuzzerGenerator::HandleNumeric(const std::unique_ptr<Argument>& argument) {
    TargetFunctionArgumentOptions trgt_argument_opts;

    bool is_pointer        = argument->type_->pointer_depth_ > 0;
    BaseType argument_type = argument->type_->base_type_;

    if (!is_pointer) {
        //
        // Plain numeric data
        //
        auto trgt_argument_opt1  = std::make_unique<TargetFunctionArgument>();
        auto trgt_argument_opt2  = std::make_unique<TargetFunctionArgument>();
        uint16_t position_ndx    = argument->index_;
        uint64_t allocation_size = argument->type_->allocation_size_;

        //
        // First option of a variable (close-to-real)
        //
        // forward_declaration -> [-]
        // variable_type       -> [-]
        // variable_name       -> [-]
        // variable_init       -> [-]
        // before_call         -> [-]
        // call_arg            -> [+]
        // after_call          -> [-]
        // additional_hdrs     -> [-]
        // support_funcs       -> [-]

        //
        // Second option of a variable (random-max)
        //
        // forward_declaration -> [-]
        // variable_type       -> [+]
        // variable_name       -> [+]
        // variable_init       -> [-]
        // before_call         -> [+]
        // call_arg            -> [+]
        // after_call          -> [-]
        // additional_hdrs     -> [+]
        // support_funcs       -> [+]

        trgt_argument_opt2->additional_hdrs = true;

        trgt_argument_opt2->support_funcs   = true;

        trgt_argument_opt2->variable_name   = "var_";
        trgt_argument_opt2->variable_name   += std::to_string(position_ndx);

        switch (argument_type) {
            case TYPE_INT8: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(int8_t) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "int8_t";

                break;
            }
            case TYPE_INT16: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(int16_t) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "int16_t";

                break;
            }
            case TYPE_INT32: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(int32_t) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "int32_t";

                break;
            }
            case TYPE_INT64: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(int64_t) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "int64_t";

                break;
            }
            case TYPE_FLOAT: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(float) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "float";

                break;
            }
            case TYPE_DOUBLE: {
                // First option of a variable (close-to-real)
                trgt_argument_opt1->call_arg = "(double) size";

                // Second option of a variable (random-max)
                trgt_argument_opt2->variable_type = "double";

                break;
            }
            default:
                // Should not reach this point, but who knows
                return {};
        }

        //
        // Initialize lines BEFORE the target function call
        //
        std::vector<std::string> before_call;
        std::string line;
        // Line 1
        line = trgt_argument_opt2->variable_type;
        line += " ";
        line += trgt_argument_opt2->variable_name;
        line += ";";
        before_call.push_back(std::move(line));

        // Line 2
        line = "FillMemory(&";
        line += trgt_argument_opt2->variable_name;
        line += ", data, size, ";
        line += std::to_string(allocation_size);
        line += ");";
        before_call.push_back(std::move(line));
        //
        // End of BEFORE
        //

        trgt_argument_opt2->call_arg = trgt_argument_opt2->variable_name;

        // Append both options to the vector and we are good to go
        trgt_argument_opts.push_back(std::move(trgt_argument_opt1));
        trgt_argument_opts.push_back(std::move(trgt_argument_opt2));
    } else {
        //
        // Pointer data
        //
        // forward_declaration -> [-]
        // variable_type       -> [-]
        // variable_name       -> [-]
        // variable_init       -> [-]
        // before_call         -> [-]
        // call_arg            -> [+]
        // after_call          -> [-]
        // additional_hdrs     -> [-]
        // support_funcs       -> [-]

        auto trgt_argument    = std::make_unique<TargetFunctionArgument>();
        uint8_t pointer_depth = argument->type_->pointer_depth_;

        switch (argument_type) {
            case TYPE_INT16: {
                trgt_argument->call_arg = "(int16";
                break;
            }
            case TYPE_INT32: {
                trgt_argument->call_arg = "(int32";
                break;
            }
            case TYPE_INT64: {
                trgt_argument->call_arg = "(int64";
                break;
            }
            case TYPE_FLOAT: {
                trgt_argument->call_arg = "(float";
                break;
            }
            case TYPE_DOUBLE: {
                trgt_argument->call_arg = "(double";
                break;
            }
            default:
                // Should not reach this point, but who knows
                return {};
        }

        trgt_argument->call_arg += GetStars(pointer_depth);
        trgt_argument->call_arg += ") data";

        trgt_argument_opts.push_back(std::move(trgt_argument));
    }

    return trgt_argument_opts;
}

inline std::string FuzzerGenerator::GetStars(uint8_t pointer_depth) {
    std::string stars;
    for (uint8_t idx = 0; idx < pointer_depth; ++idx, stars += "*") {}

    return stars;
}
