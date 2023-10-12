#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    // Возвращает "текущее" время логгера
    auto GetTime() const {
        // Если метка предварительно установлена, то возвращаем её
        if (manual_ts_) {
            return *manual_ts_;
        }
        // Если метка не установлена, возвращаем текущее время
        return std::chrono::system_clock::now();
    }

    // Возвращает временнУю метку в установленном формате, пригодном для вывода
    auto GetTimeStamp() const {
        const auto now = GetTime();
        // Конвертируем время из представления time_point в time_t
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        // Возвращаем время в заданном формате (Год-Месяц-День ЧЧ:ММ:СС)
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Возвращает временнУю метку в виде строки. Используется для генерации имени файла с логами
    std::string GetFileTimeStamp() const {
        const auto t_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        // Возвращаем время в заданном формате (Год_Месяц_День)
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
        return ss.str();
    }

    // Вспомогательная функция для рекурсивного вывода аргументов в строку
    template<typename T, class... Ts>
    void Log(std::stringstream& ss, T value, const Ts&... args) {
        ss << value;
        Log(ss, args...);
    }

    // Функция-прерыватель рекурсивного вывода.
    void Log(std::stringstream& ss) {
    }

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {
        // Собираем все аргументы в промежуточную строку
        std::stringstream ss;
        // Выводим время
        ss << GetTimeStamp() << ": ";
        // Рекурсивный вызов логгирования списка аргументов
        Log(ss, args...);
        // Разом всё сбрасываем в файл
        std::string path_to_logs("/var/log/");  // Путь к папке с логами
        std::string extention(".log");          // Расширение файлов с логами
        const std::lock_guard<std::mutex> lock(file_mutex_);
        // Открываем файл по собранному пути и пишем
        std::ofstream log_file_{path_to_logs + GetFileTimeStamp() + extention, std::ios::app};
        log_file_ << ss.str() << std::endl;
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        const std::lock_guard<std::mutex> lock(ts_mutex_);
        manual_ts_ = ts;
    }

private:
    // ВременнАя метка. Если установлена, то используется при генерации времени
    std::optional<std::chrono::system_clock::time_point> manual_ts_;
    // Мьютекс для потокобезопасного изменения manual_ts_
    std::mutex ts_mutex_;
    // Мьютекс для потокобезопасного изменения manual_ts_
    std::mutex file_mutex_;
};
