#pragma once
#include <boost/asio/steady_timer.hpp>
#include <iomanip>
#include <iostream>
#include <optional>
#include <utility>
#include <memory>
#include <chrono>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/strand.hpp>


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

namespace my_random {
    double GetRandomDouble(double min, double max);
    size_t GetRandomIndex(size_t min, size_t max);
}

namespace validators {
    bool IsValidToken(const std::string_view& token);
}

namespace geometry {
    template<typename T>
    std::optional<std::pair<T,T>> GetIntersection(std::pair<T,T> a, std::pair<T,T> b) {
        bool reverse_move = false;
        if(a.first > a.second) {
            std::swap(a.first, a.second);
            reverse_move = true;
        }

        if ( a.first > b.second || a.second < b.first ) {
            return std::nullopt;
        }

        std::pair<T,T> res{max(a.first, b.first), min(a.second, b.second)};

        if (reverse_move) {
            std::swap(res.first, res.second);
        }
        return res;
    }

    template<typename T>
    bool IsInInterval(T point, std::pair<T,T> interval) {
        return point >= interval.first && point <= interval.second;
    }

    template<typename T, typename U>
    std::optional<T> GetMaxMoveOnSegment(T move_start, T move_end, U road_beg, U road_end) {
        // Стартуем вне дороги - такого быть не может
        if (move_start < road_beg || move_start > road_end) {
            return std::nullopt;
        }

        auto res_beg = std::max(move_start, road_beg);
        auto res_end = std::min(move_end, road_end);

        // Если полученный отрезок существует, возвращаем его
        if (res_beg <= res_end) {
            return res_end;
        }

        return std::nullopt;
    }
}

namespace net = boost::asio;
namespace sys = boost::system;

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    // Функция handler будет вызываться внутри strand с интервалом period
    Ticker(Strand strand, std::chrono::milliseconds period, Handler handler)
        : strand_{strand}
        , period_{period}
        , handler_{std::move(handler)} {
    }

    void Start() {
        net::dispatch(strand_, [self = shared_from_this()] {
            self->last_tick_ = Clock::now();
            self->ScheduleTick();
        });
    }

private:
    void ScheduleTick() {
        assert(strand_.running_in_this_thread());
        timer_.expires_after(period_);
        timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
            self->OnTick(ec);
        });
    }

    void OnTick(sys::error_code ec) {
        using namespace std::chrono;
        assert(strand_.running_in_this_thread());

        if (!ec) {
            auto this_tick = Clock::now();
            auto delta = duration_cast<milliseconds>(this_tick - last_tick_);
            last_tick_ = this_tick;
            try {
                handler_(delta);
            } catch (...) {
            }
            ScheduleTick();
        }
    }

    using Clock = std::chrono::steady_clock;

    Strand strand_;
    std::chrono::milliseconds period_;
    net::steady_timer timer_{strand_};
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};
}
