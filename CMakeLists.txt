# SSZ (Simple Serialize)

cmake_minimum_required(VERSION 3.20)
project(ssz LANGUAGES CXX)

option(ENABLE_SSZ_TESTS "Build SSZ unit tests" OFF)
option(ENABLE_SSZ_EXAMPLES "Build SSZ examples" ON)

if(ENABLE_SSZ_TESTS)
    add_subdirectory(tests)
endif()

if(ENABLE_SSZ_EXAMPLES)
    add_subdirectory(example)
endif()
