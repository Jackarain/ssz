//
// Copyright (C) 2026 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

// 最简单的 SSZ 序列化/反序列化示例
//
// 编译方法:
//   g++ -std=c++20 -I../include -o example main.cpp
//
// 或使用 CMake:
//   mkdir build && cd build && cmake .. && make

#include <ssz/ssz.hpp>

#include <cstdint>
#include <cctype>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <format>

// 以二进制查看器风格输出十六进制转储。
void hex_dump(std::ostream& os, const std::vector<char>& data)
{
    constexpr int bytes_per_line = 16;

    for (size_t offset = 0; offset < data.size(); offset += bytes_per_line)
    {
        // 输出偏移地址。
        os << std::format("{:08x}  ", offset);

        size_t remaining = data.size() - offset;
        size_t line_len = (remaining < bytes_per_line) ? remaining : bytes_per_line;

        // 输出十六进制字节。
        for (size_t i = 0; i < bytes_per_line; i++)
        {
            if (i < line_len)
                os << std::format("{:02x} ", static_cast<unsigned char>(data[offset + i]));
            else
                os << "   "; // 填充空格对齐。

            // 每 8 个字节额外加一个空格分组。
            if (i == 7)
                os << " ";
        }

        os << " |";

        // 输出 ASCII 表示（可打印字符原样输出，其他输出 '.'）。
        for (size_t i = 0; i < line_len; i++)
        {
            unsigned char c = static_cast<unsigned char>(data[offset + i]);
            os << (std::isprint(c) ? static_cast<char>(c) : '.');
        }

        os << "|\n";
    }
}

// 定义一个简单的数据结构，无需任何侵入式代码。
struct Person
{
    uint32_t id;
    std::string name;
    std::vector<uint32_t> scores;
};

int main()
{
    // 1. 构造原始数据。
    Person alice{1, "Alice", {95, 87, 92}};

    // 2. 序列化为二进制缓冲区。
    auto buf = ssz::serialize(alice);

    std::cout << "序列化成功! 数据大小: " << buf.data.size() << " 字节\n\n";

    // 3. 以十六进制转储格式输出序列化后的数据。
    std::cout << "===== 序列化数据 (十六进制转储) =====\n";
    hex_dump(std::cout, buf.data);

    // 4. 反序列化回结构体。
    Person result{};
    bool ok = ssz::deserialize(buf, result);

    if (ok)
    {
        std::cout << "\n反序列化成功!\n";
        std::cout << "  id:     " << result.id << "\n";
        std::cout << "  name:   " << result.name << "\n";
        std::cout << "  scores: ";
        for (auto s : result.scores)
            std::cout << s << " ";
        std::cout << "\n";

        // 验证数据一致。
        if (result.id == alice.id &&
            result.name == alice.name &&
            result.scores.size() == alice.scores.size())
        {
            bool scores_match = true;
            for (size_t i = 0; i < alice.scores.size(); i++)
            {
                if (result.scores[i] != alice.scores[i])
                    scores_match = false;
            }
            if (scores_match)
                std::cout << "数据完全一致!\n";
        }
    }
    else
    {
        std::cout << "反序列化失败!\n";
        return 1;
    }

    return 0;
}
