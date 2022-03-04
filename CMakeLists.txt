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

#
# Find Clang API
#
# https://bugs.archlinux.org/task/66283#comments
find_library(clang_lib clang)

include_directories(
        ${LLVM_INCLUDE_DIRS}
        src/
        src/analysis/
        src/generator/
        src/file/
)
set(SOURCES
        src/main.cc
        src/analysis/module.cc
        src/analysis/module.h
        src/analysis/analysis.cc
        src/analysis/analysis.h
        src/analysis/type.h
        src/analysis/cfg.h
        src/analysis/pass_launcher.h
        src/file/file.cc
        src/file/file.h
        src/generator/fuzzer_generator.h
        src/generator/fuzzer_generator.cc
        src/generator/frontend_action.cc
        src/generator/frontend_action.h
        src/file/virtual_mapper.cc
        src/file/virtual_mapper.h
        src/analysis/ir_compiler.cc src/analysis/ir_compiler.h)

#
# Compilation flags
#
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
# Disable C++ native RTTI and EH (use RTTI with llvm instead)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")


add_executable(AutoFuzz ${SOURCES})

#
# Link LLVM + Clang API
#
# llvm_map_components_to_libnames(...) does not work with clang, see the issue
# here (https://github.com/CastXML/CastXML/issues/102#issuecomment-372488956)
#llvm_map_components_to_libnames(llvm_libs core irreader support analysis)
target_link_libraries(AutoFuzz LLVM ${clang_lib})

#
# Debug messages
#
message(STATUS "CXX_FLAGS:            ${CMAKE_CXX_FLAGS}")
message(STATUS "LLVM API version:     ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVMConfig.cmake in:  ${LLVM_DIR}")
message(STATUS "LLVM header files in: ${LLVM_INCLUDE_DIRS}")
message(STATUS "CLANG lib:            ${clang_lib}")
message(STATUS "Complete.\n")