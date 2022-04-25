#ifndef AUTOFUZZ_OPTIONS_H
#define AUTOFUZZ_OPTIONS_H

#include "llvm/Support/CommandLine.h"

llvm::cl::OptionCategory fuzzer_options("Fuzzer Options");

llvm::cl::opt<std::string> input_file(
        llvm::cl::desc("<input_file>"),
        llvm::cl::Positional,
        llvm::cl::Required
);

llvm::cl::opt<std::string> sources(
        llvm::cl::desc("Specify a file that contains every source path "
                       "of an analyzing project."),
        "sources",
        llvm::cl::value_desc("path"),
        llvm::cl::cat(fuzzer_options)
);

llvm::cl::alias sources_alias(
        llvm::cl::desc("Alias for --sources."),
        "s",
        llvm::cl::aliasopt(sources),
        llvm::cl::cat(fuzzer_options)
);

#endif //AUTOFUZZ_OPTIONS_H
