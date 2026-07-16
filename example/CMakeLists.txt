# SSZ 使用示例

add_executable(ssz_example main.cpp)
target_include_directories(ssz_example PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_features(ssz_example PRIVATE cxx_std_20)
