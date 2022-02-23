#include "pass_launcher.h"

#include <cstdio>

inline bool IsStartUpValid(int argc) { return argc == 2; }

int main(int argc, char** argv) {
    if (!IsStartUpValid(argc)) {
        fprintf(stderr, "Usage: %s <.ll>", argv[0]);
    }

    std::string ir_module(argv[1]);
    PassLauncher::LaunchOnIRModule(ir_module);

    return 0;
}