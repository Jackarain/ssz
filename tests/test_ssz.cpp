//
// Copyright (C) 2026 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

#define BOOST_TEST_MODULE ssz_test
#include <boost/test/included/unit_test.hpp>

#include <ssz/ssz.hpp>

#include <cstdint>
#include <string>
#include <vector>
#include <array>

// ============================================================================
// 测试用辅助结构体
// ============================================================================

struct empty_struct
{};

struct simple_integrals
{
    uint8_t a;
    uint16_t b;
    uint32_t c;
    uint64_t d;
    int8_t e;
    int16_t f;
    int32_t g;
    int64_t h;
};

struct with_string
{
    uint32_t id;
    std::string name;
    std::string desc;
};

struct with_vector_integral
{
    uint32_t id;
    std::vector<uint32_t> scores;
    std::vector<int8_t> flags;
};

struct with_vector_float
{
    uint32_t id;
    std::vector<float> values;
};

struct with_vector_complex
{
    uint32_t id;
    std::vector<uint8_t> data;
    std::string name;
    std::vector<uint64_t> amounts;
};

struct mixed_types
{
    uint32_t id;
    std::string label;
    std::vector<uint16_t> tags;
    float ratio;
    uint64_t timestamp;
    std::vector<int32_t> offsets;
};

struct large_data
{
    uint32_t index;
    std::vector<uint8_t> blob;
    std::string payload;
};

// 用于验证类型哈希不匹配的结构体（与 simple_integrals 字段类型不同）
struct wrong_integrals
{
    uint32_t a;      // simple_integrals 中是 uint8_t
    uint16_t b;
    uint32_t c;
    uint64_t d;
};

// ============================================================================
// 嵌套结构体测试用辅助结构体
// ============================================================================

// 简单嵌套结构体
struct inner_struct
{
    uint32_t x;
    uint64_t y;
    std::string label;
};

struct with_nested_struct
{
    uint32_t id;
    inner_struct inner;
    std::string name;
};

// 多层嵌套结构体
struct deep_inner
{
    uint32_t a;
    std::string b;
};

struct deep_middle
{
    deep_inner inner;
    std::vector<uint8_t> data;
};

struct deep_outer
{
    uint32_t id;
    deep_middle middle;
};

// 容器元素为结构体
struct item
{
    uint32_t key;
    std::string value;
};

struct with_vector_struct
{
    uint32_t id;
    std::vector<item> items;
};

// 容器元素为结构体，且结构体内还嵌套结构体
struct attribute
{
    std::string name;
    std::string value;
};

struct complex_item
{
    uint32_t id;
    std::vector<attribute> attrs;
    std::string desc;
};

struct with_nested_vector_struct
{
    uint32_t id;
    std::vector<complex_item> complex_items;
};

// 空嵌套结构体（边界情况）
struct empty_nested
{};

struct with_empty_nested
{
    uint32_t id;
    empty_nested empty;
};

// 多层 vector 嵌套
struct tag
{
    std::string key;
    std::string val;
};

struct group
{
    std::string name;
    std::vector<tag> tags;
};

struct with_multi_vector
{
    uint32_t id;
    std::vector<group> groups;
};

// ============================================================================
// 基础测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(basic_serialization)

BOOST_AUTO_TEST_CASE(empty_struct_roundtrip)
{
    empty_struct orig{};
    auto buf = ssz::serialize(orig);
    BOOST_TEST(buf.data.size() == sizeof(uint32_t) + 1); // 类型哈希 + 字节序标志

    empty_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
}

BOOST_AUTO_TEST_CASE(simple_integrals_roundtrip)
{
    simple_integrals orig{
        0x12,                       // uint8_t
        0x3456,                     // uint16_t
        0x789ABCDE,                 // uint32_t
        0xFEDCBA9876543210ULL,      // uint64_t
        -0x11,                      // int8_t
        -0x2233,                    // int16_t
        -0x44556678,                // int32_t
        -0x1122334455667788LL       // int64_t
    };

    auto buf = ssz::serialize(orig);
    // 4字节哈希 + 1字节字节序标志 + 每个字段按 sizeof 计算
    size_t expected_size = sizeof(uint32_t) + 1
        + sizeof(orig.a) + sizeof(orig.b) + sizeof(orig.c) + sizeof(orig.d)
        + sizeof(orig.e) + sizeof(orig.f) + sizeof(orig.g) + sizeof(orig.h);
    BOOST_TEST(buf.data.size() == expected_size);

    simple_integrals result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
    BOOST_TEST(result.e == orig.e);
    BOOST_TEST(result.f == orig.f);
    BOOST_TEST(result.g == orig.g);
    BOOST_TEST(result.h == orig.h);
}

BOOST_AUTO_TEST_CASE(with_string_roundtrip)
{
    with_string orig{
        42,
        "Hello, SSZ!",
        "你好，世界！"     // UTF-8 中文
    };

    auto buf = ssz::serialize(orig);
    // 4字节哈希 + 1字节字节序标志 + id(4) + name_len(4) + name_data + desc_len(4) + desc_data
    size_t expected_size = sizeof(uint32_t) + 1
        + sizeof(uint32_t)
        + sizeof(uint32_t) + orig.name.size()
        + sizeof(uint32_t) + orig.desc.size();
    BOOST_TEST(buf.data.size() == expected_size);

    with_string result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.name == orig.name);
    BOOST_TEST(result.desc == orig.desc);
}

BOOST_AUTO_TEST_CASE(empty_string_roundtrip)
{
    with_string orig{0, "", ""};

    auto buf = ssz::serialize(orig);

    with_string result{999, "should_be_cleared", "should_be_cleared"};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.name.empty());
    BOOST_TEST(result.desc.empty());
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 容器序列化测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(container_serialization)

BOOST_AUTO_TEST_CASE(vector_integral_roundtrip)
{
    with_vector_integral orig{
        100,
        {95, 87, 92, 88, 76},
        {1, -2, 3, -4, 5, -6}
    };

    auto buf = ssz::serialize(orig);

    with_vector_integral result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.scores.size() == orig.scores.size());
    BOOST_TEST(result.flags.size() == orig.flags.size());
    for (size_t i = 0; i < orig.scores.size(); i++)
        BOOST_TEST(result.scores[i] == orig.scores[i]);
    for (size_t i = 0; i < orig.flags.size(); i++)
        BOOST_TEST(result.flags[i] == orig.flags[i]);
}

BOOST_AUTO_TEST_CASE(empty_vector_roundtrip)
{
    with_vector_integral orig{0, {}, {}};

    auto buf = ssz::serialize(orig);

    with_vector_integral result{999, {1, 2, 3}, {4, 5, 6}};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.scores.empty());
    BOOST_TEST(result.flags.empty());
}

BOOST_AUTO_TEST_CASE(vector_float_roundtrip)
{
    with_vector_float orig{
        1,
        {3.14f, 2.718f, 1.618f, 0.577f}
    };

    auto buf = ssz::serialize(orig);

    with_vector_float result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.values.size() == orig.values.size());
    for (size_t i = 0; i < orig.values.size(); i++)
        BOOST_TEST(result.values[i] == orig.values[i]);
}

BOOST_AUTO_TEST_CASE(complex_mixed_roundtrip)
{
    with_vector_complex orig{
        7,
        {0x00, 0x01, 0x02, 0xFE, 0xFF},
        "test-data",
        {1000000, 2000000, 3000000}
    };

    auto buf = ssz::serialize(orig);

    with_vector_complex result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.data.size() == orig.data.size());
    BOOST_TEST(result.name == orig.name);
    BOOST_TEST(result.amounts.size() == orig.amounts.size());
    for (size_t i = 0; i < orig.data.size(); i++)
        BOOST_TEST(result.data[i] == orig.data[i]);
    for (size_t i = 0; i < orig.amounts.size(); i++)
        BOOST_TEST(result.amounts[i] == orig.amounts[i]);
}

BOOST_AUTO_TEST_CASE(mixed_types_roundtrip)
{
    mixed_types orig{
        0xDEAD,
        "mixed-type-test",
        {10, 20, 30, 40, 50},
        3.14159f,
        0x1234567890ABCDEFULL,
        {-100, -200, -300}
    };

    auto buf = ssz::serialize(orig);

    mixed_types result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.label == orig.label);
    BOOST_TEST(result.tags.size() == orig.tags.size());
    for (size_t i = 0; i < orig.tags.size(); i++)
        BOOST_TEST(result.tags[i] == orig.tags[i]);
    BOOST_TEST(result.ratio == orig.ratio);
    BOOST_TEST(result.timestamp == orig.timestamp);
    BOOST_TEST(result.offsets.size() == orig.offsets.size());
    for (size_t i = 0; i < orig.offsets.size(); i++)
        BOOST_TEST(result.offsets[i] == orig.offsets[i]);
}

BOOST_AUTO_TEST_CASE(large_data_roundtrip)
{
    large_data orig{};
    orig.index = 0xFF;
    orig.blob.resize(1024 * 10); // 10KB blob
    for (size_t i = 0; i < orig.blob.size(); i++)
        orig.blob[i] = static_cast<uint8_t>(i & 0xFF);
    orig.payload = std::string(1024, 'A'); // 1KB string

    auto buf = ssz::serialize(orig);

    large_data result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.index == orig.index);
    BOOST_TEST(result.blob.size() == orig.blob.size());
    BOOST_TEST(result.payload == orig.payload);
    for (size_t i = 0; i < orig.blob.size(); i++)
        BOOST_TEST(result.blob[i] == orig.blob[i]);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 类型哈希验证测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(type_hash_checks)

BOOST_AUTO_TEST_CASE(type_hash_mismatch_detected)
{
    simple_integrals orig{};
    auto buf = ssz::serialize(orig);

    // 使用不同的类型反序列化
    wrong_integrals result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(!ok); // 应该返回 false
}

BOOST_AUTO_TEST_CASE(type_hash_consistency)
{
    // 同一结构体多次序列化应得到相同的哈希
    simple_integrals a{};
    simple_integrals b{};
    auto buf_a = ssz::serialize(a);
    auto buf_b = ssz::serialize(b);

    // 前4字节是类型哈希
    BOOST_TEST(buf_a.data.size() >= sizeof(uint32_t));
    BOOST_TEST(buf_b.data.size() >= sizeof(uint32_t));
    bool hash_match = true;
    for (size_t i = 0; i < sizeof(uint32_t); i++)
        hash_match = hash_match && (buf_a.data[i] == buf_b.data[i]);
    BOOST_TEST(hash_match);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// ssz::buffer 功能测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(buffer_functionality)

BOOST_AUTO_TEST_CASE(advance_read_behavior)
{
    simple_integrals orig{};
    auto buf = ssz::serialize(orig);

    size_t initial_offset = buf.offset;

    // 读取一个字节后更新偏移
    buf.advance_read(4);
    BOOST_TEST(buf.offset == initial_offset + 4);

    // get_read_iterator 应反映当前偏移
    auto it = buf.get_read_iterator();
    auto start = buf.data.data() + buf.offset;
    BOOST_TEST(it == start);
}

BOOST_AUTO_TEST_CASE(multiple_serialize_independence)
{
    // 多次序列化应产生独立的 buffer
    simple_integrals a{1, 2, 3, 4, 5, 6, 7, 8};
    simple_integrals b{9, 10, 11, 12, 13, 14, 15, 16};

    auto buf_a = ssz::serialize(a);
    auto buf_b = ssz::serialize(b);

    // 两个 buffer 应包含不同的数据（不同值序列化结果不同）
    BOOST_TEST(buf_a.data.size() > sizeof(uint32_t) + 1);
    BOOST_TEST(buf_a.data.size() == buf_b.data.size());
    // 验证数据内容不同（第一个字段不同）
    BOOST_TEST(buf_a.data[sizeof(uint32_t) + 1] != buf_b.data[sizeof(uint32_t) + 1]);

    // 反序列化应得到各自的值
    simple_integrals ra{};
    simple_integrals rb{};
    BOOST_TEST(ssz::deserialize(buf_a, ra));
    BOOST_TEST(ssz::deserialize(buf_b, rb));
    BOOST_TEST(ra.a == a.a);
    BOOST_TEST(rb.a == b.a);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 嵌套结构体序列化测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(nested_struct_serialization)

BOOST_AUTO_TEST_CASE(simple_nested_struct_roundtrip)
{
    with_nested_struct orig{
        42,
        {100, 0x1234567890ABCDEFULL, "inner-label"},
        "outer-name"
    };

    auto buf = ssz::serialize(orig);

    with_nested_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.inner.x == orig.inner.x);
    BOOST_TEST(result.inner.y == orig.inner.y);
    BOOST_TEST(result.inner.label == orig.inner.label);
    BOOST_TEST(result.name == orig.name);
}

BOOST_AUTO_TEST_CASE(nested_struct_empty_string)
{
    with_nested_struct orig{
        0,
        {1, 2, ""},
        ""
    };

    auto buf = ssz::serialize(orig);

    with_nested_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.inner.x == orig.inner.x);
    BOOST_TEST(result.inner.y == orig.inner.y);
    BOOST_TEST(result.inner.label.empty());
    BOOST_TEST(result.name.empty());
}

BOOST_AUTO_TEST_CASE(deep_nested_struct_roundtrip)
{
    deep_outer orig{
        7,
        {{1, "deep-inner"}, {0xAA, 0xBB, 0xCC}}
    };

    auto buf = ssz::serialize(orig);

    deep_outer result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.middle.inner.a == orig.middle.inner.a);
    BOOST_TEST(result.middle.inner.b == orig.middle.inner.b);
    BOOST_TEST(result.middle.data.size() == orig.middle.data.size());
    for (size_t i = 0; i < orig.middle.data.size(); i++)
        BOOST_TEST(result.middle.data[i] == orig.middle.data[i]);
}

BOOST_AUTO_TEST_CASE(empty_nested_struct_roundtrip)
{
    with_empty_nested orig{
        99,
        {}
    };

    auto buf = ssz::serialize(orig);

    with_empty_nested result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 容器内嵌结构体序列化测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(vector_struct_serialization)

BOOST_AUTO_TEST_CASE(vector_of_structs_roundtrip)
{
    with_vector_struct orig{
        100,
        {
            {1, "first"},
            {2, "second"},
            {3, "third"}
        }
    };

    auto buf = ssz::serialize(orig);

    with_vector_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.items.size() == orig.items.size());
    for (size_t i = 0; i < orig.items.size(); i++)
    {
        BOOST_TEST(result.items[i].key == orig.items[i].key);
        BOOST_TEST(result.items[i].value == orig.items[i].value);
    }
}

BOOST_AUTO_TEST_CASE(empty_vector_of_structs)
{
    with_vector_struct orig{
        0,
        {}
    };

    auto buf = ssz::serialize(orig);

    with_vector_struct result{42, {{99, "should-be-cleared"}}};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.items.empty());
}

BOOST_AUTO_TEST_CASE(single_element_vector_struct)
{
    with_vector_struct orig{
        1,
        {{42, "single"}}
    };

    auto buf = ssz::serialize(orig);

    with_vector_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.items.size() == 1);
    BOOST_TEST(result.items[0].key == orig.items[0].key);
    BOOST_TEST(result.items[0].value == orig.items[0].value);
}

BOOST_AUTO_TEST_CASE(vector_of_structs_nested_attrs)
{
    with_nested_vector_struct orig{
        7,
        {
            {1, {{"color", "red"}, {"size", "large"}}, "item1-desc"},
            {2, {{"weight", "10kg"}}, "item2-desc"}
        }
    };

    auto buf = ssz::serialize(orig);

    with_nested_vector_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.complex_items.size() == orig.complex_items.size());
    for (size_t i = 0; i < orig.complex_items.size(); i++)
    {
        BOOST_TEST(result.complex_items[i].id == orig.complex_items[i].id);
        BOOST_TEST(result.complex_items[i].desc == orig.complex_items[i].desc);
        BOOST_TEST(result.complex_items[i].attrs.size() == orig.complex_items[i].attrs.size());
        for (size_t j = 0; j < orig.complex_items[i].attrs.size(); j++)
        {
            BOOST_TEST(result.complex_items[i].attrs[j].name == orig.complex_items[i].attrs[j].name);
            BOOST_TEST(result.complex_items[i].attrs[j].value == orig.complex_items[i].attrs[j].value);
        }
    }
}

BOOST_AUTO_TEST_CASE(multi_level_vector_struct)
{
    with_multi_vector orig{
        1,
        {
            {"group1", {{"g1k1", "g1v1"}, {"g1k2", "g1v2"}}},
            {"group2", {{"g2k1", "g2v1"}}},
            {"group3", {}}
        }
    };

    auto buf = ssz::serialize(orig);

    with_multi_vector result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.groups.size() == orig.groups.size());
    for (size_t i = 0; i < orig.groups.size(); i++)
    {
        BOOST_TEST(result.groups[i].name == orig.groups[i].name);
        BOOST_TEST(result.groups[i].tags.size() == orig.groups[i].tags.size());
        for (size_t j = 0; j < orig.groups[i].tags.size(); j++)
        {
            BOOST_TEST(result.groups[i].tags[j].key == orig.groups[i].tags[j].key);
            BOOST_TEST(result.groups[i].tags[j].val == orig.groups[i].tags[j].val);
        }
    }
}

BOOST_AUTO_TEST_CASE(vector_struct_type_hash_mismatch)
{
    // 使用 with_vector_struct 序列化，用 with_nested_vector_struct 反序列化应失败
    with_vector_struct orig{1, {{1, "a"}}};
    auto buf = ssz::serialize(orig);

    with_nested_vector_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(!ok);
}

BOOST_AUTO_TEST_CASE(inner_struct_type_hash_mismatch)
{
    // 使用 with_nested_struct 序列化，使用不同的嵌套结构体反序列化应失败
    // item 和 inner_struct 布局不同
    with_nested_struct orig{
        1,
        {10, 20, "test"},
        "outer"
    };
    auto buf = ssz::serialize(orig);

    // 尝试用 with_vector_struct 反序列化（字段类型不同）
    with_vector_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(!ok);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// char 类型序列化测试
// ============================================================================

struct with_char_types
{
    uint32_t id;
    char a;
    signed char b;
    unsigned char c;
    wchar_t d;
};

struct with_char_and_string
{
    char initial;
    std::string name;
    unsigned char code;
};

BOOST_AUTO_TEST_SUITE(char_types_serialization)

BOOST_AUTO_TEST_CASE(char_types_roundtrip)
{
    with_char_types orig{
        42,
        'A',
        -1,
        200,
        L'世'
    };

    auto buf = ssz::serialize(orig);
    size_t expected_size = sizeof(uint32_t) + 1  // type hash + 字节序标志
        + sizeof(uint32_t)  // id
        + sizeof(char)      // a
        + sizeof(signed char) // b
        + sizeof(unsigned char) // c
        + sizeof(wchar_t);  // d
    BOOST_TEST(buf.data.size() == expected_size);

    with_char_types result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
}

BOOST_AUTO_TEST_CASE(char_and_string_roundtrip)
{
    with_char_and_string orig{
        'Q',
        "test-with-char",
        0xAB
    };

    auto buf = ssz::serialize(orig);

    with_char_and_string result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.initial == orig.initial);
    BOOST_TEST(result.name == orig.name);
    BOOST_TEST(result.code == orig.code);
}

BOOST_AUTO_TEST_CASE(char_extreme_values)
{
    with_char_types orig{
        0,
        static_cast<char>(0x7F),
        static_cast<signed char>(-128),
        static_cast<unsigned char>(255),
        static_cast<wchar_t>(0x10FFFF)
    };

    auto buf = ssz::serialize(orig);

    with_char_types result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// std::array 序列化测试
// ============================================================================

struct with_array_integral
{
    uint32_t id;
    std::array<uint8_t, 4> bytes;
    std::array<uint32_t, 3> integers;
    std::array<uint64_t, 2> longs;
};

struct with_array_float
{
    uint32_t id;
    std::array<float, 3> floats;
    std::array<double, 2> doubles;
};

struct with_array_char
{
    uint32_t id;
    std::array<char, 16> label;
    std::array<unsigned char, 8> raw;
};

struct with_2d_array
{
    uint32_t id;
    std::array<std::array<int, 3>, 2> matrix;
};

struct point_2d
{
    int32_t x;
    int32_t y;
};

struct with_array_struct
{
    uint32_t id;
    std::array<point_2d, 3> points;
};

struct with_array_and_vector
{
    uint32_t id;
    std::array<int, 4> fixed;
    std::vector<int> dynamic;
};

BOOST_AUTO_TEST_SUITE(std_array_serialization)

BOOST_AUTO_TEST_CASE(array_integral_roundtrip)
{
    with_array_integral orig{};
    orig.id = 1;
    orig.bytes = {{0x12, 0x34, 0x56, 0x78}};
    orig.integers = {{0xDEAD, 0xBEAF, 0xCAFE}};
    orig.longs = {{0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL}};

    auto buf = ssz::serialize(orig);
    size_t expected_size = sizeof(uint32_t) + 1  // type hash + 字节序标志
        + sizeof(uint32_t)                                       // id
        + sizeof(uint8_t) * 4                                    // bytes[4]
        + sizeof(uint32_t) * 3                                   // integers[3]
        + sizeof(uint64_t) * 2;                                  // longs[2]
    BOOST_TEST(buf.data.size() == expected_size);

    with_array_integral result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.bytes == orig.bytes);
    BOOST_TEST(result.integers == orig.integers);
    BOOST_TEST(result.longs == orig.longs);
}

BOOST_AUTO_TEST_CASE(array_float_roundtrip)
{
    with_array_float orig{};
    orig.id = 2;
    orig.floats = {{3.14f, 2.718f, 1.618f}};
    orig.doubles = {{3.14159265358979, 2.71828182845904}};

    auto buf = ssz::serialize(orig);

    with_array_float result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    for (size_t i = 0; i < orig.floats.size(); i++)
        BOOST_TEST(result.floats[i] == orig.floats[i]);
    for (size_t i = 0; i < orig.doubles.size(); i++)
        BOOST_TEST(result.doubles[i] == orig.doubles[i]);
}

BOOST_AUTO_TEST_CASE(array_char_roundtrip)
{
    with_array_char orig{};
    orig.id = 3;
    std::strncpy(orig.label.data(), "array-label-123", orig.label.size() - 1);
    orig.label[orig.label.size() - 1] = '\0';
    orig.raw = {{0x00, 0xFF, 0xAA, 0x55, 0x01, 0x80, 0x7F, 0xFE}};

    auto buf = ssz::serialize(orig);

    with_array_char result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.label == orig.label);
    BOOST_TEST(result.raw == orig.raw);
}

BOOST_AUTO_TEST_CASE(array_2d_roundtrip)
{
    with_2d_array orig{};
    orig.id = 4;
    orig.matrix = {{ {{1, 2, 3}}, {{4, 5, 6}} }};

    auto buf = ssz::serialize(orig);

    with_2d_array result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    for (size_t i = 0; i < orig.matrix.size(); i++)
        for (size_t j = 0; j < orig.matrix[i].size(); j++)
            BOOST_TEST(result.matrix[i][j] == orig.matrix[i][j]);
}

BOOST_AUTO_TEST_CASE(array_of_struct_roundtrip)
{
    with_array_struct orig{};
    orig.id = 5;
    orig.points = {{ {10, 20}, {30, -40}, {50, 60} }};

    auto buf = ssz::serialize(orig);

    with_array_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    for (size_t i = 0; i < orig.points.size(); i++)
    {
        BOOST_TEST(result.points[i].x == orig.points[i].x);
        BOOST_TEST(result.points[i].y == orig.points[i].y);
    }
}

BOOST_AUTO_TEST_CASE(array_and_vector_mixed_roundtrip)
{
    with_array_and_vector orig{};
    orig.id = 6;
    orig.fixed = {{10, 20, 30, 40}};
    orig.dynamic = {100, 200, 300};

    auto buf = ssz::serialize(orig);

    with_array_and_vector result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.fixed == orig.fixed);
    BOOST_TEST(result.dynamic.size() == orig.dynamic.size());
    for (size_t i = 0; i < orig.dynamic.size(); i++)
        BOOST_TEST(result.dynamic[i] == orig.dynamic[i]);
}

BOOST_AUTO_TEST_CASE(array_default_values)
{
    with_array_integral orig{};
    // 默认构造的 std::array 是值初始化的（对于整数是 0）
    auto buf = ssz::serialize(orig);

    with_array_integral result{};
    result.id = 999;
    result.bytes = {{1, 2, 3, 4}};
    result.integers = {{5, 6, 7}};
    result.longs = {{8, 9}};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == 0);
    for (auto v : result.bytes)
        BOOST_TEST(v == 0);
    for (auto v : result.integers)
        BOOST_TEST(v == 0);
    for (auto v : result.longs)
        BOOST_TEST(v == 0);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// bool 类型序列化测试
// ============================================================================

struct with_bool
{
    uint32_t id;
    bool flag_a;
    bool flag_b;
    bool flag_c;
};

struct with_bool_mixed
{
    uint32_t id;
    bool active;
    std::string label;
    bool enabled;
    std::vector<int32_t> data;
};

BOOST_AUTO_TEST_SUITE(bool_type_serialization)

BOOST_AUTO_TEST_CASE(bool_true_false_roundtrip)
{
    with_bool orig{1, true, false, true};

    auto buf = ssz::serialize(orig);
    size_t expected_size = sizeof(uint32_t) + 1  // type hash + 字节序标志
        + sizeof(uint32_t)  // id
        + sizeof(bool) * 3; // flag_a, flag_b, flag_c
    BOOST_TEST(buf.data.size() == expected_size);

    with_bool result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.flag_a == true);
    BOOST_TEST(result.flag_b == false);
    BOOST_TEST(result.flag_c == true);
}

BOOST_AUTO_TEST_CASE(bool_all_false)
{
    with_bool orig{0, false, false, false};

    auto buf = ssz::serialize(orig);

    with_bool result{999, true, true, true};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == 0);
    BOOST_TEST(result.flag_a == false);
    BOOST_TEST(result.flag_b == false);
    BOOST_TEST(result.flag_c == false);
}

BOOST_AUTO_TEST_CASE(bool_all_true)
{
    with_bool orig{42, true, true, true};

    auto buf = ssz::serialize(orig);

    with_bool result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == 42);
    BOOST_TEST(result.flag_a == true);
    BOOST_TEST(result.flag_b == true);
    BOOST_TEST(result.flag_c == true);
}

BOOST_AUTO_TEST_CASE(bool_mixed_with_other_types)
{
    with_bool_mixed orig{};
    orig.id = 7;
    orig.active = true;
    orig.label = "bool-test";
    orig.enabled = false;
    orig.data = {1, -2, 3};

    auto buf = ssz::serialize(orig);

    with_bool_mixed result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.active == true);
    BOOST_TEST(result.label == orig.label);
    BOOST_TEST(result.enabled == false);
    BOOST_TEST(result.data.size() == orig.data.size());
    for (size_t i = 0; i < orig.data.size(); i++)
        BOOST_TEST(result.data[i] == orig.data[i]);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 更多整数类型序列化测试 (short / unsigned short / long / unsigned long / long long / unsigned long long)
// ============================================================================

struct with_integer_variations
{
    uint32_t id;
    short s;
    unsigned short us;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
};

BOOST_AUTO_TEST_SUITE(integer_variation_serialization)

BOOST_AUTO_TEST_CASE(integer_variations_roundtrip)
{
    with_integer_variations orig{};
    orig.id = 1;
    orig.s = -32768;
    orig.us = 65535;
    orig.l = -123456789L;
    orig.ul = 987654321UL;
    orig.ll = -1234567890123456789LL;
    orig.ull = 9876543210987654321ULL;

    auto buf = ssz::serialize(orig);
    size_t expected_size = sizeof(uint32_t) + 1  // type hash + 字节序标志
        + sizeof(uint32_t)       // id
        + sizeof(short)          // s
        + sizeof(unsigned short) // us
        + sizeof(long)           // l
        + sizeof(unsigned long)  // ul
        + sizeof(long long)      // ll
        + sizeof(unsigned long long); // ull
    BOOST_TEST(buf.data.size() == expected_size);

    with_integer_variations result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.s == orig.s);
    BOOST_TEST(result.us == orig.us);
    BOOST_TEST(result.l == orig.l);
    BOOST_TEST(result.ul == orig.ul);
    BOOST_TEST(result.ll == orig.ll);
    BOOST_TEST(result.ull == orig.ull);
}

BOOST_AUTO_TEST_CASE(integer_variations_zero)
{
    with_integer_variations orig{};
    orig.id = 0;
    orig.s = 0;
    orig.us = 0;
    orig.l = 0;
    orig.ul = 0;
    orig.ll = 0;
    orig.ull = 0;

    auto buf = ssz::serialize(orig);

    with_integer_variations result{};
    result.id = 999;
    result.s = 1; result.us = 2; result.l = 3; result.ul = 4;
    result.ll = 5; result.ull = 6;
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == 0);
    BOOST_TEST(result.s == 0);
    BOOST_TEST(result.us == 0);
    BOOST_TEST(result.l == 0);
    BOOST_TEST(result.ul == 0);
    BOOST_TEST(result.ll == 0);
    BOOST_TEST(result.ull == 0);
}

BOOST_AUTO_TEST_CASE(integer_variations_extreme)
{
    with_integer_variations orig{};
    orig.id = 0xFF;
    orig.s = 32767;               // SHRT_MAX
    orig.us = 0;                  // 0
    orig.l = 2147483647L;         // LONG_MAX
    orig.ul = 0UL;               // 0
    orig.ll = 9223372036854775807LL;  // LLONG_MAX
    orig.ull = 0ULL;             // 0

    auto buf = ssz::serialize(orig);

    with_integer_variations result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.s == orig.s);
    BOOST_TEST(result.us == orig.us);
    BOOST_TEST(result.l == orig.l);
    BOOST_TEST(result.ul == orig.ul);
    BOOST_TEST(result.ll == orig.ll);
    BOOST_TEST(result.ull == orig.ull);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 大端序字节序验证测试
// ============================================================================

struct with_fixed_hex
{
    uint16_t a;
    uint32_t b;
    uint64_t c;
};

BOOST_AUTO_TEST_SUITE(endian_validation)

BOOST_AUTO_TEST_CASE(big_endian_encoding)
{
    with_fixed_hex orig{};
    orig.a = 0x0102;
    orig.b = 0x03040506;
    orig.c = 0x0708090A0B0C0D0EULL;

    auto buf = ssz::serialize(orig);

    // 验证大端序编码：高位字节在前
    // 跳过类型哈希 (4字节) 和字节序标志 (1字节)
    // a (uint16_t): 0x01 0x02
    BOOST_TEST(buf.data[5] == static_cast<char>(0x01));
    BOOST_TEST(buf.data[6] == static_cast<char>(0x02));
    // b (uint32_t): 0x03 0x04 0x05 0x06
    BOOST_TEST(buf.data[7] == static_cast<char>(0x03));
    BOOST_TEST(buf.data[8] == static_cast<char>(0x04));
    BOOST_TEST(buf.data[9] == static_cast<char>(0x05));
    BOOST_TEST(buf.data[10] == static_cast<char>(0x06));
    // c (uint64_t): 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E
    BOOST_TEST(buf.data[11] == static_cast<char>(0x07));
    BOOST_TEST(buf.data[12] == static_cast<char>(0x08));
    BOOST_TEST(buf.data[13] == static_cast<char>(0x09));
    BOOST_TEST(buf.data[14] == static_cast<char>(0x0A));
    BOOST_TEST(buf.data[15] == static_cast<char>(0x0B));
    BOOST_TEST(buf.data[16] == static_cast<char>(0x0C));
    BOOST_TEST(buf.data[17] == static_cast<char>(0x0D));
    BOOST_TEST(buf.data[18] == static_cast<char>(0x0E));

    with_fixed_hex result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
}

BOOST_AUTO_TEST_CASE(little_endian_flag_byte)
{
    // 验证 little_endian=true 时字节序标志为 1
    with_fixed_hex orig{};
    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    // 默认 (big-endian) 时字节序标志为 0
    auto buf_default = ssz::serialize(orig, false);
    BOOST_TEST(buf_default.data[0] == static_cast<char>(0));

    // 默认参数应为大端序 (标志为 0)
    auto buf_default2 = ssz::serialize(orig);
    BOOST_TEST(buf_default2.data[0] == static_cast<char>(0));
}

BOOST_AUTO_TEST_CASE(little_endian_encoding)
{
    // 验证小端序编码：低位字节在前
    with_fixed_hex orig{};
    orig.a = 0x0102;
    orig.b = 0x03040506;
    orig.c = 0x0708090A0B0C0D0EULL;

    auto buf = ssz::serialize(orig, true);

    // 跳过类型哈希 (4字节) 和字节序标志 (1字节)
    // a (uint16_t) 小端: 0x02 0x01
    BOOST_TEST(buf.data[5] == static_cast<char>(0x02));
    BOOST_TEST(buf.data[6] == static_cast<char>(0x01));
    // b (uint32_t) 小端: 0x06 0x05 0x04 0x03
    BOOST_TEST(buf.data[7] == static_cast<char>(0x06));
    BOOST_TEST(buf.data[8] == static_cast<char>(0x05));
    BOOST_TEST(buf.data[9] == static_cast<char>(0x04));
    BOOST_TEST(buf.data[10] == static_cast<char>(0x03));
    // c (uint64_t) 小端: 0x0E 0x0D 0x0C 0x0B 0x0A 0x09 0x08 0x07
    BOOST_TEST(buf.data[11] == static_cast<char>(0x0E));
    BOOST_TEST(buf.data[12] == static_cast<char>(0x0D));
    BOOST_TEST(buf.data[13] == static_cast<char>(0x0C));
    BOOST_TEST(buf.data[14] == static_cast<char>(0x0B));
    BOOST_TEST(buf.data[15] == static_cast<char>(0x0A));
    BOOST_TEST(buf.data[16] == static_cast<char>(0x09));
    BOOST_TEST(buf.data[17] == static_cast<char>(0x08));
    BOOST_TEST(buf.data[18] == static_cast<char>(0x07));

    // 验证反序列化正确
    with_fixed_hex result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
}

BOOST_AUTO_TEST_CASE(little_endian_roundtrip)
{
    // 小端序完整往返测试：覆盖各种整数类型
    simple_integrals orig{
        0x12,                       // uint8_t
        0x3456,                     // uint16_t
        0x789ABCDE,                 // uint32_t
        0xFEDCBA9876543210ULL,      // uint64_t
        -0x11,                      // int8_t
        -0x2233,                    // int16_t
        -0x44556678,                // int32_t
        -0x1122334455667788LL       // int64_t
    };

    auto buf = ssz::serialize(orig, true);
    // 4字节哈希 + 1字节字节序标志 + 各字段
    size_t expected_size = sizeof(uint32_t) + 1
        + sizeof(orig.a) + sizeof(orig.b) + sizeof(orig.c) + sizeof(orig.d)
        + sizeof(orig.e) + sizeof(orig.f) + sizeof(orig.g) + sizeof(orig.h);
    BOOST_TEST(buf.data.size() == expected_size);
    // 字节序标志应为 1
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    simple_integrals result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
    BOOST_TEST(result.e == orig.e);
    BOOST_TEST(result.f == orig.f);
    BOOST_TEST(result.g == orig.g);
    BOOST_TEST(result.h == orig.h);
}

BOOST_AUTO_TEST_CASE(little_endian_with_string)
{
    // 小端序下 string 类型的序列化/反序列化
    with_string orig{
        42,
        "Hello, Little Endian!",
        "小端序测试"
    };

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    with_string result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.name == orig.name);
    BOOST_TEST(result.desc == orig.desc);
}

BOOST_AUTO_TEST_CASE(little_endian_with_vector)
{
    // 小端序下 vector 的序列化/反序列化
    with_vector_integral orig{
        100,
        {95, 87, 92, 88, 76},
        {1, -2, 3, -4, 5, -6}
    };

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    with_vector_integral result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.scores.size() == orig.scores.size());
    BOOST_TEST(result.flags.size() == orig.flags.size());
    for (size_t i = 0; i < orig.scores.size(); i++)
        BOOST_TEST(result.scores[i] == orig.scores[i]);
    for (size_t i = 0; i < orig.flags.size(); i++)
        BOOST_TEST(result.flags[i] == orig.flags[i]);
}

BOOST_AUTO_TEST_CASE(little_endian_nested_struct)
{
    // 小端序下嵌套结构体的序列化/反序列化
    with_nested_struct orig{
        42,
        {100, 0x1234567890ABCDEFULL, "inner-label"},
        "outer-name"
    };

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    with_nested_struct result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.inner.x == orig.inner.x);
    BOOST_TEST(result.inner.y == orig.inner.y);
    BOOST_TEST(result.inner.label == orig.inner.label);
    BOOST_TEST(result.name == orig.name);
}

BOOST_AUTO_TEST_CASE(little_endian_vs_big_endian_order)
{
    // 验证小端序和大端序产生不同的字节序列（对多字节整数）
    with_fixed_hex orig{};
    orig.a = 0x0102;
    orig.b = 0x03040506;
    orig.c = 0x0708090A0B0C0D0EULL;

    auto buf_big = ssz::serialize(orig, false);   // 大端序
    auto buf_little = ssz::serialize(orig, true);  // 小端序

    // 类型哈希 (第1-4字节) 字节逆序
    for (size_t i = 0; i < sizeof(uint32_t); i++)
        BOOST_TEST(buf_big.data[1 + i] == buf_little.data[1 + sizeof(uint32_t) - 1 - i]);

    // 字节序标志不同
    BOOST_TEST(buf_big.data[0] == static_cast<char>(0));
    BOOST_TEST(buf_little.data[0] == static_cast<char>(1));

    // 数据部分应当按字节逆序
    for (size_t i = 0; i < sizeof(orig.a); i++)
        BOOST_TEST(buf_big.data[5 + i] == buf_little.data[5 + sizeof(orig.a) - 1 - i]);
    for (size_t i = 0; i < sizeof(orig.b); i++)
        BOOST_TEST(buf_big.data[7 + i] == buf_little.data[7 + sizeof(orig.b) - 1 - i]);

    // 两种端序都应反序列化正确
    with_fixed_hex result_big{};
    with_fixed_hex result_little{};
    BOOST_TEST(ssz::deserialize(buf_big, result_big));
    BOOST_TEST(ssz::deserialize(buf_little, result_little));
    BOOST_TEST(result_big.a == orig.a);
    BOOST_TEST(result_big.b == orig.b);
    BOOST_TEST(result_big.c == orig.c);
    BOOST_TEST(result_little.a == orig.a);
    BOOST_TEST(result_little.b == orig.b);
    BOOST_TEST(result_little.c == orig.c);
}

BOOST_AUTO_TEST_CASE(little_endian_mixed_types)
{
    // 小端序下复杂混合类型的序列化/反序列化
    mixed_types orig{
        0xDEAD,
        "mixed-little-endian-test",
        {10, 20, 30, 40, 50},
        3.14159f,
        0x1234567890ABCDEFULL,
        {-100, -200, -300}
    };

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    mixed_types result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.label == orig.label);
    BOOST_TEST(result.tags.size() == orig.tags.size());
    for (size_t i = 0; i < orig.tags.size(); i++)
        BOOST_TEST(result.tags[i] == orig.tags[i]);
    BOOST_TEST(result.ratio == orig.ratio);
    BOOST_TEST(result.timestamp == orig.timestamp);
    BOOST_TEST(result.offsets.size() == orig.offsets.size());
    for (size_t i = 0; i < orig.offsets.size(); i++)
        BOOST_TEST(result.offsets[i] == orig.offsets[i]);
}

BOOST_AUTO_TEST_CASE(little_endian_large_data)
{
    // 小端序下大数据量的序列化/反序列化
    large_data orig{};
    orig.index = 0xFF;
    orig.blob.resize(1024 * 10); // 10KB blob
    for (size_t i = 0; i < orig.blob.size(); i++)
        orig.blob[i] = static_cast<uint8_t>(i & 0xFF);
    orig.payload = std::string(1024, 'A'); // 1KB string

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    large_data result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.index == orig.index);
    BOOST_TEST(result.blob.size() == orig.blob.size());
    BOOST_TEST(result.payload == orig.payload);
    for (size_t i = 0; i < orig.blob.size(); i++)
        BOOST_TEST(result.blob[i] == orig.blob[i]);
}

BOOST_AUTO_TEST_CASE(little_endian_bool)
{
    // 小端序下 bool 类型的序列化/反序列化
    with_bool orig{1, true, false, true};

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    with_bool result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.flag_a == true);
    BOOST_TEST(result.flag_b == false);
    BOOST_TEST(result.flag_c == true);
}

BOOST_AUTO_TEST_CASE(little_endian_integer_variations)
{
    // 小端序下各种整数类型的序列化/反序列化
    with_integer_variations orig{};
    orig.id = 1;
    orig.s = -32768;
    orig.us = 65535;
    orig.l = -123456789L;
    orig.ul = 987654321UL;
    orig.ll = -1234567890123456789LL;
    orig.ull = 9876543210987654321ULL;

    auto buf = ssz::serialize(orig, true);
    BOOST_TEST(buf.data[0] == static_cast<char>(1));

    with_integer_variations result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.s == orig.s);
    BOOST_TEST(result.us == orig.us);
    BOOST_TEST(result.l == orig.l);
    BOOST_TEST(result.ul == orig.ul);
    BOOST_TEST(result.ll == orig.ll);
    BOOST_TEST(result.ull == orig.ull);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// 类型哈希与内存布局无关性测试
// ============================================================================

// 与 simple_integrals 字段类型相同但顺序不同，验证哈希不同
struct reordered_integrals
{
    uint64_t d;
    uint32_t c;
    uint16_t b;
    uint8_t a;
    int64_t h;
    int32_t g;
    int16_t f;
    int8_t e;
};

BOOST_AUTO_TEST_SUITE(type_hash_layout_independence)

BOOST_AUTO_TEST_CASE(field_order_affects_hash)
{
    simple_integrals orig{};
    auto buf_orig = ssz::serialize(orig);

    reordered_integrals result{};
    bool ok = ssz::deserialize(buf_orig, result);
    BOOST_TEST(!ok); // 字段顺序不同，哈希不同，反序列化应失败
}

BOOST_AUTO_TEST_CASE(same_layout_same_hash)
{
    // with_array_integral 和另一个相同布局的结构体应该有不同哈希
    // 因为结构体类型不同
    with_array_integral a{};
    with_array_integral b{};
    auto buf_a = ssz::serialize(a);
    auto buf_b = ssz::serialize(b);

    // 前4字节是类型哈希，同一类型的应相同
    for (size_t i = 0; i < sizeof(uint32_t); i++)
        BOOST_TEST(buf_a.data[i] == buf_b.data[i]);
}

BOOST_AUTO_TEST_SUITE_END()

// ============================================================================
// serialize_without_meta / deserialize_without_meta 测试
// ============================================================================

BOOST_AUTO_TEST_SUITE(without_meta_serialization)

BOOST_AUTO_TEST_CASE(without_meta_no_header)
{
    simple_integrals orig{};
    auto buf = ssz::serialize_without_meta(orig);

    // 不应包含类型哈希 (4字节) 和字节序标志 (1字节)
    size_t expected_size = sizeof(orig.a) + sizeof(orig.b) + sizeof(orig.c) + sizeof(orig.d)
        + sizeof(orig.e) + sizeof(orig.f) + sizeof(orig.g) + sizeof(orig.h);
    BOOST_TEST(buf.data.size() == expected_size);
}

BOOST_AUTO_TEST_CASE(without_meta_roundtrip)
{
    simple_integrals orig{
        0x12, 0x3456, 0x789ABCDE, 0xFEDCBA9876543210ULL,
        -0x11, -0x2233, -0x44556678, -0x1122334455667788LL
    };

    auto buf = ssz::serialize_without_meta(orig);

    simple_integrals result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
    BOOST_TEST(result.e == orig.e);
    BOOST_TEST(result.f == orig.f);
    BOOST_TEST(result.g == orig.g);
    BOOST_TEST(result.h == orig.h);
}

BOOST_AUTO_TEST_CASE(without_meta_string_roundtrip)
{
    with_string orig{42, "Hello without meta!", "无元数据测试"};

    auto buf = ssz::serialize_without_meta(orig);

    with_string result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.name == orig.name);
    BOOST_TEST(result.desc == orig.desc);
}

BOOST_AUTO_TEST_CASE(without_meta_vector_roundtrip)
{
    with_vector_integral orig{
        100,
        {95, 87, 92, 88, 76},
        {1, -2, 3, -4, 5, -6}
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_vector_integral result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.scores.size() == orig.scores.size());
    BOOST_TEST(result.flags.size() == orig.flags.size());
    for (size_t i = 0; i < orig.scores.size(); i++)
        BOOST_TEST(result.scores[i] == orig.scores[i]);
    for (size_t i = 0; i < orig.flags.size(); i++)
        BOOST_TEST(result.flags[i] == orig.flags[i]);
}

BOOST_AUTO_TEST_CASE(without_meta_empty_vector)
{
    with_vector_integral orig{0, {}, {}};

    auto buf = ssz::serialize_without_meta(orig);

    with_vector_integral result{999, {1, 2, 3}, {4, 5, 6}};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.scores.empty());
    BOOST_TEST(result.flags.empty());
}

BOOST_AUTO_TEST_CASE(without_meta_float_roundtrip)
{
    with_vector_float orig{1, {3.14f, 2.718f, 1.618f}};

    auto buf = ssz::serialize_without_meta(orig);

    with_vector_float result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.values.size() == orig.values.size());
    for (size_t i = 0; i < orig.values.size(); i++)
        BOOST_TEST(result.values[i] == orig.values[i]);
}

BOOST_AUTO_TEST_CASE(without_meta_little_endian)
{
    simple_integrals orig{
        0x12, 0x3456, 0x789ABCDE, 0xFEDCBA9876543210ULL,
        -0x11, -0x2233, -0x44556678, -0x1122334455667788LL
    };

    auto buf = ssz::serialize_without_meta(orig, true);
    BOOST_TEST(buf.data[0] != static_cast<char>(0)); // 小端序编码，但无字节序标志

    simple_integrals result{};
    bool ok = ssz::deserialize_without_meta(buf, result, true);
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
    BOOST_TEST(result.e == orig.e);
    BOOST_TEST(result.f == orig.f);
    BOOST_TEST(result.g == orig.g);
    BOOST_TEST(result.h == orig.h);
}

BOOST_AUTO_TEST_CASE(without_meta_nested_struct)
{
    with_nested_struct orig{
        42,
        {100, 0x1234567890ABCDEFULL, "inner-without-meta"},
        "outer"
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_nested_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.inner.x == orig.inner.x);
    BOOST_TEST(result.inner.y == orig.inner.y);
    BOOST_TEST(result.inner.label == orig.inner.label);
    BOOST_TEST(result.name == orig.name);
}

BOOST_AUTO_TEST_CASE(without_meta_deep_nested)
{
    deep_outer orig{
        7,
        {{1, "deep-without-meta"}, {0xAA, 0xBB, 0xCC}}
    };

    auto buf = ssz::serialize_without_meta(orig);

    deep_outer result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.middle.inner.a == orig.middle.inner.a);
    BOOST_TEST(result.middle.inner.b == orig.middle.inner.b);
    BOOST_TEST(result.middle.data.size() == orig.middle.data.size());
    for (size_t i = 0; i < orig.middle.data.size(); i++)
        BOOST_TEST(result.middle.data[i] == orig.middle.data[i]);
}

BOOST_AUTO_TEST_CASE(without_meta_vector_struct)
{
    with_vector_struct orig{
        100,
        {{1, "first"}, {2, "second"}, {3, "third"}}
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_vector_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.items.size() == orig.items.size());
    for (size_t i = 0; i < orig.items.size(); i++)
    {
        BOOST_TEST(result.items[i].key == orig.items[i].key);
        BOOST_TEST(result.items[i].value == orig.items[i].value);
    }
}

BOOST_AUTO_TEST_CASE(without_meta_multi_level_vector)
{
    with_multi_vector orig{
        1,
        {
            {"group1", {{"k1", "v1"}, {"k2", "v2"}}},
            {"group2", {{"k3", "v3"}}}
        }
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_multi_vector result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.groups.size() == orig.groups.size());
    for (size_t i = 0; i < orig.groups.size(); i++)
    {
        BOOST_TEST(result.groups[i].name == orig.groups[i].name);
        BOOST_TEST(result.groups[i].tags.size() == orig.groups[i].tags.size());
        for (size_t j = 0; j < orig.groups[i].tags.size(); j++)
        {
            BOOST_TEST(result.groups[i].tags[j].key == orig.groups[i].tags[j].key);
            BOOST_TEST(result.groups[i].tags[j].val == orig.groups[i].tags[j].val);
        }
    }
}

BOOST_AUTO_TEST_CASE(without_meta_bool_roundtrip)
{
    with_bool orig{1, true, false, true};

    auto buf = ssz::serialize_without_meta(orig);

    with_bool result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.flag_a == true);
    BOOST_TEST(result.flag_b == false);
    BOOST_TEST(result.flag_c == true);
}

BOOST_AUTO_TEST_CASE(without_meta_char_types)
{
    with_char_types orig{
        42, 'A', static_cast<signed char>(-1), static_cast<unsigned char>(200), L'世'
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_char_types result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.a == orig.a);
    BOOST_TEST(result.b == orig.b);
    BOOST_TEST(result.c == orig.c);
    BOOST_TEST(result.d == orig.d);
}

BOOST_AUTO_TEST_CASE(without_meta_array_struct)
{
    with_array_struct orig{};
    orig.id = 5;
    orig.points = {{ {10, 20}, {30, -40}, {50, 60} }};

    auto buf = ssz::serialize_without_meta(orig);

    with_array_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    for (size_t i = 0; i < orig.points.size(); i++)
    {
        BOOST_TEST(result.points[i].x == orig.points[i].x);
        BOOST_TEST(result.points[i].y == orig.points[i].y);
    }
}

BOOST_AUTO_TEST_CASE(without_meta_default_parameter)
{
    // 验证 deserialize_without_meta 的 little_endian 默认参数
    simple_integrals orig{};
    auto buf = ssz::serialize_without_meta(orig); // 默认大端序

    simple_integrals result{};
    bool ok = ssz::deserialize_without_meta(buf, result); // 默认大端序
    BOOST_TEST(ok);
    BOOST_TEST(result.a == orig.a);
}

BOOST_AUTO_TEST_CASE(serialize_without_meta_equals_serialize_fields)
{
    // 验证 serialize_without_meta 和 serialize 的字段数据部分相同
    simple_integrals orig{};
    auto buf_with_meta = ssz::serialize(orig);
    auto buf_without_meta = ssz::serialize_without_meta(orig);

    // 跳过 serialize 的元数据头 (5字节: 1字节标志 + 4字节哈希)
    const size_t header_size = 1 + sizeof(uint32_t);
    BOOST_TEST(buf_with_meta.data.size() == buf_without_meta.data.size() + header_size);

    // 比较字段数据部分是否相同
    for (size_t i = 0; i < buf_without_meta.data.size(); i++)
        BOOST_TEST(buf_with_meta.data[header_size + i] == buf_without_meta.data[i]);
}

BOOST_AUTO_TEST_CASE(deserialize_fails_on_without_meta_data)
{
    // 使用 deserialize 读取 serialize_without_meta 的数据应失败（缺少元数据）
    simple_integrals orig{};
    auto buf = ssz::serialize_without_meta(orig);

    simple_integrals result{};
    bool ok = ssz::deserialize(buf, result);
    BOOST_TEST(!ok);
}

BOOST_AUTO_TEST_CASE(without_meta_roundtrip_with_string_and_vector)
{
    mixed_types orig{
        0xDEAD,
        "without-meta-test",
        {10, 20, 30, 40, 50},
        3.14159f,
        0x1234567890ABCDEFULL,
        {-100, -200, -300}
    };

    auto buf = ssz::serialize_without_meta(orig);

    mixed_types result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.label == orig.label);
    BOOST_TEST(result.tags.size() == orig.tags.size());
    for (size_t i = 0; i < orig.tags.size(); i++)
        BOOST_TEST(result.tags[i] == orig.tags[i]);
    BOOST_TEST(result.ratio == orig.ratio);
    BOOST_TEST(result.timestamp == orig.timestamp);
    BOOST_TEST(result.offsets.size() == orig.offsets.size());
    for (size_t i = 0; i < orig.offsets.size(); i++)
        BOOST_TEST(result.offsets[i] == orig.offsets[i]);
}

BOOST_AUTO_TEST_CASE(without_meta_little_endian_nested_struct)
{
    with_nested_struct orig{
        42,
        {100, 0x1234567890ABCDEFULL, "inner-le"},
        "outer-le"
    };

    auto buf = ssz::serialize_without_meta(orig, true);

    with_nested_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result, true);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.inner.x == orig.inner.x);
    BOOST_TEST(result.inner.y == orig.inner.y);
    BOOST_TEST(result.inner.label == orig.inner.label);
    BOOST_TEST(result.name == orig.name);
}

BOOST_AUTO_TEST_CASE(without_meta_empty_struct)
{
    empty_struct orig{};
    auto buf = ssz::serialize_without_meta(orig);
    BOOST_TEST(buf.data.empty()); // 空结构体序列化后应为空

    empty_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
}

BOOST_AUTO_TEST_CASE(without_meta_nested_vector_struct_attrs)
{
    with_nested_vector_struct orig{
        7,
        {
            {1, {{"color", "red"}, {"size", "large"}}, "desc1"},
            {2, {{"weight", "10kg"}}, "desc2"}
        }
    };

    auto buf = ssz::serialize_without_meta(orig);

    with_nested_vector_struct result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.complex_items.size() == orig.complex_items.size());
    for (size_t i = 0; i < orig.complex_items.size(); i++)
    {
        BOOST_TEST(result.complex_items[i].id == orig.complex_items[i].id);
        BOOST_TEST(result.complex_items[i].desc == orig.complex_items[i].desc);
        BOOST_TEST(result.complex_items[i].attrs.size() == orig.complex_items[i].attrs.size());
        for (size_t j = 0; j < orig.complex_items[i].attrs.size(); j++)
        {
            BOOST_TEST(result.complex_items[i].attrs[j].name == orig.complex_items[i].attrs[j].name);
            BOOST_TEST(result.complex_items[i].attrs[j].value == orig.complex_items[i].attrs[j].value);
        }
    }
}

BOOST_AUTO_TEST_CASE(without_meta_integer_variations)
{
    with_integer_variations orig{};
    orig.id = 1;
    orig.s = -32768;
    orig.us = 65535;
    orig.l = -123456789L;
    orig.ul = 987654321UL;
    orig.ll = -1234567890123456789LL;
    orig.ull = 9876543210987654321ULL;

    auto buf = ssz::serialize_without_meta(orig);

    with_integer_variations result{};
    bool ok = ssz::deserialize_without_meta(buf, result);
    BOOST_TEST(ok);
    BOOST_TEST(result.id == orig.id);
    BOOST_TEST(result.s == orig.s);
    BOOST_TEST(result.us == orig.us);
    BOOST_TEST(result.l == orig.l);
    BOOST_TEST(result.ul == orig.ul);
    BOOST_TEST(result.ll == orig.ll);
    BOOST_TEST(result.ull == orig.ull);
}

BOOST_AUTO_TEST_SUITE_END()
