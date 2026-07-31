# SSZ (Simple Serialize)

cmake_minimum_required(VERSION 3.20)
project(ssz LANGUAGES CXX)

option(ENABLE_SSZ_TESTS "Build SSZ unit tests" OFF)
option(ENABLE_SSZ_EXAMPLES "Build SSZ examples" ON)

# 启用 CTest 测试注册（add_test），使 `ctest` 可运行单元测试。
enable_testing()

if(ENABLE_SSZ_TESTS)
    add_subdirectory(tests)
endif()

if(ENABLE_SSZ_EXAMPLES)
    add_subdirectory(example)
endif()
