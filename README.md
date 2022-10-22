# FuzzTrigger

[![Dependency](https://img.shields.io/badge/dependency-llvm/clang_(14.0.6)-informational.svg)](https://github.com/llvm/llvm-project)
[![Dependency](https://img.shields.io/badge/dependency-wllvm-informational.svg)](https://github.com/travitch/whole-program-llvm)
![Platform](https://img.shields.io/badge/platform-linux-informational.svg)
[![License](https://img.shields.io/badge/license-apache--2.0-blueviolet.svg)](https://opensource.org/licenses/Apache-2.0)

### Introduction

*FuzzTrigger*, a tool that triggers generation of fuzzer programs for C source
code.

The core idea behind *FuzzTrigger* is to find presumably a vulnerable piece of
code in the analyzed project and write a fuzzer program to test the robustness
of the found code.

*FuzzTrigger* was initially the basis for my master thesis in the field of
Information Security and inspired by 
[FuzzGen](https://github.com/HexHive/FuzzGen/).

### Status

*FuzzTrigger* is **currently under development** and already ready to go.
However, the project is at its initial stage and some functionality may not
be available at the moment.

### How it works

*FuzzTrigger* performs:

1. **Analysis** of C code with the help of LLVM/libclang and search for the 
so-called standalone functions that possess a minimal amount of dependencies
from the rest code in the analyzed project. These functions might be safely
extracted from the code without breaking inner dependencies. 
2. **Generation** (writing) of fuzzer code for each standalone function. 
3. **Consolidation** of two pieces together (the code of a standalone function 
and the code of fuzzer) into a single file and further compilation of the
final file into an executable file.

As a result of the above steps many fuzzer programs are created. Each fuzzer
program corresponds to a particular standalone function. The above method can
be used to test the code for bugs and vulnerabilities.

*FuzzTrigger* works most of the time on the IR level and uses
[libFuzzer](https://llvm.org/docs/LibFuzzer.html) and
[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html).

### Key features

1) Ability to work with both **a shared library** and
**a system executable file** (source code must be available).
2) Ability to work with both **a full-scale project** on autotools/cmake and
**a single executable file**.
3) Ability to **support other fuzzer engines**, for instance,
AFL/AFL++ (currently it is available only manually).
4) A small number of dependencies.
5) The dependencies used in the project are give or take up to date.

### Dependencies

* [LLVM/Clang](https://github.com/llvm/llvm-project) (14.0.6)
* [wllvm](https://github.com/travitch/whole-program-llvm) (but not directly by *FuzzTrigger*)

### Supported platforms

*FuzzTrigger* is developed, tested and works on Arch Linux x86_64 
(5.19.12-arch1-1).

### License
Licensed under the [Apache 2.0](LICENSE) License.