#include "fuzzer_generator.h"

FuzzerGenerator::FuzzerGenerator(std::string function_declaration,
                                 const std::shared_ptr<Function>& function_dump)
: function_declaration_(std::move(function_declaration)),
function_dump_(function_dump) {}

const std::string &FuzzerGenerator::GetFuzzer() const {
    return fuzzer_;
}

bool FuzzerGenerator::Generate() {
    std::pair<bool, std::string> result = GenerateFuzzerBody();
    if (!result.first) {
        // An error occurred during generation
        return false;
    }

    if (!InsertFuzzerBody(result.second)) {
        // Can not insert the body, nothing to insert
        return false;
    }

    fuzzer_ += HEADERS;
    fuzzer_ += function_declaration_;
    fuzzer_ += FUZZER_STUB;

    return true;
}

std::pair<bool, std::string> FuzzerGenerator::GenerateFuzzerBody() {
    std::string body;

    body += "(void) ";
    body += function_dump_->name_;
    body += "(";

    std::pair<bool, std::string> result = GenerateArguments();
    if (!result.first) {
        // Some arguments are unsupported
        return std::make_pair(false, body);
    }
    body += result.second;

    body += ");";

    return std::make_pair(true, body);
}

std::pair<bool, std::string> FuzzerGenerator::GenerateArguments() {
    std::string arguments;

    for (const auto& argument: function_dump_->arguments_) {
        bool is_pointer        = argument->type_->pointer_depth_ > 0;
        uint8_t pointer_depth  = argument->type_->pointer_depth_;
        BaseType argument_type = argument->type_->base_type_;

        // Do not support more than one asterisk (**)
        if (pointer_depth > 1) {
            return std::make_pair(false, "");
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
            return std::make_pair(false, "");
        }

        // TYPE_VOID and TYPE_INT8

        // Check if the type is a pointer
        if (!is_pointer) {
            // Not a pointer
            return std::make_pair(false, "");
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

    if (!arguments.empty()) {
        // If the arguments are not empty, at least one iteration was made -
        // at least one argument was created
        arguments.pop_back(); // Delete the last comma
    }

    return std::make_pair(true, arguments);
}

bool FuzzerGenerator::InsertFuzzerBody(std::string& body) {
    if (body.empty()) {
        return false;
    }

    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);
    FUZZER_STUB = std::regex_replace(FUZZER_STUB, pattern, body);

    return true;
}
