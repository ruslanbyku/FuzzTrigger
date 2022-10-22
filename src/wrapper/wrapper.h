#ifndef FUZZTRIGGER_WRAPPER_H
#define FUZZTRIGGER_WRAPPER_H

#include "full_source_parser.h"
#include "fuzzer_generator.h"
#include "pass_launcher.h"
#include "compiler.h"
#include "virtual_mapper.h"
#include "logger.h"
#include "utils.h"
#include "file.h"
#include "module.h"

#include <exception>

struct Options {
    bool auto_deletion_ = false;  // delete temporary files on exit
    bool random_on_     = false;  // create unique name files/directories
    bool override_      = true;   // override existing files
};

class Wrapper {
public:
    explicit Wrapper(Options);
    virtual ~Wrapper()           = default;

    virtual bool LaunchRoutine() = 0;

protected:
    Options           options_;
    std::vector<File> garbage_;

    virtual bool PerformAnalysis()                                   = 0;
    virtual bool PerformGeneration(std::string,
                                   const std::shared_ptr<Function>&,
                                   std::string)                      = 0;
    virtual std::string GetDeclaration(const std::string&) const     = 0;

    SourceEntity FindDeclarationsPerSource(const std::string&,
                                          const StandaloneFunctions&);

    std::string ConstructResultDirectoryPath(
            const std::string&, const File&, bool);

    std::string ConstructFunctionDirectoryPath(
            const std::string&, const std::string&, bool);

    bool CreateDirectory(const std::string&, bool);

    std::string ConstructFuzzerStubPath(const std::string&, const std::string&);

    std::string ConstructFuzzerExecutablePath(const std::string&);

    bool WriteFuzzerContentToFile(File&, const std::string&, bool);

    void PlaceIntoGarbage(File&);
    void EmptyGarbage(bool);
};

#endif //FUZZTRIGGER_WRAPPER_H
