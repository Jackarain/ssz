# SSZ (Simple Serialize)

cmake_minimum_required(VERSION 3.20)
project(ssz LANGUAGES CXX)

option(ENABLE_SSZ_TESTS "Build SSZ unit tests" ON)

if(ENABLE_SSZ_TESTS)
    add_subdirectory(tests)
endif()
