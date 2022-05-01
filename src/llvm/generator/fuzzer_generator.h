#ifndef AUTOFUZZ_FUZZER_GENERATOR_H
#define AUTOFUZZ_FUZZER_GENERATOR_H

#include "module.h"

#include <string>
#include <cstdint>
#include <vector>
#include <regex>

class FuzzerGenerator {
private:
    std::string HEADERS = R"(#include <cstdio>
#include <cstdint>
)";
    std::string FUZZER_STUB = R"(
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    $[fuzzer_body]$
    return 0;
}
)";

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
    bool InsertFuzzerBody(std::string&);
};

#endif //AUTOFUZZ_FUZZER_GENERATOR_H
