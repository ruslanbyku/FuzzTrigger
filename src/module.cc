#include "module.h"

Module::Module() : functions_number_(0) {}

std::shared_ptr<StructType>
        Module::GetStructByName(const std::string& struct_name) const {
    for (auto& struct_: structs_) {
        if (struct_->name_ == struct_name) {
            return struct_;
        }
    }

    return nullptr;
}

Function::Function()
: arguments_fixed_(false), is_local_(true), arguments_number_(0) {}

Argument::Argument() : index_(0) {}
