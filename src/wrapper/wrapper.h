#ifndef AUTOFUZZ_WRAPPER_H
#define AUTOFUZZ_WRAPPER_H

#include "fuzzer_generator.h"
#include "pass_launcher.h"
#include "compiler.h"
#include "virtual_mapper.h"
#include "logger.h"
#include "utils.h"
#include "file.h"
#include "module.h"

#include <exception>

class Wrapper {
public:
    virtual ~Wrapper()           = default;
    virtual bool LaunchRoutine() = 0;

protected:
    std::vector<File> garbage_;

    virtual bool PerformAnalysis()                                   = 0;
    virtual bool PerformGeneration(std::string,
                                   const std::unique_ptr<Function>&) = 0;

    bool CreateDirectory(const std::string&, bool);

    std::string ConstructResultDirectoryPath(
            const std::string&, const File&, bool);

    std::string ConstructFunctionDirectoryPath(
            const std::string&, const std::string&, bool);

    std::string ConstructFuzzerStubPath(const std::string&, const std::string&);

    std::string ConstructFuzzerExecutablePath(const std::string&);

    bool WriteFuzzerContentToFile(File&, const std::string&, bool);

    void PlaceIntoGarbage(File&);
    void EmptyGarbage(bool);
};

#endif //AUTOFUZZ_WRAPPER_H
