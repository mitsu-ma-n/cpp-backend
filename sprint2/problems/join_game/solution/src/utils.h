#pragma once
#include <iomanip>
#include <iostream>

namespace utils {

using namespace std;

class FormattedOutput
{
    private:
        int width;
        char fill;
        stringstream& stream_obj;

    public:
        FormattedOutput(stringstream& obj, int w, char c = '0'): width(w), fill(c), stream_obj(obj) {}

        template<typename T>
        FormattedOutput& operator<<(const T& output)
        {
            stream_obj << hex << setfill(fill) << setw(width) << output;
            return *this;
        }

        FormattedOutput& operator<<(stringstream& (*func)(stringstream&))
        {
            func(stream_obj);
            return *this;
        }

        std::string str()
        {
            return stream_obj.str();
        }
};

}
