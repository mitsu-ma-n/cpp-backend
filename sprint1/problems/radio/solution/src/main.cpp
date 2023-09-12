#include "audio.h"
#include <boost/asio.hpp>

#include <iostream>

namespace net = boost::asio;
using net::ip::udp;

using namespace std::literals;

// Максимальное количество фрэймов
static const int max_n_frames = 65000;
// Максимальный размер буфера в байтах
static const size_t max_buffer_size = 65000;

void PrintBadArgsMessage(const char *taskName)
{
    std::cout << "Usage: "sv << taskName << " server|client"sv
              << " <connection port>"sv << std::endl;
}

void StartServer(uint16_t port)
{
    try
    {
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
        for (;;)
        {
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
            std::array<char, max_buffer_size> recv_buf;

            // Получаем не только данные, но и endpoint клиента
            // size - размер полученных данных. Количество фрэймов может быть другим
            auto datagram_size = socket.receive(boost::asio::buffer(recv_buf));

            Player player(ma_format_u8, 1);

            int frame_size = player.GetFrameSize();
            size_t n_frames = datagram_size / frame_size;

            // Воспроизводим принятые данные
            player.PlayBuffer(recv_buf.data(), n_frames, 1.5s);
            std::cout << "Playing done" << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void StartClient(uint16_t port)
{
    // По заданию через аргументы передаётся только порт. Прописываем IP сервера прямо в коде
    // Использую localhost для тестирования локально и чтобы не светить IP на GitHub
    static const char server_IP[] = "localhost";

    Recorder recorder(ma_format_u8, 1);
    std::string str; // Промежуточная строка для получения ввода от пользователя
    std::cout << "Press Enter to record message..." << std::endl;
    std::getline(std::cin, str);

    // Производим запись в rec_result
    auto rec_result = recorder.Record(max_n_frames, 1.5s);
    std::cout << "Recording done" << std::endl;

    int frame_size = recorder.GetFrameSize();
    size_t n_frames = rec_result.frames;
    size_t datagram_size = frame_size * n_frames;

    if (datagram_size > max_buffer_size)
    {
        std::cout << "Can't send datagram: size record" << datagram_size << " grater then max buffer size " << max_buffer_size << std::endl;
        return;
    }

    // Отправляем данные на сервер
    try
    {
        net::io_context io_context;

        // Перед отправкой данных нужно открыть сокет.
        // При открытии указываем протокол (IPv4 или IPv6) вместо endpoint.
        udp::socket socket(io_context, udp::v4());

        boost::system::error_code ec;
        auto endpoint = udp::endpoint(net::ip::make_address(server_IP, ec), port);
        socket.send_to(net::buffer(rec_result.data, datagram_size), endpoint);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main(int argc, char **argv)
{
    // Проверка запуска
    if (argc != 3)
    {
        PrintBadArgsMessage(argv[0]);
        return 1;
    }
    std::string appType = std::string(argv[1]);
    if (appType != "server"sv && appType != "client"sv)
    {
        PrintBadArgsMessage(argv[0]);
        return 1;
    }

    int port = -1;
    try
    {
        port = std::stoi(std::string(argv[2]));
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << "Bad input: " << argv[2] << " isn't port number" << std::endl;
        return 1;
    }

    if (appType == "client"sv) // Клиентская часть приложения
    {
        StartClient(port);
    }

    if (appType == "server"sv) // Серверная часть приложения
    {
        StartServer(port);
    }

    return 0;
}
