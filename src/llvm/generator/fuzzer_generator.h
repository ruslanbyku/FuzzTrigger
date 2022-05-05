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

    std::pair<bool, std::string> GenerateFuzzerBody();
    std::pair<bool, std::string> GenerateArguments();
    std::string InsertFuzzerBody(const std::string&);
};

#endif //AUTOFUZZ_FUZZER_GENERATOR_H
