#include "module.h"

Module::Module()
: success_(false), functions_number_(0), standalone_funcs_number_(0) {}

Module::operator bool() const {
    return success_;
}

Function::Function()
: linkage_(UNKNOWN_LINKAGE), arguments_fixed_(false), is_local_(true),
is_standalone_(false), arguments_number_(0) {}

Argument::Argument() : index_(0) {}
