# GCC default include directories
# echo | gcc -Wp,-v -x {language} - -fsyntax-only

cmake_minimum_required(VERSION 3.21)
project(AutoFuzz)
set(CMAKE_CXX_STANDARD 20)

#
# Resolve LLVM API (https://llvm.org/docs/CMake.html#id18)
#
find_package(LLVM)
message(STATUS "LLVM API version:     ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVMConfig.cmake in:  ${LLVM_DIR}")
message(STATUS "LLVM header files in: ${LLVM_INCLUDE_DIRS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

include_directories(
        ${LLVM_INCLUDE_DIRS}
)
set(SOURCES src/main.cc src/module.cc src/module.h src/analysis.cc src/analysis.h src/type.h)

#
# Compilation flags
#
# Disable C++ native RTTI and EH.
# Use RTTI with llvm instead
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -D${LLVM_ENABLE_RTTI}:BOOL=OFF -D${LLVM_ENABLE_EH}:BOOL=OFF")

add_executable(AutoFuzz ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader analysis)
target_link_libraries(AutoFuzz ${llvm_libs})

message(STATUS "Complete.")