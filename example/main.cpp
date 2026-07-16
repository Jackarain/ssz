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
#include <string>
#include <vector>
#include <iostream>

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

    std::cout << "序列化成功! 数据大小: " << buf.data.size() << " 字节\n";

    // 3. 反序列化回结构体。
    Person result{};
    bool ok = ssz::deserialize(buf, result);

    if (ok)
    {
        std::cout << "反序列化成功!\n";
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
