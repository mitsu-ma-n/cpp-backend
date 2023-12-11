#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    // Пустая строка.
    BOOST_TEST(UrlDecode(""sv) == ""s);
    // Строка без %-последовательностей.
    BOOST_TEST(UrlDecode("https://vk.com/im?sel=c1"sv) == "https://vk.com/im?sel=c1"s);
    // Строка с валидными %-последовательностями, записанными в разном регистре.
    BOOST_TEST(UrlDecode("https://ru.wikipedia.org/wiki/%D0%97%D0%B0%d0%B3%D0%Bb%D0%b0%D0%B2%D0%bD%D0%B0%D1%8f%D0%bb"sv) == "https://ru.wikipedia.org/wiki/Заглавнаял"s);
    // Строка с невалидными %-последовательностями.
    BOOST_CHECK_THROW(UrlDecode("wiki/%LL%97%DX%B0"sv), std::invalid_argument);
    // Строка с неполными %-последовательностями.
    BOOST_CHECK_THROW(UrlDecode("wiki/%D%97%"sv), std::invalid_argument);
    // Строка с символом +.
    BOOST_TEST(UrlDecode("https://ru.wikipedia.org/wiki/%D0%97+%D0%97"sv) == "https://ru.wikipedia.org/wiki/З З"s);
    // Напишите остальные тесты для функции UrlDecode самостоятельно
}