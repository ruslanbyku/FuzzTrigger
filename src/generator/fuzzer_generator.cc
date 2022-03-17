#include "fuzzer_generator.h"

std::string fuzzer_entry_point = R"(#include <cstdio>
#include <cstdint>
#include "$[file_with_function]$"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    $[fuzzer_body]$
    return 0;
}
)";

FuzzerGenerator::FuzzerGenerator(std::string source_file,
                                 const std::unique_ptr<Function>& function_dump)
                                 : source_file_path_(std::move(source_file)),
                                   function_dump_(function_dump) {}

const std::string &FuzzerGenerator::GetFuzzer() const {
    return fuzzer_;
}

bool FuzzerGenerator::Generate() {
    bool result;

    result = GenerateIntroPoint();
    if (!result) {
        return false;
    }

    // Append the constructed fuzzer intro function
    fuzzer_ += intro_point_;

    return true;
}

void FuzzerGenerator::UpdateHeader() {
    std::regex pattern(R"(\$\[file_with_function\]\$)");
    intro_point_ = std::regex_replace(fuzzer_entry_point, pattern, source_file_path_);
}


void FuzzerGenerator::UpdateFuzzerBody(std::string& body) {
    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);
    intro_point_ = std::regex_replace(intro_point_, pattern, body);
}


bool FuzzerGenerator::GenerateIntroPoint() {
    std::string body;
    bool result = GenerateFuzzerBody(body);
    if (!result) {
        return false;
    }

    // The body is ready, insert it to the fuzzer
    UpdateHeader();
    UpdateFuzzerBody(body);

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
                // Get rid of const
                arguments += "(const char*) data";
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