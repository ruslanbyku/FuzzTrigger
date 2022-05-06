#ifndef AUTOFUZZ_FUZZER_GENERATOR_H
#define AUTOFUZZ_FUZZER_GENERATOR_H

#include "module.h"

#include <string>
#include <cstdint>
#include <vector>
#include <regex>

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

    std::string GenerateFuzzerBody();
    std::pair<std::string, std::string>
                               CreateNumericVariable(BaseType, uint16_t);
    inline std::string GetStars(uint8_t pointer_depth);
    std::string InsertFuzzerBody(const std::string&);

    bool HasPlainTypeInArguments();
    bool NeedStructForwardDeclaration();
    std::string MakeStructForwardDeclaration();
};

#endif //AUTOFUZZ_FUZZER_GENERATOR_H
