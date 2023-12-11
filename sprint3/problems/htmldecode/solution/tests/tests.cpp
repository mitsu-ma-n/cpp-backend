#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text empty string", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
}

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("hello"sv) == "hello"s);
}

TEST_CASE("Text with mnemonics lower case", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);
}

TEST_CASE("Text with mnemonics upper case", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == "Johnson&Johnson"s);
}

TEST_CASE("Text with mnemonics mixed case", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&AmPJohnson"sv) == "Johnson&AmPJohnson"s);
}

TEST_CASE("Text with mnemonics in different positions", "[HtmlDecode]") {
    CHECK(HtmlDecode("&APOSJohnson&AMPJohnson&lt"sv) == "\'Johnson&Johnson<"s);
}

TEST_CASE("Text with unfinished mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&AMJohnson"sv) == "Johnson&AMJohnson"s);
}

TEST_CASE("Test \";\" symbol", "[HtmlDecode]") {
    CHECK(HtmlDecode("Johnson&AMP;Johnson"sv) == "Johnson&Johnson"s);
}

