#ifndef AUTOFUZZ_FUZZER_GENERATOR_H
#define AUTOFUZZ_FUZZER_GENERATOR_H

#include "module.h"

#include <regex>
#include <queue>

struct TargetFunctionArgument {
    std::string              forward_decl;

    std::string              variable_type;
    std::string              variable_name;
    std::string              variable_init;

    std::vector<std::string> before_call;

    std::string              call_arg;

    std::vector<std::string> after_call;

    bool additional_hdrs = false;
    bool support_funcs   = false;
};

using TargetFunctionArgumentOptions =
        std::vector<std::unique_ptr<TargetFunctionArgument>>;

using TargetFunctionArgumentQueue = std::queue<TargetFunctionArgumentOptions>;

using CallArguments = std::vector<std::vector<std::string>>;

class FuzzerGenerator {
public:
    explicit FuzzerGenerator(std::string, const std::shared_ptr<Function>&);

    bool Generate();
    const std::string& GetFuzzer() const;

private:
    std::string                      function_declaration_;
    const std::shared_ptr<Function>& function_dump_;

    std::string                      fuzzer_;

    bool IsSupported();

    std::string ConstructFuzzerStubBody(const std::string&,
                                        const std::string&,
                                        const std::string&,
                                        const CallArguments&);
    std::string InsertFuzzerStubBody(const std::string&);

    bool ProcessFunctionArguments(TargetFunctionArgumentQueue&);

    TargetFunctionArgumentOptions
                          ProcessArgumentVoid(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                          ProcessArgumentInt8(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                         ProcessArgumentInt16(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                         ProcessArgumentInt32(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                         ProcessArgumentInt64(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                         ProcessArgumentFloat(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                        ProcessArgumentDouble(const std::unique_ptr<Argument>&);
    TargetFunctionArgumentOptions
                        ProcessArgumentStruct(const std::unique_ptr<Argument>&);

    TargetFunctionArgumentOptions
                           HandleNumeric(const std::unique_ptr<Argument>&);

    inline std::string GetStars(uint8_t pointer_depth);
};

#endif //AUTOFUZZ_FUZZER_GENERATOR_H
