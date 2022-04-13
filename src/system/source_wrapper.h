#ifndef AUTOFUZZ_SOURCE_WRAPPER_H
#define AUTOFUZZ_SOURCE_WRAPPER_H

#include "frontend_action.h"
#include "fuzzer_generator.h"
#include "pass_launcher.h"
#include "module.h"
#include "utils.h"
#include "compiler.h"
#include "virtual_mapper.h"
#include "file.h"
#include "logger.h"
#include "ml_oracle.h"

#include <exception>

class SourceWrapper {
public:
    explicit SourceWrapper(std::string,
                           bool auto_deletion = true,
                           bool random_on = false,
                           bool override = true) noexcept(false);
    ~SourceWrapper();
    SourceWrapper(const SourceWrapper&)                = delete;
    SourceWrapper& operator=(const SourceWrapper&)     = delete;
    SourceWrapper(SourceWrapper&&) noexcept            = delete;
    SourceWrapper& operator=(SourceWrapper&&) noexcept = delete;

    bool LaunchRoutine();

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

    void InitializeState();

    bool PerformAnalysis();
    bool PerformGeneration(const std::unique_ptr<Function>&);

    void ConstructResultDirectoryPath();
    void ConstructFunctionDirectoryPath(const std::string&, std::string&);
    void ConstructFuzzerStubPath(const std::string&,
                                 const std::string&, std::string&);
    void ConstructFuzzerExecutablePath(const std::string&, std::string&);

    bool WriteFuzzerContentToFile(File&, const std::string&);

    bool CreateDirectory(const std::string&);

    void PlaceIntoGarbage(File&);
    void EmptyGarbage();
};

#endif //AUTOFUZZ_SOURCE_WRAPPER_H
