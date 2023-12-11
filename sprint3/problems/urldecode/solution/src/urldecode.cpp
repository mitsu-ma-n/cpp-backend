#include "urldecode.h"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <stdexcept>
#include <regex>

#include <set>
#include <sstream>
#include <iostream>

using namespace std::literals;

template<typename T>
std::string to_hex(T c) {
    std::stringstream ss;
    ss << '%' << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(c);
    std::cout << "char: " << c << " , string: " << ss.str() << std::endl;
    return ss.str();
}


std::string UrlDecode(std::string_view str) {
    std::string result;
    result.reserve(str.size());
    
    for (std::size_t i = 0; i < str.size(); ++i)
    {
        auto ch = str[i];
        
        if (ch == '%' && (i + 2) < str.size())
        {
            std::string hex{str.substr(i + 1, 2)};
            std::transform(hex.begin(), hex.end(), hex.begin(), ::toupper);
            std::smatch sm;
            if( !std::regex_match(hex, sm, std::regex("[0-9A-F]{2}")) ) {
                throw std::invalid_argument("Invalid URL encoding");
            }
            auto dec = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            result.push_back(dec);
            i += 2;
        }
        else if (ch == '+')
        {
            result.push_back(' ');
        }
        else
        {
            result.push_back(ch);
        }
    }
    
    return result;
}

std::string UrlEncode(std::string_view str) {
    const char tmp[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    std::set<char> not_encode{tmp, tmp + sizeof(tmp) - 1}; 

    std::string res;

    auto st = str.begin();// указывает на текущий символ строки
    // запускаем цикл, пока не кончится строка
    while (st != str.end()) {
        // Если символ в списке допустимых
        if( not_encode.contains(*st) ) { // тогда копируем его как есть
            res.push_back(*st);
        } else {    // Преобразуем
            res += to_hex(*st);
        }
        ++st;
    }

    return res;
}
