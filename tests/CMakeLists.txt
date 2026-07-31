# ============================================================================
# SSZ 单元测试
# ============================================================================
#
# 单元测试使用自包含的轻量测试框架（tests/unit_test.hpp），
# 序列化核心所需的 Boost.PFR 头文件已捆绑在 include/boost 中，
# 因此本项目测试不依赖外部 Boost 库。

set(test_src
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ssz.cpp
)

add_executable(test_ssz
    ${test_src}
)

target_compile_features(test_ssz PRIVATE cxx_std_20)

target_include_directories(test_ssz PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_test(NAME test_ssz COMMAND test_ssz)
