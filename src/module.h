/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZTRIGGER_MODULE_H
#define FUZZTRIGGER_MODULE_H

#include "type.h"

struct Function;
struct Argument;

using StandaloneFunctions = std::vector<std::shared_ptr<Function>>;

struct Module {
    explicit Module();
    ~Module()                        = default;
    Module(const Module&)            = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&)                 = delete;
    Module& operator=(Module&&)      = delete;

    explicit operator bool() const;

    bool                                     success_;

    std::string                              name_;
    std::string                              source_name_;

    // Definition of structs
    std::vector<std::unique_ptr<StructType>> structs_;

    // internal + external
    uint64_t                                 functions_number_;
    uint64_t                                 standalone_funcs_number_;
    // Registered only internal functions (external are discarded in process)
    std::vector<std::shared_ptr<Function>>   functions_;
    // Only standalone functions
    StandaloneFunctions                      standalone_functions_;
};

struct Function {
    explicit Function();
    ~Function()                          = default;
    Function(const Function&)            = delete;
    Function& operator=(const Function&) = delete;
    Function(Function&&)                 = delete;
    Function& operator=(Function&&)      = delete;

    std::string                            name_;
    bool                                   arguments_fixed_;
    std::unique_ptr<Type>                  return_type_;
    bool                                   is_local_;
    bool                                   is_standalone_;

    // Definition of arguments
    uint16_t                               arguments_number_;
    std::vector<std::unique_ptr<Argument>> arguments_;
};

struct Argument {
    explicit Argument();
    ~Argument()                          = default;
    Argument(const Argument&)            = delete;
    Argument& operator=(const Argument&) = delete;
    Argument(Argument&&)                 = delete;
    Argument& operator=(Argument&&)      = delete;

    uint16_t              index_;
    std::unique_ptr<Type> type_;
};

#endif //FUZZTRIGGER_MODULE_H
