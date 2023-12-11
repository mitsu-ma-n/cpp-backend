#include <iostream>

#include "urldecode.h"

int main() {
    using namespace std::literals;

    try {
        std::string s("https://ru.wikipedia.org/wiki/%D0%97%D0%B0%d0%B3%D0%Bb%D0%b0%D0%B2%D0%bD%D0%B0%D1%8f%D0%bb");
        //std::getline(std::cin, s);

        std::cout << UrlDecode(s) << std::endl;

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: "sv << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error"sv << std::endl;
    }

    return EXIT_FAILURE;
}
