//
// Copyright (C) 2026 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

#pragma once

#include <boost/pfr.hpp>

#include <vector>
#include <string_view>
#include <type_traits>
#include <utility>
#include <cstring>
#include <cstdint>
#include <concepts>
#include <bit>

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------
namespace ssz {

class buffer
{
public:
    std::vector<char> data;
    size_t offset = 0;

    auto get_write_iterator()
    {
        return std::back_inserter(data);
    }

    auto get_read_iterator()
    {
        return data.data() + offset;
    }

    void advance_read(size_t size)
    {
        offset += size;
    }
};

// Forward declarations for public API.
template <typename T>
buffer serialize(const T& obj, bool little_endian = false);

template <typename T>
bool deserialize(buffer& buf, T& obj);

template <typename T>
buffer serialize_without_meta(const T& obj, bool little_endian = false);

template <typename T>
bool deserialize_without_meta(buffer& buf, T& obj, bool little_endian = false);

} // namespace ssz

// -----------------------------------------------------------------------
// Internal helpers
// -----------------------------------------------------------------------
namespace ssz::detail {

// 检测模板特化的辅助类型 trait
template<typename T, template<typename...> class Primary>
struct is_specialization_of : std::false_type {};

template<template<typename...> class Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};

template<typename T, template<typename...> class Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;

// 判断类型是否为需要递归序列化的 SSZ 结构体
template<typename T>
concept is_ssz_struct = std::is_class_v<T>
    && !std::is_trivially_copyable_v<T>
    && !std::is_same_v<std::decay_t<T>, std::string>
    && !is_specialization_of_v<std::decay_t<T>, std::vector>;

// 辅助 trait: 获取无符号整数类型，bool 特化为 unsigned char
template<typename T>
struct ssz_unsigned { using type = void; };

template<> struct ssz_unsigned<bool> { using type = unsigned char; };
template<> struct ssz_unsigned<char> { using type = unsigned char; };
template<> struct ssz_unsigned<signed char> { using type = unsigned char; };
template<> struct ssz_unsigned<unsigned char> { using type = unsigned char; };
template<> struct ssz_unsigned<short> { using type = unsigned short; };
template<> struct ssz_unsigned<unsigned short> { using type = unsigned short; };
template<> struct ssz_unsigned<int> { using type = unsigned int; };
template<> struct ssz_unsigned<unsigned int> { using type = unsigned int; };
template<> struct ssz_unsigned<long> { using type = unsigned long; };
template<> struct ssz_unsigned<unsigned long> { using type = unsigned long; };
template<> struct ssz_unsigned<long long> { using type = unsigned long long; };
template<> struct ssz_unsigned<unsigned long long> { using type = unsigned long long; };
template<> struct ssz_unsigned<wchar_t> { using type = std::make_unsigned_t<wchar_t>; };
#if defined(__cpp_char8_t)
template<> struct ssz_unsigned<char8_t> { using type = unsigned char; };
#endif
#if defined(__cpp_unicode_characters)
template<> struct ssz_unsigned<char16_t> { using type = std::make_unsigned_t<char16_t>; };
template<> struct ssz_unsigned<char32_t> { using type = std::make_unsigned_t<char32_t>; };
#endif

template<typename T>
using ssz_unsigned_t = typename ssz_unsigned<T>::type;

// 主机字节序检测（C++20 std::endian）
constexpr bool host_is_little_endian = std::endian::native == std::endian::little;

template<std::integral type, typename source>
type ssz_read(source& p, bool little_endian = false)
{
    ssz_unsigned_t<type> ret = 0;
    if (little_endian && host_is_little_endian)
    {
        std::memcpy(&ret, p, sizeof(type));
        p += sizeof(type);
    }
    else if (little_endian)
    {
        for (std::size_t i = 0; i < sizeof(type); i++)
            ret |= (static_cast<ssz_unsigned_t<type>>(static_cast<unsigned char>(*p++)) << (i * 8));
    }
    else
    {
        for (std::size_t i = 0; i < sizeof(type); i++)
            ret = (ret << 8) | (static_cast<unsigned char>(*p++));
    }
    return static_cast<type>(ret);
}

template<std::integral type, typename target>
void ssz_write(type v, target& p, bool little_endian = false)
{
    using uv_type = ssz_unsigned_t<type>;
    if (little_endian && host_is_little_endian)
    {
        const char* src = reinterpret_cast<const char*>(&v);
        for (std::size_t i = 0; i < sizeof(type); i++)
            *p++ = src[i];
    }
    else if (little_endian)
    {
        for (std::size_t i = 0; i < sizeof(type); i++, p++)
            *p = static_cast<unsigned char>((uv_type(v) >> (i * 8)) & 0xff);
    }
    else
    {
        for (auto i = (int)sizeof(type) - 1; i >= 0; i--, p++)
            *p = static_cast<unsigned char>((uv_type(v) >> (i * 8)) & 0xff);
    }
}

template <typename T>
constexpr uint32_t get_type_hash()
{
#if defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
#else
    std::string_view name = __PRETTY_FUNCTION__;
#endif
    uint32_t hash = 2166136261U;
    for (char c : name)
    {
        hash ^= static_cast<uint32_t>(c);
        hash *= 16777619U;
    }
    return hash;
}

// 前向声明，供 type_hash_for 递归引用。
template <typename T>
constexpr uint32_t calculate_struct_hash();

// 获取类型（递归展开嵌套结构体）的哈希值。
template <typename T>
constexpr uint32_t type_hash_for()
{
    if constexpr (is_specialization_of_v<T, std::vector>)
    {
        using value_type = typename T::value_type;
        if constexpr (is_ssz_struct<value_type>)
            return calculate_struct_hash<value_type>();
        else
            return get_type_hash<value_type>();
    }
    else if constexpr (is_ssz_struct<T>)
        return calculate_struct_hash<T>();
    else
        return get_type_hash<T>();
}

// 为平凡可复制类型提供特化的哈希计算（直接使用类型名称哈希，避免遍历字段）。
template <typename T>
constexpr uint32_t calculate_struct_hash_impl(std::true_type)
{
    return get_type_hash<T>();
}

// 为SSZ结构体（非平凡可复制）提供递归字段哈希计算。
template <typename T>
constexpr uint32_t calculate_struct_hash_impl(std::false_type)
{
    uint32_t hash = get_type_hash<T>();

    if constexpr (std::is_default_constructible_v<std::decay_t<T>>)
    {
        using raw_type = std::decay_t<T>;
        constexpr auto field_count = boost::pfr::tuple_size_v<raw_type>;

        [&hash]<std::size_t... I>(std::index_sequence<I...>)
        {
            ((hash ^= get_type_hash<std::integral_constant<std::size_t, I>>()
                  + 0x9e3779b9 + (hash << 6) + (hash >> 2),
              hash ^= type_hash_for<boost::pfr::tuple_element_t<I, raw_type>>()
                  + 0x9e3779b9 + (hash << 6) + (hash >> 2)), ...);
        }(std::make_index_sequence<field_count>{});
    }

    return hash;
}

template <typename T>
constexpr uint32_t calculate_struct_hash()
{
    return calculate_struct_hash_impl<T>(
        std::integral_constant<bool, std::is_trivially_copyable_v<T>>{});
}

// ---- 序列化辅助 ----

template <typename Writer, typename Elem>
void serialize_vector_elem(const Elem& elem, Writer& writer, bool little_endian = false);

template <typename Writer, typename FieldType>
void serialize_field(const FieldType& field, Writer& writer, bool little_endian = false);

// ---- 反序列化辅助 ----

template <typename Reader, typename Elem>
bool deserialize_vector_elem(Reader& field_reader, ssz::buffer& buf, Elem& elem, bool little_endian = false);

template <typename Reader, typename FieldType>
bool deserialize_field(Reader& field_reader, ssz::buffer& buf, FieldType& field, bool little_endian = false);

template <typename Writer, typename T>
void serialize_fields(const T& obj, Writer& writer, bool little_endian = false);

template <typename Reader, typename T>
bool deserialize_fields(Reader& field_reader, ssz::buffer& buf, T& obj, bool little_endian = false);

} // namespace ssz::detail

// -----------------------------------------------------------------------
// Public API — serialize / deserialize
// -----------------------------------------------------------------------
namespace ssz {

template <typename T>
buffer serialize(const T& obj, bool little_endian)
{
    buffer buf;
    auto writer = buf.get_write_iterator();

    *writer++ = static_cast<char>(little_endian ? 1 : 0);

    uint32_t type_hash = detail::calculate_struct_hash<T>();
    detail::ssz_write(type_hash, writer, little_endian);

    detail::serialize_fields(obj, writer, little_endian);

    return buf;
}

template <typename T>
bool deserialize(buffer& buf, T& obj)
{
    auto reader = buf.get_read_iterator();

    uint8_t endian_flag = static_cast<uint8_t>(*reader++);
    buf.advance_read(1);
    bool little_endian = (endian_flag != 0);

    uint32_t stream_hash = detail::ssz_read<uint32_t>(reader, endian_flag);
    buf.advance_read(sizeof(uint32_t));

    uint32_t expected_hash = detail::calculate_struct_hash<T>();
    if (stream_hash != expected_hash)
        return false;

    return deserialize_without_meta<T>(buf, obj, endian_flag);
}


template <typename T>
buffer serialize_without_meta(const T& obj, bool little_endian)
{
    buffer buf;
    auto writer = buf.get_write_iterator();

    detail::serialize_fields(obj, writer, little_endian);

    return buf;
}

template <typename T>
bool deserialize_without_meta(buffer& buf, T& obj, bool little_endian)
{
    auto field_reader = buf.get_read_iterator();
    return detail::deserialize_fields(field_reader, buf, obj, little_endian);
}

} // namespace ssz

// -----------------------------------------------------------------------
// Detail helper implementations
// -----------------------------------------------------------------------
namespace ssz::detail {

template <typename Writer, typename Elem>
void serialize_vector_elem(const Elem& elem, Writer& writer, bool little_endian)
{
    using T = std::decay_t<Elem>;
    if constexpr (std::integral<T>)
        ssz_write(elem, writer, little_endian);
    else if constexpr (is_ssz_struct<T>)
    {
        serialize_fields(elem, writer, little_endian);
    }
    else
    {
        const char* ptr = reinterpret_cast<const char*>(&elem);
        if (little_endian != host_is_little_endian)
        {
            for (size_t i = 0; i < sizeof(T); ++i)
                *writer++ = ptr[sizeof(T) - 1 - i];
        }
        else
        {
            for (size_t i = 0; i < sizeof(T); ++i)
                *writer++ = *ptr++;
        }
    }
}

template <typename Writer, typename FieldType>
void serialize_field(const FieldType& field, Writer& writer, bool little_endian)
{
    if constexpr (is_specialization_of_v<FieldType, std::vector>)
    {
        using value_type = typename FieldType::value_type;
        ssz_write(static_cast<uint32_t>(field.size()), writer, little_endian);
        for (const auto& elem : field)
            serialize_vector_elem(elem, writer, little_endian);
    }
    else if constexpr (std::is_same_v<FieldType, std::string>)
    {
        ssz_write(static_cast<uint32_t>(field.size()), writer, little_endian);
        for (char c : field)
            *writer++ = c;
    }
    else if constexpr (std::integral<FieldType>)
        ssz_write(field, writer, little_endian);
    else if constexpr (is_ssz_struct<FieldType>)
    {
        serialize_fields(field, writer, little_endian);
    }
    else
    {
        const char* ptr = reinterpret_cast<const char*>(&field);
        if (little_endian != host_is_little_endian)
        {
            for (size_t i = 0; i < sizeof(FieldType); ++i)
                *writer++ = ptr[sizeof(FieldType) - 1 - i];
        }
        else
        {
            for (size_t i = 0; i < sizeof(FieldType); ++i)
                *writer++ = *ptr++;
        }
    }
}

template <typename Reader, typename Elem>
bool deserialize_vector_elem(Reader& field_reader, ssz::buffer& buf, Elem& elem, bool little_endian)
{
    using T = std::decay_t<Elem>;
    if constexpr (std::integral<T>)
    {
        elem = ssz_read<T>(field_reader, little_endian);
        buf.advance_read(sizeof(T));
    }
    else if constexpr (is_ssz_struct<T>)
    {
        if (!deserialize_fields(field_reader, buf, elem, little_endian))
            return false;
    }
    else
    {
        if (little_endian != host_is_little_endian)
        {
            char* ptr = reinterpret_cast<char*>(&elem);
            for (size_t i = 0; i < sizeof(T); ++i)
                ptr[sizeof(T) - 1 - i] = *field_reader++;
        }
        else
        {
            std::memcpy(&elem, field_reader, sizeof(T));
            field_reader += sizeof(T);
        }
        buf.advance_read(sizeof(T));
    }
    return true;
}

template <typename Reader, typename FieldType>
bool deserialize_field(Reader& field_reader, ssz::buffer& buf, FieldType& field, bool little_endian)
{
    if constexpr (is_specialization_of_v<FieldType, std::vector>)
    {
        using value_type = typename FieldType::value_type;
        uint32_t size = ssz_read<uint32_t>(field_reader, little_endian);
        buf.advance_read(sizeof(uint32_t));
        field.resize(size);
        for (auto& elem : field)
        {
            if (!deserialize_vector_elem(field_reader, buf, elem, little_endian))
                return false;
        }
    }
    else if constexpr (std::is_same_v<FieldType, std::string>)
    {
        uint32_t size = ssz_read<uint32_t>(field_reader, little_endian);
        buf.advance_read(sizeof(uint32_t));
        field.assign(field_reader, field_reader + size);
        field_reader += size;
        buf.advance_read(size);
    }
    else if constexpr (std::integral<FieldType>)
    {
        field = ssz_read<FieldType>(field_reader, little_endian);
        buf.advance_read(sizeof(FieldType));
    }
    else if constexpr (is_ssz_struct<FieldType>)
    {
        if (!deserialize_fields(field_reader, buf, field, little_endian))
            return false;
    }
    else
    {
        if (little_endian != host_is_little_endian)
        {
            char* ptr = reinterpret_cast<char*>(&field);
            for (size_t i = 0; i < sizeof(FieldType); ++i)
                ptr[sizeof(FieldType) - 1 - i] = *field_reader++;
        }
        else
        {
            std::memcpy(&field, field_reader, sizeof(FieldType));
            field_reader += sizeof(FieldType);
        }
        buf.advance_read(sizeof(FieldType));
    }
    return true;
}

template <typename Writer, typename T>
void serialize_fields(const T& obj, Writer& writer, bool little_endian)
{
    boost::pfr::for_each_field(obj,
    [&writer, little_endian](const auto& field)
    {
        serialize_field(field, writer, little_endian);
    });
}

template <typename Reader, typename T>
bool deserialize_fields(Reader& field_reader, ssz::buffer& buf, T& obj, bool little_endian)
{
    bool failed = false;
    boost::pfr::for_each_field(obj,
    [&field_reader, &buf, &failed, little_endian](auto& field)
    {
        if (failed) return;
        if (!deserialize_field(field_reader, buf, field, little_endian))
            failed = true;
    });
    return !failed;
}

} // namespace ssz::detail
