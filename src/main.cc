#include "source_wrapper.h"

#include <cstdio>

inline bool IsStartUpValid(int argc) { return argc == 2; }

// /home/chinesegranny/CLionProjects/AutoFuzz/test/llvm_pass/test_main.c
int main(int argc, char** argv) {
    if (!IsStartUpValid(argc)) {
        fprintf(stderr, "Usage: %s <.c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    SourceWrapper wrapper(argv[1]);
    bool result = wrapper.LaunchRoutine();
    if (!result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
