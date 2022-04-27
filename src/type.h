#ifndef AUTOFUZZ_TYPE_H
#define AUTOFUZZ_TYPE_H

#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <variant>
#include <algorithm>

enum BaseType : uint8_t {
    TYPE_UNKNOWN,
    TYPE_VOID,
    TYPE_INT8,  // + bool type
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_INT_UNKNOWN,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRUCT,
    TYPE_FUNC,
    TYPE_ARRAY
};

// ------------------------------------------------------------------------- //
//                     Basic information about a type                        //
// ------------------------------------------------------------------------- //
// A base type (aka header) of every variable
struct Type {
    explicit Type() : base_type_(TYPE_UNKNOWN), pointer_depth_(0),
    allocation_size_(0) {}
    virtual ~Type()              = default;
    Type(const Type&)            = delete;
    Type& operator=(const Type&) = delete;
    Type(Type&&)                 = delete;
    Type& operator=(Type&&)      = delete;

    BaseType base_type_;
    uint8_t  pointer_depth_;
    uint64_t allocation_size_; // how many bytes allocated for the type
};

// ------------------------------------------------------------------------- //
//                               Struct type                                 //
// ------------------------------------------------------------------------- //
// A pair consists of:
// 1. Offset of a field in the struct
// 2. Type of the field
using StructField = std::pair<uint64_t, std::unique_ptr<Type>>;

struct Body {
    explicit Body() : number_of_fields_(0), size_(0), alignment_(0) {}
    uint32_t                 number_of_fields_;
    uint64_t                 size_;
    uint16_t                 alignment_;
    std::vector<StructField> fields_;
};

// Content consists of a union:
// std::variant
//     |__std::shared_ptr<StructType> (the address to the struct with a body)
//     |__std::unique_ptr<Body>       (actual content (body) of the struct)
struct StructType;
using Content = std::variant<std::shared_ptr<StructType>,
                                                     std::unique_ptr<Body>>;

struct StructType : public Type {
    explicit StructType() : Type(), is_original_(true)  {}
    ~StructType() override                   = default;
    StructType(const StructType&)            = delete;
    StructType& operator=(const StructType&) = delete;
    StructType(StructType&&)                 = delete;
    StructType& operator=(StructType&&)      = delete;

    std::string name_;
    bool        is_original_;
    Content     content_;
};

// ------------------------------------------------------------------------- //
//                         Function type (pointer)                           //
// ------------------------------------------------------------------------- //
struct FunctionType : public Type {
    explicit FunctionType() : Type() {}
    ~FunctionType() override                     = default;
    FunctionType(const FunctionType&)            = delete;
    FunctionType& operator=(const FunctionType&) = delete;
    FunctionType(FunctionType&&)                 = delete;
    FunctionType& operator=(FunctionType&&)      = delete;
};

#endif //AUTOFUZZ_TYPE_H
