#include "htmldecode.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <iostream>

std::string HtmlDecode(std::string_view str) {
    std::unordered_map<std::string_view, char> mnemonics = {
        {"lt", '<' },   {"LT", '<' }, 
        {"gt", '>' },   {"GT", '>' },
        {"amp", '&' },  {"AMP", '&' },
        {"apos", '\'' }, {"APOS", '\'' },
        {"quot", '"' }, {"QUOT", '"' }
    };

    std::string res;

    auto GetMnemonics = [&mnemonics](std::string_view str, int pos, int mnemo_length) -> std::optional<char> {
        if ( auto it = mnemonics.find(str.substr(pos+1, mnemo_length)); it != mnemonics.end() )  {
            return it->second;
        }
        return std::nullopt;
    };

    for ( size_t i = 0; i != str.size(); ++i ) {
        bool isFinded = false;
        if ( str[i] == '&' ) {   // далее возможна мнемоника
            for ( int len = 2; len < 5; ++len ) { // По всем длинам мнемоник
                // Ищем мнемонику в списке
                if ( auto it = GetMnemonics(str, i, len); it != std::nullopt ) {
                    // Если мнемоника нашлась
                    isFinded = true;
                    res += it.value();  // Сохраняем в результат
                    i += len;           // Смещаемся на длину мнемоники
                    if (str[i+1] == ';') {  // Убираем замыкающую ; если она есть
                        ++i;
                    }
                    continue;
                }
            }
        }
        // Если не найдено, то оставляем символ без изменений
        if ( !isFinded ) {
            res += str[i];
        }
    }
    return res;
}
