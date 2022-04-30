#ifndef AUTOFUZZ_WRAPPER_H
#define AUTOFUZZ_WRAPPER_H

#include "module.h"

class Wrapper {
public:
    virtual ~Wrapper()           = default;
    virtual bool LaunchRoutine() = 0;

protected:
    virtual bool PerformAnalysis()                                   = 0;
    virtual bool PerformGeneration(std::string,
                                   const std::unique_ptr<Function>&) = 0;
};

#endif //AUTOFUZZ_WRAPPER_H
