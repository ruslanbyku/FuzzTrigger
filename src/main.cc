#include "source_wrapper.h"
#include "logger.h"

#include <cstdio>

inline bool IsStartUpValid(int argc) { return argc == 2; }

//TODO: 1) Add support for relative paths
//      2) Add more log entries
//      3) Add support for projects' analysis (libraries included)
//      4) + resolve clang frontend parser to fetch functions' declarations
//      5) Add a parser of cmd arguments OR add a config to start the app
//      6) Add more working types for fuzzer generation
//      7) Comment

// /home/chinesegranny/CLionProjects/AutoFuzz/test/llvm_pass/test_main.c
int main(int argc, char** argv) {
    if (!IsStartUpValid(argc)) {
        fprintf(stderr, "Usage: %s <.c>\n", argv[0]);

        return EXIT_FAILURE;
    }

    StartLogging();

    bool result = false;
    try {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_INFO) << "Begin working with " << argv[1] << ".";
        }

        SourceWrapper wrapper(argv[1]);
        result = wrapper.LaunchRoutine();
    } catch (const std::runtime_error& exception) {
        if (LOGGER_ON) {
            LOG(LOG_LEVEL_ERROR) << exception.what();
        }
    }

    FinishLogging();

    if (!result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
