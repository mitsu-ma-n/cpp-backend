#include "urlencode.h"

#include <iomanip>
#include <set>
#include <sstream>

std::string UrlEncode(std::string_view str) {
    std::ostringstream oss;

    const char tmp[] = "!#$&'()*+,/:;=?@[]";
    std::set<char> not_encode{tmp, tmp + sizeof(tmp) - 1}; 

    for (auto& c : str) {
        if (not_encode.contains(c) || c < 32 || c >= 128) {
            oss << "%" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (0xff & c);
        } else if ( c == ' ' ) {
            oss << "+";
        } else {
            oss << c;
        }
    }
    return oss.str();
}
