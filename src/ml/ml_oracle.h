#ifndef AUTOFUZZ_ML_ORACLE_H
#define AUTOFUZZ_ML_ORACLE_H

#include "module.h"

#include <array>
#include <cstdio>

const char* const call_oracle =
        "$HOME/CLionProjects/AutoFuzz/src/ml/call_oracle";

enum ArgumentClassification : uint8_t {
    PARAM_ABSENT = 0, // no parameter
    PARAM_STRING = 1, // void*, char*
    PARAM_DIGIT  = 2, // (int, float, double, bool) + pointers, char
    PARAM_STRUCT = 3  // object + pointers
};

class MLOracle {
public:
    explicit MLOracle(const std::vector<std::unique_ptr<Argument>>&);
    ~MLOracle()                          = default;
    MLOracle(const MLOracle&)            = delete;
    MLOracle& operator=(const MLOracle&) = delete;
    MLOracle(MLOracle&&)                 = delete;
    MLOracle& operator=(MLOracle&&)      = delete;

    bool GetVerdict() const;

private:
    const std::vector<std::unique_ptr<Argument>>& function_arguments_;

    bool CallOracle(const std::string&) const;
};

#endif //AUTOFUZZ_ML_ORACLE_H
