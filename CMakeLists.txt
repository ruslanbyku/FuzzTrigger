# GCC default include directories
# echo | gcc -Wp,-v -x {language} - -fsyntax-only

cmake_minimum_required(VERSION 3.21)
project(AutoFuzz)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_VERBOSE_MAKEFILE on)

#
# Find LLVM API (https://llvm.org/docs/CMake.html#id18)
#
find_package(LLVM REQUIRED)
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(
        ${LLVM_INCLUDE_DIRS}
        src/
        src/analysis/
        src/generator/
        src/system/
)
set(SOURCES
        src/main.cc
        src/module.cc
        src/module.h
        src/analysis/analysis.cc
        src/analysis/analysis.h
        src/type.h
        src/analysis/cfg.h
        src/pass_launcher.h
        src/system/file.cc
        src/system/file.h
        src/generator/fuzzer_generator.h
        src/generator/fuzzer_generator.cc
        src/system/virtual_mapper.cc
        src/system/virtual_mapper.h
        src/generator/sanitizer.cc
        src/generator/sanitizer.h
        src/pass_launcher.cc
        src/generator/name_corrector.cc
        src/generator/name_corrector.h
        src/system/compiler.cc
        src/system/compiler.h
        src/utils.cc
        src/utils.h
        src/system/source_wrapper.cc
        src/system/source_wrapper.h
        )

#
# Compilation flags
#
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# Disable C++ native RTTI and EH (use RTTI with llvm instead)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti") # -fno-exceptions


add_executable(AutoFuzz ${SOURCES})

#
# Link LLVM
#
target_link_libraries(AutoFuzz LLVM)

#
# Debug messages
#
message(STATUS "CXX_FLAGS:            ${CMAKE_CXX_FLAGS}")
message(STATUS "LLVM API version:     ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVMConfig.cmake in:  ${LLVM_DIR}")
message(STATUS "LLVM header files in: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Complete.\n")