#pragma once
#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <syncstream>
#include <sstream>
#include <thread>

using namespace std::chrono;
using namespace std::literals;

namespace net = boost::asio;

using Timer = net::steady_timer;

std::string getThreadIdStr()
{
    std::stringstream o;
    o << std::this_thread::get_id();
    return o.str();
}

class Logger {
public:
    explicit Logger(std::string id)
        : id_(std::move(id)) {
    }

    void LogMessage(std::string_view message) const {
        std::osyncstream os{std::cout};
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
           << "s] "sv << message << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
}; 
