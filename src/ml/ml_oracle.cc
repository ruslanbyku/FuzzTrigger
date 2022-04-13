#include "ml_oracle.h"

MLOracle::MLOracle(
        const std::vector<std::unique_ptr<Argument>>& function_arguments)
        : function_arguments_(function_arguments) {}

bool MLOracle::GetVerdict() const {
    std::string classified_arguments;
    std::string banned_symbol = std::to_string(PARAM_ABSENT);

    for (const auto& argument: function_arguments_) {
        bool is_pointer        = argument->type_->pointer_depth_ > 0;
        BaseType argument_type = argument->type_->base_type_;

        switch (argument_type) {
            case TYPE_VOID:
                if (!is_pointer) { // void
                    classified_arguments += banned_symbol;
                } else {           // void*
                    classified_arguments += std::to_string(PARAM_STRING);
                }
                break;
            case TYPE_INT8:
                if (is_pointer) { // char*
                    classified_arguments += std::to_string(PARAM_STRING);
                    break;
                }
            case TYPE_INT16:
            case TYPE_INT32:
            case TYPE_INT64:
            case TYPE_INT_UNKNOWN:
            case TYPE_FLOAT:
            case TYPE_DOUBLE:
            case TYPE_BOOL: // + char
                classified_arguments += std::to_string(PARAM_DIGIT);
                break;
            case TYPE_STRUCT:
                classified_arguments += std::to_string(PARAM_STRUCT);
                break;
            case TYPE_FUNC:
            case TYPE_ARRAY:
            case TYPE_UNKNOWN:
            default:
                classified_arguments += banned_symbol;
        }
    }


    if (classified_arguments.find(banned_symbol) != std::string::npos) {
        return false;
    }

    return CallOracle(classified_arguments);
}

bool MLOracle::CallOracle(const std::string& classified_arguments) const {
    const uint8_t buffer_length            = 255;
    std::array<char, buffer_length> buffer = {};
    std::string result;
    std::string command;

    command += call_oracle;
    command += " ";
    command += classified_arguments;

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }

    while (!feof(pipe)) {
        if (fgets(buffer.data(), buffer_length, pipe) != nullptr) {
            result += buffer.data();
        }
    }

    pclose(pipe);

    if (result == "1") {
        return true;
    }

    return false;
}
