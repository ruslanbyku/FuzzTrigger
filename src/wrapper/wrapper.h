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
