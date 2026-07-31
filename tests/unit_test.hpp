//
// Copyright (C) 2026 Jack.
//
// Author: jack
// Email:  jack.wgm at gmail dot com
//

// 极简、自包含的单元测试框架。
//
// 提供与 Boost.Test（included 模式）兼容的宏子集：
//   - BOOST_TEST_MODULE
//   - BOOST_AUTO_TEST_SUITE / BOOST_AUTO_TEST_CASE / BOOST_AUTO_TEST_SUITE_END
//   - BOOST_TEST
//
// 这样测试用例的书写方式与 Boost.Test 完全一致，但无需依赖外部 Boost 库，
// 使本项目可以开箱即用地编译和运行单元测试。

#pragma once

#include <cstdio>
#include <vector>

namespace ssz_test {

struct test_case
{
    const char* suite;
    const char* name;
    void (*fn)();
};

// 所有测试用例的注册表。
inline std::vector<test_case>& registry()
{
    static std::vector<test_case> cases;
    return cases;
}

// 全局失败计数。
inline int& failure_count()
{
    static int count = 0;
    return count;
}

// 当前正在运行的测试名（用于失败信息输出）。
inline const char*& current_test_name()
{
    static const char* name = "";
    return name;
}

// 记录一次断言结果。
inline void report(bool ok, const char* expr, const char* file, int line)
{
    if (!ok)
    {
        std::printf("    [FAIL] %s (%s:%d): %s\n", current_test_name(), file, line, expr);
        ++failure_count();
    }
}

// 注册测试用例（由 BOOST_AUTO_TEST_CASE 宏展开调用）。
struct registrar
{
    registrar(const char* suite, const char* name, void (*fn)())
    {
        registry().push_back({suite, name, fn});
    }
};

} // namespace ssz_test

// -----------------------------------------------------------------------
// 与 Boost.Test 兼容的宏
// -----------------------------------------------------------------------

#define BOOST_AUTO_TEST_SUITE(suite_name) \
    namespace suite_name { \
    static const char* const current_suite_name = #suite_name;

#define BOOST_AUTO_TEST_CASE(case_name) \
    static void case_name(); \
    static const auto case_name##_registrar = \
        ssz_test::registrar(current_suite_name, #case_name, &case_name); \
    static void case_name()

#define BOOST_AUTO_TEST_SUITE_END() }

#define BOOST_TEST(expr) \
    ssz_test::report(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

// 仅在定义了 BOOST_TEST_MODULE 时提供 main()（与 Boost.Test 的 included 模式一致）。
#ifdef BOOST_TEST_MODULE

int main()
{
    int total = 0;
    int failed = 0;

    for (const auto& c : ssz_test::registry())
    {
        ++total;
        ssz_test::current_test_name() = c.name;
        int before = ssz_test::failure_count();
        c.fn();
        if (ssz_test::failure_count() != before)
        {
            ++failed;
            std::printf("[FAILED] %s::%s\n", c.suite, c.name);
        }
        else
        {
            std::printf("[passed] %s::%s\n", c.suite, c.name);
        }
    }

    std::printf("\n%d of %d tests passed.\n", total - failed, total);
    return failed == 0 ? 0 : 1;
}

#endif // BOOST_TEST_MODULE
