#ifndef AUTOFUZZ_PROJECT_WRAPPER_H
#define AUTOFUZZ_PROJECT_WRAPPER_H

#include "pass_launcher.h"
#include "compiler.h"
#include "module.h"
#include "file.h"
#include "logger.h"
#include "wrapper.h"

#include <fstream>
#include <exception>

class ProjectWrapper : public Wrapper {
public:
    explicit ProjectWrapper(std::string, std::string,
                            bool auto_deletion = true,
                            bool random_on = false,
                            bool override = true) noexcept(false);
    ~ProjectWrapper() override;
    ProjectWrapper(const ProjectWrapper&)                = delete;
    ProjectWrapper& operator=(const ProjectWrapper&)     = delete;
    ProjectWrapper(ProjectWrapper&&) noexcept            = delete;
    ProjectWrapper& operator=(ProjectWrapper&&) noexcept = delete;

    bool LaunchRoutine() override;

private:
    File                    ir_project_;
    File                    sources_;
    std::string             working_directory_;
    std::string             result_directory_path_;
    std::unique_ptr<Module> module_dump_;

    std::set<std::string>   source_paths_;

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
    bool PerformGeneration(const std::unique_ptr<Function>&) override;
};


#endif //AUTOFUZZ_PROJECT_WRAPPER_H
