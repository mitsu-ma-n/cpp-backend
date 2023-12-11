#include "urldecode.h"

#include <charconv>
#include <stdexcept>

#include <set>
#include <sstream>

using namespace std::literals;

std::string to_hex(char c) {
    std::stringstream ss;
    ss << std::hex << c;
    return ss.str();
}

std::string UrlDecode(std::string_view str) {
    const char tmp[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    std::set<char> not_decode{tmp, tmp + sizeof(tmp) - 1}; 

    std::string res;

    auto st = str.begin();// указывает на текущий символ строки
    // запускаем цикл, пока не кончится строка
    while (st != str.end()) {
        // Если символ в списке допустимых
        if( not_decode.contains(*st) ) { // тогда копируем его как есть
            res.push_back(*st);
        } else {    // Преобразуем
            res += to_hex(*st);
        }
    }

    return res;
}
