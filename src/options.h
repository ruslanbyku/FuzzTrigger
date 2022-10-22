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

#ifndef FUZZTRIGGER_OPTIONS_H
#define FUZZTRIGGER_OPTIONS_H

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

#endif //FUZZTRIGGER_OPTIONS_H
