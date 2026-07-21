# ============================================================================
# SSZ 单元测试
# ============================================================================

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(test_src
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ssz.cpp
)

add_executable(test_ssz
    ${test_src}
)

target_link_libraries(test_ssz
    PRIVATE
    Boost::unit_test_framework
#   Boost::pfr
)

target_compile_features(test_ssz PRIVATE cxx_std_20)

target_include_directories(test_ssz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${BOOST_INCLUDE_DIRS}
)

add_test(NAME test_ssz COMMAND test_ssz)
