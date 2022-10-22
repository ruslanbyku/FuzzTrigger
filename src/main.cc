// Copyright 2022 Ruslan Byku
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source_wrapper.h"
#include "project_wrapper.h"
#include "logger.h"
#include "options.h"

#include "cstdio"

//TODO: 1) Add support for relative paths
//      2) Add more log entries
//      3) !!! Comment
//      4) !!! Add support for projects' analysis (libraries included)

int main(int argc, char** argv) {
    llvm::cl::HideUnrelatedOptions(fuzzer_options);
    llvm::cl::ParseCommandLineOptions(argc, argv);

    StartLogging();

    std::string target = input_file.c_str();
    bool result        = false;

    if (LOGGER_ON) {
        LOG(LOG_LEVEL_INFO) << "Begin working with " << target << ".";
    }

    try {
        Options program_options;
        std::unique_ptr<Wrapper> wrapper;

        // Initialize a fuzzer state
        if (sources.empty()) {
            wrapper = std::make_unique<SourceWrapper>(target, program_options);
        } else {
            wrapper = std::make_unique<ProjectWrapper>(target,
                                                       sources.c_str(),
                                                       program_options);
        }

        // Run the fuzzer main routine
        result = wrapper->LaunchRoutine();

    } catch (const std::runtime_error& exception) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << exception.what();
        }
    }

    FinishLogging();

    if (!result) {
        fprintf(stdout, "Failure.\n");

        return EXIT_FAILURE;
    }

    fprintf(stdout, "Success.\n");

    return EXIT_SUCCESS;
}
