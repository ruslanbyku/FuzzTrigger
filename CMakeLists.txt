# GCC default include directories
# echo | gcc -Wp,-v -x {language} - -fsyntax-only

cmake_minimum_required(VERSION 3.21)
project(AutoFuzz)
set(CMAKE_CXX_STANDARD 20)

#
# Find LLVM API (https://llvm.org/docs/CMake.html#id18)
#
find_package(LLVM REQUIRED)
add_definitions(${LLVM_DEFINITIONS_LIST})

#
# Find Clang API (https://bugs.archlinux.org/task/66283#comments)
#
# NOTE:
# Having one CMakeLists.txt may lead to different compilation results.
#
# Using just clang instead of clang-cpp CLion automatically links to the right
# library - libclang-cpp.so, but when compile manually with cmake/make an
# error occurs - libclang.so is used, what is unacceptable for C++.
#
# To avoid such conduct, state explicitly clang-cpp to find libclang-cpp.so.
find_library(clang_lib clang-cpp)

include_directories(
        ${LLVM_INCLUDE_DIRS}
        src/
        src/llvm/
        src/llvm/analysis/
        src/llvm/generator/
        src/system/
        src/log/
        src/wrapper/
)
set(SOURCES
        src/main.cc
        src/module.cc
        src/module.h
        src/llvm/analysis/analysis.cc
        src/llvm/analysis/analysis.h
        src/type.h
        src/llvm/analysis/cfg.h
        src/llvm/pass_launcher.h
        src/system/file.cc
        src/system/file.h
        src/llvm/generator/fuzzer_generator.h
        src/llvm/generator/fuzzer_generator.cc
        src/system/virtual_mapper.cc
        src/system/virtual_mapper.h
        src/llvm/generator/sanitizer.cc
        src/llvm/generator/sanitizer.h
        src/llvm/pass_launcher.cc
        src/llvm/generator/name_corrector.cc
        src/llvm/generator/name_corrector.h
        src/system/compiler.cc
        src/system/compiler.h
        src/utils.cc
        src/utils.h
        src/log/logger.h
        src/wrapper/wrapper.h
        src/wrapper/project_wrapper.cc
        src/wrapper/project_wrapper.h
        src/wrapper/source_wrapper.cc
        src/wrapper/source_wrapper.h
        src/options.h
        src/llvm/generator/function_declaration.h
        src/llvm/generator/function_declaration.cc
        src/llvm/generator/full_source_parser.cc
        src/llvm/generator/full_source_parser.h
        src/wrapper/wrapper.cc
        )

#
# Compilation flags
#
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# Disable C++ native RTTI and EH (use RTTI with llvm instead)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti") # -fno-exceptions
# Turn on basic optimization
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")

add_executable(autofuzz ${SOURCES})

#
# Link LLVM
#
target_link_libraries(autofuzz LLVM ${clang_lib})

#
# Debug messages
#
message(STATUS "CXX_FLAGS:            ${CMAKE_CXX_FLAGS}")
message(STATUS "LLVM API version:     ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVMConfig.cmake in:  ${LLVM_DIR}")
message(STATUS "LLVM header files in: ${LLVM_INCLUDE_DIRS}")
message(STATUS "CLANG lib:            ${clang_lib}")
message(STATUS "Complete.\n")