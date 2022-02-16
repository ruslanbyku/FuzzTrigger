#include "analysis.h"

#include <cstdio>

inline bool IsStartUpValid(int argc) { return argc == 2; }

int main(int argc, char** argv) {
    if (!IsStartUpValid(argc)) {
        fprintf(stderr, "Usage: %s <.ll>", argv[0]);
    }

    Analysis analysis(argv[1]);

    return 0;
}