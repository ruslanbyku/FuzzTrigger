/*
 * Copyright 2022 Ruslan Byku
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FUZZTRIGGER_PROJECT_WRAPPER_H
#define FUZZTRIGGER_PROJECT_WRAPPER_H

#include "wrapper.h"

#include <fstream>

class ProjectWrapper : public Wrapper {
public:
    explicit ProjectWrapper(std::string, std::string, Options) noexcept(false);
    ~ProjectWrapper() override;
    ProjectWrapper(const ProjectWrapper&)                = delete;
    ProjectWrapper& operator=(const ProjectWrapper&)     = delete;
    ProjectWrapper(ProjectWrapper&&) noexcept            = delete;
    ProjectWrapper& operator=(ProjectWrapper&&) noexcept = delete;

    bool LaunchRoutine() override;

private:
    File                                ir_project_;
    File                                sources_;
    std::string                         working_directory_;
    std::string                         result_directory_path_;
    std::unique_ptr<Module>             module_dump_;

    std::set<std::string>               source_paths_;
    std::map<std::string, SourceEntity> function_declarations_;

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


#endif //FUZZTRIGGER_PROJECT_WRAPPER_H
