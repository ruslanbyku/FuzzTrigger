#include "fuzzer_generator.h"

std::string headers = R"(#include <cstdio>
#include <cstdint>
)";

std::string fuzzer_stub = R"(
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    $[fuzzer_body]$
    return 0;
}
)";

FuzzerGenerator::FuzzerGenerator(std::string function_declaration,
                                 const std::unique_ptr<Function>& function_dump)
: function_declaration_(std::move(function_declaration)),
function_dump_(function_dump) {}

const std::string &FuzzerGenerator::GetFuzzer() const {
    return fuzzer_;
}

bool FuzzerGenerator::Generate() {
    std::string body;
    bool result = GenerateFuzzerBody(body);
    if (!result) {
        return false;
    }
    InsertFuzzerBody(body);

    fuzzer_ += headers;
    fuzzer_ += function_declaration_;
    fuzzer_ += fuzzer_stub;

    return true;
}

bool FuzzerGenerator::GenerateFuzzerBody(std::string& body) {
    body += "(void) ";
    body += function_dump_->name_;
    body += "(";

    std::string arguments;
    bool result = GenerateArguments(arguments);
    if (!result) {
        // Some arguments are unsupported
        return false;
    }
    body += arguments;

    body += ");";

    return true;
}

bool FuzzerGenerator::GenerateArguments(std::string& arguments) {
    for (const auto& argument: function_dump_->arguments_) {
        bool is_pointer        = argument->type_->pointer_depth_ > 0;
        uint8_t pointer_depth  = argument->type_->pointer_depth_;
        BaseType argument_type = argument->type_->base_type_;

        // Do not support more than one asterisk (**)
        if (pointer_depth > 1) {
            return false;
        }

        // Get rid of the types that are not supported
        bool is_supported = true;
        switch (argument_type) {
            default:
                break;
            case TYPE_INT16:
            case TYPE_INT32:
            case TYPE_INT64:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
            case TYPE_BOOL:   // ?
            case TYPE_STRUCT:
            case TYPE_FUNC:
            case TYPE_ARRAY:
            case TYPE_INT_UNKNOWN:
            case TYPE_UNKNOWN:
                is_supported = false;
                break;
        }
        // Do no know how to work with a function that has unsupported
        // argument types
        if (!is_supported) {
            return false;
        }

        // TYPE_VOID and TYPE_INT8

        // Check if the type is a pointer
        if (!is_pointer) {
            // Not a pointer
            return false;
        }

        switch (argument_type) {
            case TYPE_VOID:
                arguments += "(void*) data";
                break;
            case TYPE_INT8: {
                // Cast from const to a regular type for compiling
                // compatibility reasons
                arguments += "(char*) data";
            }
            default:
                break;
        }

        arguments += ",";
    }

    // Delete the last comma
    arguments.pop_back();

    return true;
}

void FuzzerGenerator::InsertFuzzerBody(std::string& body) {
    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);
    fuzzer_stub = std::regex_replace(fuzzer_stub, pattern, body);
}
