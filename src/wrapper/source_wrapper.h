#ifndef AUTOFUZZ_SOURCE_WRAPPER_H
#define AUTOFUZZ_SOURCE_WRAPPER_H

#include "single_function_source_parser.h"
#include "fuzzer_generator.h"
#include "pass_launcher.h"
#include "module.h"
#include "utils.h"
#include "compiler.h"
#include "virtual_mapper.h"
#include "file.h"
#include "logger.h"
#include "wrapper.h"

#include <exception>

class SourceWrapper : public Wrapper {
public:
    explicit SourceWrapper(std::string,
                           bool auto_deletion = true,
                           bool random_on = false,
                           bool override = true) noexcept(false);
    ~SourceWrapper() override;
    SourceWrapper(const SourceWrapper&)                = delete;
    SourceWrapper& operator=(const SourceWrapper&)     = delete;
    SourceWrapper(SourceWrapper&&) noexcept            = delete;
    SourceWrapper& operator=(SourceWrapper&&) noexcept = delete;

    bool LaunchRoutine() override;

private:
    File                    source_file_;
    File                    ir_source_file_;
    std::string             working_directory_;
    std::string             result_directory_path_;
    std::unique_ptr<Module> module_dump_;

    VirtualMapper           memory_;

    bool                    auto_deletion_;
    std::vector<File>       garbage_;
    bool                    random_on_;
    bool                    override_;

    // Heed this:
    // The method is called in the constructor of the current class, so it can
    // not be overridden from the Wrapper class that is a Base class. Why?
    // Because:
    // "In a constructor, the virtual call mechanism is disabled because
    // overriding from derived classes hasn’t yet happened. Objects are
    // constructed from the base up, “base before derived”".
    // Resource:
    // https://isocpp.org/wiki/faq/strange-inheritance#calling-virtuals-from-ctors
    void InitializeState();

    bool PerformAnalysis() override;
    bool PerformGeneration(std::string,
                           const std::unique_ptr<Function>&) override;

    void ConstructResultDirectoryPath();
    void ConstructFunctionDirectoryPath(const std::string&, std::string&);
    void ConstructFuzzerStubPath(const std::string&,
                                 const std::string&, std::string&);
    void ConstructFuzzerExecutablePath(const std::string&, std::string&);

    bool WriteFuzzerContentToFile(File&, const std::string&);

    bool CreateDirectory(const std::string&);

    void PlaceIntoTemporaryStorage(File&);
    void PlaceIntoGarbage(File&);
    void EmptyGarbage();
};

#endif //AUTOFUZZ_SOURCE_WRAPPER_H
