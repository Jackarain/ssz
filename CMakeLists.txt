# SSZ (Simple Serialize)

cmake_minimum_required(VERSION 3.20)
project(ssz LANGUAGES CXX)

option(ENABLE_SSZ_TESTS "Build SSZ unit tests" OFF)
option(ENABLE_SSZ_EXAMPLES "Build SSZ examples" ON)

# 启用 CTest 测试注册（add_test），使 `ctest` 可运行单元测试。
enable_testing()

# Windows/MSVC 下源文件为 UTF-8 编码（无 BOM），MSVC 默认按系统代码页（如 1252）
# 解析源码，导致中文注释/字符串产生 C4566 字符集警告。
# 显式指定 /utf-8，使源码字符集与执行字符集均为 UTF-8，消除警告并保证运行时中文输出正确。
if(MSVC)
    add_compile_options(/utf-8)
endif()

if(ENABLE_SSZ_TESTS)
    add_subdirectory(tests)
endif()

if(ENABLE_SSZ_EXAMPLES)
    add_subdirectory(example)
endif()
