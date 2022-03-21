#include "module.h"

Module::Module() : success_(false), functions_number_(0) {}

Module::operator bool() const {
    return success_;
}

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
: linkage_(UNKNOWN_LINKAGE), arguments_fixed_(false), is_local_(true),
is_standalone_(false), arguments_number_(0) {}

Argument::Argument() : index_(0) {}
