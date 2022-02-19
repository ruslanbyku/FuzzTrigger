#ifndef AUTOFUZZ_MODULE_H
#define AUTOFUZZ_MODULE_H

#include "type.h"

struct Function;
struct Argument;

struct Module {
    explicit Module();
    ~Module()                        = default;
    Module(const Module&)            = delete;
    Module& operator=(const Module&) = delete;
    Module(Module&&)                 = delete;
    Module& operator=(Module&&)      = delete;

    std::shared_ptr<StructType> GetStructByName(const std::string&) const;

    std::string                              name_;
    std::string                              source_name_;

    // Definition of structs
    std::set<std::string>                    struct_list_;
    std::vector<std::shared_ptr<StructType>> structs_;

    // Definition of functions (external + internal)
    uint64_t                                 functions_number_;
    std::vector<std::unique_ptr<Function>>   functions_;
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

#endif //AUTOFUZZ_MODULE_H
