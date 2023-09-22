#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <cassert>
#include <deque>
#include <memory>

#include "logger.h"

namespace net = boost::asio;
namespace sys = boost::system;

/*
Газовая плита - совместно используемый ресурс кафетерия
Содержит несколько горелок (burner), которые можно асинхронно занимать (метод UseBurner) и
освобождать (метод ReleaseBurner).
Если свободных горелок нет, то запрос на занимание горелки ставится в очередь.
Методы класса можно вызывать из разных потоков.
*/
class GasCooker : public std::enable_shared_from_this<GasCooker> {
public:
    using Handler = std::function<void()>;

    GasCooker(net::io_context& io, int num_burners = 8)
        : io_{io}
        , number_of_burners_{num_burners} {
    }

    GasCooker(const GasCooker&) = delete;
    GasCooker& operator=(const GasCooker&) = delete;

    ~GasCooker() {
        assert(burners_in_use_ == 0);
    }

    // Используется для того, чтобы занять горелку. handler будет вызван в момент, когда горелка
    // занята
    // Этот метод можно вызывать параллельно с вызовом других методов
    void UseBurner(Handler handler) {
        logger_.LogMessage("UseBurner on thread #"s + getThreadIdStr());
        // Выполняем работу внутри strand, чтобы изменение состояния горелки выполнялось
        // последовательно
        net::dispatch(strand_,
                      // За счёт захвата self в лямбда-функции, время жизни GasCooker будет продлено
                      // до её вызова
                      [handler = std::move(handler), self = shared_from_this(), this]() mutable {
                          assert(strand_.running_in_this_thread());
                          assert(burners_in_use_ >= 0 && burners_in_use_ <= number_of_burners_);

                          // Есть свободные горелки?
                          if (burners_in_use_ < number_of_burners_) {
                              // Занимаем горелку
                              ++burners_in_use_;
                              // Асинхронно уведомляем обработчик о том, что горелка занята.
                              // Используется асинхронный вызов, так как handler может
                              // выполняться долго, а strand лучше освободить
                              logger_.LogMessage("UseBurner:post on thread #"s + getThreadIdStr());
                              net::post(io_, std::move(handler));
                          } else {  // Все горелки заняты
                              // Ставим обработчик в хвост очереди
                              pending_handlers_.emplace_back(std::move(handler));
                          }

                          // Проверка инвариантов класса
                          assert(burners_in_use_ > 0 && burners_in_use_ <= number_of_burners_);
                      });
    }

    void ReleaseBurner() {
        logger_.LogMessage("ReleaseBurner on thread #"s + getThreadIdStr());
        // Освобождение выполняем также последовательно
        net::dispatch(strand_, [this, self = shared_from_this()] {
            assert(strand_.running_in_this_thread());
            assert(burners_in_use_ > 0 && burners_in_use_ <= number_of_burners_);

            // Есть ли ожидающие обработчики?
            if (!pending_handlers_.empty()) {
                // Выполняем асинхронно обработчик первый обработчик
                logger_.LogMessage("ReleaseBurner:post on thread #"s + getThreadIdStr());
                net::post(io_, std::move(pending_handlers_.front()));
                // И удаляем его из очереди ожидания
                pending_handlers_.pop_front();
            } else {
                // Освобождаем горелку
                --burners_in_use_;
            }
        });
    }

private:
    using Strand = net::strand<net::io_context::executor_type>;
    net::io_context& io_;
    // Задаёт порядок выполнения обработчиков, переданных в UseBurner и ReleaseBurner.
    // Гарантирует, что выполнение этих обработчиков будет ровно в том порядке, 
    // в котором они попали в очередь. Если будет несколько газовых плит в io_, то это поможет
    // выполнять в правильном порядке обработчики данной конкретной плиты
    Strand strand_{net::make_strand(io_)};
    int number_of_burners_;
    int burners_in_use_ = 0;
    // Очередь обработчиков на использование горелок
    std::deque<Handler> pending_handlers_;
    Logger logger_{"GasCooker"};
};

// RAII-класс для автоматического освобождения газовой плиты
class GasCookerLock {
public:
    GasCookerLock() = default;

    explicit GasCookerLock(std::shared_ptr<GasCooker> cooker) noexcept
        : cooker_{std::move(cooker)} {
    }

    GasCookerLock(GasCookerLock&& other) = default;
    GasCookerLock& operator=(GasCookerLock&& rhs) = default;

    GasCookerLock(const GasCookerLock&) = delete;
    GasCookerLock& operator=(const GasCookerLock&) = delete;

    ~GasCookerLock() {
        try {
            Unlock();
        } catch (...) {
        }
    }

    void Unlock() {
        if (cooker_) {
            // Освобождаем горелку
            cooker_->ReleaseBurner();
            // Операция над умным указателем! Удаляем копию shared_ptr на газовую плиту - 
            // мы больше её не используем данным объектом
            cooker_.reset();
        }
    }

private:
    std::shared_ptr<GasCooker> cooker_;
};
