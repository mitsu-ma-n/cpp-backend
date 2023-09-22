#pragma once
#include <functional>
#include <optional>

#include "logger.h"

#include "clock.h"
#include "gascooker.h"

/*
Класс "Сосиска".
Позволяет себя обжаривать на газовой плите
*/
class Sausage : public std::enable_shared_from_this<Sausage> {
public:
    using Handler = std::function<void()>;

    explicit Sausage(int id)
        : id_{id} {
    }

    int GetId() const {
        return id_;
    }

    // Асинхронно начинает приготовление. Вызывает handler, как только началось приготовление
    void StartFry(GasCooker& cooker, Handler handler) {
        logger_.LogMessage("StartFry on thread #"s + getThreadIdStr());
        // Метод StartFry можно вызвать только один раз
        if (frying_start_time_) {
            throw std::logic_error("Frying already started");
        }

        // Запрещаем повторный вызов StartFry
        frying_start_time_ = Clock::now();

        // Готовимся занять газовую плиту
        gas_cooker_lock_ = GasCookerLock{cooker.shared_from_this()};

        // Занимаем горелку для начала обжаривания.
        // Чтобы продлить жизнь текущего объекта, захватываем shared_ptr в лямбде
        cooker.UseBurner([self = shared_from_this(), handler = std::move(handler)] {
            // Запоминаем время фактического начала обжаривания
            self->frying_start_time_ = Clock::now();
            self->logger_.LogMessage("UseBurner handler call on thread #"s + getThreadIdStr());
            handler();
        });
    }

    // Завершает приготовление и освобождает горелку
    void StopFry() {
        logger_.LogMessage("StopFry on thread #"s + getThreadIdStr());
        if (!frying_start_time_) {
            throw std::logic_error("Frying has not started");
        }
        if (frying_end_time_) {
            throw std::logic_error("Frying has already stopped");
        }
        frying_end_time_ = Clock::now();
        // Освобождаем горелку
        gas_cooker_lock_.Unlock();
        logger_.LogMessage("gas_cooker unlocked on thread #"s + getThreadIdStr());
    }

    bool IsCooked() const noexcept {
        return frying_start_time_.has_value() && frying_end_time_.has_value();
    }

    Clock::duration GetCookDuration() const {
        if (!frying_start_time_ || !frying_end_time_) {
            throw std::logic_error("Sausage has not been cooked");
        }
        return *frying_end_time_ - *frying_start_time_;
    }

private:
    int id_;
    Logger logger_{"sausage #" + std::to_string(id_)};
    GasCookerLock gas_cooker_lock_;
    std::optional<Clock::time_point> frying_start_time_;
    std::optional<Clock::time_point> frying_end_time_;
};

// Класс "Хлеб". Ведёт себя аналогично классу "Сосиска"
class Bread : public std::enable_shared_from_this<Bread> {
public:
    using Handler = std::function<void()>;

    explicit Bread(int id)
        : id_{id} {
    }

    int GetId() const {
        return id_;
    }

    // Начинает приготовление хлеба на газовой плите. Как только горелка будет занята, вызовет
    // handler
    void StartBake(GasCooker& cooker, Handler handler) {
        logger_.LogMessage("StartBake on thread #"s + getThreadIdStr());
        // Метод StartBake можно вызвать только один раз.
        // Проверка делается через optional-поле класса. Проверяем, инициализировано оно или нет
        if (baking_start_time_) {
            throw std::logic_error("Baking already started");
        }

        // Запрещаем повторный вызов StartBake через инициализацию optional-поля
        baking_start_time_ = Clock::now();

        // Готовимся занять газовую плиту. Для себя взвели "флаг" через 
        // создание объекта GasCookerLock (инициализацию gas_cooker_lock_)
        gas_cooker_lock_ = GasCookerLock{cooker.shared_from_this()};

        // Занимаем горелку для начала выпекания.
        // Чтобы продлить жизнь текущего объекта, захватываем shared_ptr в лямбде
        // Объект сосиски в лямбде нужен для правильной установки времени приготовления
        cooker.UseBurner([self = shared_from_this(), handler = std::move(handler)] {
            // Запоминаем время фактического начала выпекания
            self->baking_start_time_ = Clock::now();
            // Вызываем обработчик, который получили снаружи - то, что нужно сделать в процессе готовки
            // и по её завершению. Тут должны ждать, пока не испечётся
            self->logger_.LogMessage("UseBurner handler call on thread #"s + getThreadIdStr());
            handler();
        });
    }

    // Останавливает приготовление хлеба и освобождает горелку.
    void StopBaking() {
        logger_.LogMessage("StopBaking on thread #"s + getThreadIdStr());
       // Проверка на ошибки в порядке готовки:
        // Если не инициализировано начало готовки, значит ещё не вызывали StartBake()
        if (!baking_start_time_) {
            throw std::logic_error("Frying has not started");
        }
        // Если уже инициализировано окончание готовки, значит произошёл повторный вызов StopBaking()
        if (baking_end_time_) {
            throw std::logic_error("Frying has already stopped");
        }
        // Инициализируем и запоминаем время окончания готовки
        baking_end_time_ = Clock::now();
        // Освобождаем горелку через снятие "замка"
        gas_cooker_lock_.Unlock();
        logger_.LogMessage("gas_cooker unlocked on thread #"s + getThreadIdStr());
    }

    // Информирует, испечён ли хлеб
    bool IsCooked() const noexcept {
        return baking_start_time_.has_value() && baking_end_time_.has_value();
    }

    // Возвращает продолжительность выпекания хлеба. Бросает исключение, если хлеб не был испечён
    Clock::duration GetBakingDuration() const {
        if (!baking_start_time_ || !baking_end_time_) {
            throw std::logic_error("Sausage has not been cooked");
        }
        return *baking_end_time_ - *baking_start_time_;
    }

private:
    int id_;
    Logger logger_{"bread #" + std::to_string(id_)};
   // "замок" для того, чтобы корректно сообщать плите, когда её занимают и когда освобождают.
    GasCookerLock gas_cooker_lock_;
    std::optional<Clock::time_point> baking_start_time_;
    std::optional<Clock::time_point> baking_end_time_;
};

// Склад ингредиентов (возвращает ингредиенты с уникальным id)
class Store {
public:
    std::shared_ptr<Bread> GetBread() {
        return std::make_shared<Bread>(++next_id_);
    }

    std::shared_ptr<Sausage> GetSausage() {
        return std::make_shared<Sausage>(++next_id_);
    }

private:
        std::atomic_int next_id_ = 0;
};
