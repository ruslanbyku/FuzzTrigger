# AutoFuzz

### Introduction

A program that writes small fuzzer programs for source code 
on C. The core idea behind the project is to find the 
so-called standalone functions that possess a minimal 
amount of dependencies from the rest code in the analyzed 
project. These functions might be safely extracted from the
code without breaking inner dependencies and small fuzzer
code is written for them. Then the two pieces are combined
into one entity and compiled into an executable fuzzer
program that can test the code for bugs and vulnerabilities.

*AutoFuzz* is a tentative and provisional name, so it may
be changed over time. It uses
[libFuzzer](https://llvm.org/docs/LibFuzzer.html) and
[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) in
its core.

The project was initially a basis for my master thesis and 
inspired by [FuzzGen](https://github.com/HexHive/FuzzGen/).
Currently, under development, readiness ~80%.

### Dependencies
* llvm/clang (13.0.1)
* [wllvm](https://github.com/travitch/whole-program-llvm) (but not directly by *AutoFuzz*)

### License
None