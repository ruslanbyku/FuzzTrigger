# CMAKE FOR TEST PURPOSES

cmake_minimum_required(VERSION 3.21)
project(FuzzTrigger)
set(CMAKE_CXX_STANDARD 20)

find_package(LLVM)
message(STATUS "LLVM API version:     ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVMConfig.cmake in:  ${LLVM_DIR}")
message(STATUS "LLVM header files in: ${LLVM_INCLUDE_DIRS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -D${LLVM_ENABLE_RTTI}:BOOL=OFF -D${LLVM_ENABLE_EH}:BOOL=OFF")

include_directories(
        ${LLVM_INCLUDE_DIRS}
        src/
)
set(SOURCES
        src/main.cc
        )

add_executable(FuzzTrigger ${SOURCES})

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(FuzzTrigger ${llvm_libs})

message(STATUS "Complete.")