# SSZ — Simple Serialize

[![License](https://img.shields.io/badge/license-BSL--1.0-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-%2300599C.svg)](https://en.cppreference.com/w/cpp/20)
[![Boost](https://img.shields.io/badge/Boost-PFR-green.svg)](https://github.com/boostorg/pfr)

**SSZ** 是一个轻量级、header-only 的 C++20 序列化库，基于 [Boost.PFR](https://github.com/boostorg/pfr) 实现编译期反射，支持对任意 C++ 结构体进行二进制序列化与反序列化，无需手写任何模板特化或宏。由于使用了Boost.PFR，所以 `SSZ` 的实现相当简单，您可以在此基础上轻松修改出自己需要序列化库。

## 特性

- **Header-only** — 仅需包含 `include/ssz/ssz.hpp` 即可使用
- **零配置序列化** — 借助 Boost.PFR 自动遍历结构体字段，无需注册或声明
- **字节序支持** — 默认不必关心字节序，可选择小端序输出

## 要求

- C++20 或更高版本
- [Boost](https://www.boost.org/)（需要 `Boost.PFR` 和 `Boost.UnitTestFramework` 用于测试）
- CMake 3.20+（仅用于构建测试）

## 使用

### 快速开始

```cpp
#include <ssz/ssz.hpp>
#include <cstdint>
#include <string>
#include <vector>

// 普通结构体，无需侵入或添加额外代码描述
struct Person {
    uint32_t id;
    std::string name;
    std::vector<uint32_t> scores;
};

int main() {
    Person orig{1, "Alice", {95, 87, 92}};

    // 序列化
    auto buf = ssz_pack::serialize(orig);

    // 反序列化
    Person result{};
    bool ok = ssz_pack::deserialize(buf, result);
    // ok == true, result == orig
}
```

### 序列化格式说明

```text
┌────────────────────────────────────────┐
│ 字节序标志 (uint8_t)                   │ ← 1 字节 (0=大端序, 1=小端序)
├────────────────────────────────────────┤
│ 类型哈希 (uint32_t, 字节序由标志决定)  │ ← 4 字节
├────────────────────────────────────────┤
│ 字段数据                               │ ← 按结构体字段顺序排列
│  ├─ 整数/浮点/平凡可复制: 按指定字节序 │
│  ├─ string:   长度(uint32_t) + 数据    │
│  ├─ vector:   数量(uint32_t) + 元素    │
│  └─ 嵌套结构体: 递归序列化             │
└────────────────────────────────────────┘
```

### 字节序

`ssz_pack::serialize()` 和 `ssz_pack::deserialize()` 的第二个参数 `little_endian`（默认为 `false`）控制字节序：

- `false` — 默认字节序（网络字节序）
- `true` — 使用小端序，当主机也为小端序时直接内存复制，性能更高

字节序标志写入序列化结果中，反序列化时自动识别，无需手动指定。

## 与 CMake 集成

```cmake
# 将 ssz 加入你的项目
add_subdirectory(path/to/ssz)

# 或直接引用头文件路径
target_include_directories(your_target PRIVATE /path/to/ssz/include)
```

## 鸣谢

本项目核心功能基于 [Boost.PFR](https://github.com/boostorg/pfr/)

本项部分设计灵感来自 [struct_pack](https://github.com/alibaba/yalantinglibs/tree/main/include/ylt/struct_pack)

## 许可

本项目基于 Boost Software License 1.0 开源。详见 [LICENSE](LICENSE) 文件。
