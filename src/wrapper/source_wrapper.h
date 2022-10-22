#ifndef FUZZTRIGGER_SOURCE_WRAPPER_H
#define FUZZTRIGGER_SOURCE_WRAPPER_H

#include "wrapper.h"

class SourceWrapper : public Wrapper {
public:
    explicit SourceWrapper(std::string, Options) noexcept(false);
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

    SourceEntity            source_entity_;

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
                           const std::shared_ptr<Function>&,
                           std::string) override;

    std::string GetDeclaration(const std::string&) const override;
};

#endif //FUZZTRIGGER_SOURCE_WRAPPER_H
