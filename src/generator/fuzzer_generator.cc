#include "fuzzer_generator.h"

std::string fuzzer_entry_point = R"(
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    $[fuzzer_body]$
    return 0;
}
)";

FuzzerGenerator::FuzzerGenerator(std::string source_file,
                                 const std::unique_ptr<Function>& function_dump)
                                 : source_file_(std::move(source_file)),
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

    result = FindDependencyHeaders();
    if (!result) {
        return false;
    }

    result = FindTargetFunction();
    if (!result) {
        return false;
    }

    // All data has been gathered, construct the fuzzer
    // Construct headers
    for (std::string& header: headers_) {
        fuzzer_ += "#include <";
        fuzzer_ += header;
        fuzzer_ += ">\n";
    }
    fuzzer_ += "\n";

    // Append the target function
    fuzzer_ += function_;
    fuzzer_ += "\n";

    // Append the constructed fuzzer intro function
    fuzzer_ += intro_point_;

    return true;
}

bool FuzzerGenerator::FindDependencyHeaders() {
    //TODO: Resolve local headers

    const std::regex pattern(".*?\\#include.*?<(.+?)>");
    // Get value only in parentheses
    const uint8_t target = 1;

    auto text_start = std::sregex_iterator(source_file_.begin(),
                                      source_file_.end(), pattern);
    auto text_end = std::sregex_iterator();
    int64_t hits_amount = std::distance(text_start, text_end);

    if (hits_amount <= 0) {
        return false;
    }


    for (std::sregex_iterator ii = text_start; ii != text_end; ++ii) {
        std::smatch match = *ii;

        headers_.push_back(match[target].str());
    }

    return true;
}

bool FuzzerGenerator::FindTargetFunction() {
    FunctionLocation location(function_dump_->name_);
    clang::tooling::runToolOnCode(
            std::make_unique<FrontendAction>(location),
                    source_file_
                    );

    // Check if the function was found
    if (!location.is_filled_) {
        // Not found
        return false;
    }

    if (location.entity_.empty()) {
        // The found was not actually found
        return false;
    }

    function_ = location.entity_;

    return true;
}

bool FuzzerGenerator::GenerateIntroPoint() {
    std::string body;
    bool result = GenerateFuzzerBody(body);
    if (!result) {
        return false;
    }

    // The body is ready, insert it to the fuzzer
    std::regex pattern(R"(\$\[fuzzer_body\]\$)", std::regex::ECMAScript);
    intro_point_ = std::regex_replace(fuzzer_entry_point, pattern, body);

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
