cmake_minimum_required(VERSION 3.16)
project(module-dumper VERSION 0.1 LANGUAGES CXX)

set(LLVM_DIR "/usr" CACHE PATH "Path to LLVM installation")
set(LLVM_INCLUDE_DIR "${LLVM_DIR}/include" CACHE PATH "Path to LLVM includes")
set(LLVM_LIBRARY_DIR "${LLVM_DIR}/lib" CACHE PATH "Path to LLVM libraries")

find_library(CLANG clang HINTS ${LLVM_LIBRARY_DIR} REQUIRED)
find_library(CLANGPP clang-cpp HINTS ${LLVM_LIBRARY_DIR} REQUIRED)
find_library(LLVM LLVM HINTS ${LLVM_LIBRARY_DIR} REQUIRED)

add_executable(module-dumper ModuleDumper.cpp)
target_compile_features(module-dumper PRIVATE cxx_std_17)
target_include_directories(module-dumper PRIVATE ${LLVM_INCLUDE_DIR})
target_link_libraries(module-dumper PRIVATE ${CLANG} ${CLANGPP} ${LLVM})

