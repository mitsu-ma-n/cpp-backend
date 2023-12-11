#include <gtest/gtest.h>

#include "../src/urlencode.h"
#include <set>
#include <sstream>
#include <iostream>

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    // Пустая входная строка
    EXPECT_EQ(UrlEncode(""sv), ""s);
    // Входная строка без служебных символов
    EXPECT_EQ(UrlEncode("https://vk.com/im"sv), "https%3A%2F%2Fvk.com%2Fim"s);
}

TEST(UrlEncodeTestSuite, SpetialCharsAreEncoded) {
    // Входная строка со служебными символами
    EXPECT_EQ(UrlEncode("!#$&'()*+,/:;=?@[]*"sv), "%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D%2A"s);
}

TEST(UrlEncodeTestSuite, SpaceCharAreEncoded) {
    // Входная строка с пробелами
    EXPECT_EQ(UrlEncode("wiki piki "sv), "wiki+piki+"s);
}

TEST(UrlEncodeTestSuite, AsciiCharsBefore32AndAfter128AreEncoded) {
    // Входная строка с символами с кодами меньше 31 и большими или равными 128
    EXPECT_EQ(UrlEncode("\b\t"), "%08%09"s);
    // 251 code
    EXPECT_EQ(UrlEncode("©"), "%C2%A9"s);
}
