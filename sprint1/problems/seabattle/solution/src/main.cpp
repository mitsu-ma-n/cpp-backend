#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

// Распечатывает поля: своё и соперника
void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        // Игровой цикл
        while(!IsGameEnded()) {
            PrintFields();

            if (my_initiative)
            {
                std::cout << "Your turn: "s;

                // Запрос хода у пользователя
                std::string my_move;
                std::cin >> my_move;
                auto shootCell = ParseMove(my_move);
                
                if (!shootCell) {
                    std::cout << "Bad move! Try Again."s << std::endl;
                    continue;
                }

                // Отправляем выстрел
                SendMove(socket, *shootCell);

                std::pair<int,int> cell = {(*shootCell).second, (*shootCell).first};

                // Получение результата
                SeabattleField::ShotResult res = ReadResult(socket);

                // Обработка результата
                if (res == SeabattleField::ShotResult::MISS) {
                    other_field_.MarkMiss(cell.first, cell.second);
                    my_initiative = false;  // Теряем право хода при промахе
                    std::cout << "You MISS! Enemy's move."s << std::endl;
                } else if (res == SeabattleField::ShotResult::KILL) {
                    other_field_.MarkKill(cell.first, cell.second);
                    std::cout << "You KILL! Your move."s << std::endl;
                } else if (res == SeabattleField::ShotResult::HIT) {
                    other_field_.MarkHit(cell.first, cell.second);
                    std::cout << "You HIT! Your move."s << std::endl;
                }
            }
            else
            {
                std::cout << "Waiting for turn..."s << std::endl;

                // Получение хода соперника
                auto enemy_move = ReadMove(socket);

                // Обработка хода с проверкой
                if (enemy_move) {

                    auto shoot_res_str = MoveToString(*enemy_move);
                    std::cout << "Shot to "s << shoot_res_str << std::endl;

                    std::pair<int,int> cell = {(*enemy_move).second, (*enemy_move).first};
                    auto shoot_res = my_field_.Shoot(cell.first, cell.second);

                    // Обработка результата
                    if (shoot_res == SeabattleField::ShotResult::MISS) {
                        my_field_.MarkMiss(cell.first, cell.second);
                        my_initiative = true;  // Соперник промазал - получаем право хода при промахе
                        std::cout << "Opponent MISS! Your move."s << std::endl;
                    } else if (shoot_res == SeabattleField::ShotResult::KILL) {
                        my_field_.MarkKill(cell.first, cell.second);
                        std::cout << "Your ship is KILLED! Enemy's move."s << std::endl;
                    } else if (shoot_res == SeabattleField::ShotResult::HIT) {
                        my_field_.MarkHit(cell.first, cell.second);
                        std::cout << "Your ship is HITED! Enemy's move."s << std::endl;
                    }

                    // Отсылка результата
                    SendResult(socket, shoot_res);
                }
                else
                {
                    std::cout << "Bad answer from opponent. Connection closed!"s << std::endl;
                    break;
                }
            }
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 >= 8) return std::nullopt;
        if (p2 < 0 || p2 >= 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    // Получение хода от соперника
    std::optional<std::pair<int, int>> ReadMove(tcp::socket& socket) {
        auto enemy_move_str = ReadExact<2>(socket);
        return ParseMove(*enemy_move_str);
    }

    // Получение информации о результате своего выстрела от соперника
    SeabattleField::ShotResult ReadResult(tcp::socket& socket) {
        auto enemy_res_str = ReadExact<1>(socket);
        SeabattleField::ShotResult enemy_res = static_cast<SeabattleField::ShotResult>((*enemy_res_str)[0]);
        return enemy_res;
    }

    // Отсылка собственного хода сопернику
    void SendMove(tcp::socket& socket, std::pair<int,int>& coord) {
        auto move_str = MoveToString(coord);
        std::string_view move_view(move_str.c_str(),2);
        WriteExact(socket, move_view);
    }

    // Отсылка своей информации о результате выстрела соперника
    void SendResult(tcp::socket& socket, SeabattleField::ShotResult shot_res) {
        char shot = static_cast<char>(shot_res);
        std::string_view move_view(&shot,1);
        WriteExact(socket, move_view);
    }

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    net::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    boost::system::error_code ec;
    tcp::socket socket{io_context};
    acceptor.accept(socket, ec);

    if (ec) {
        std::cout << "Can't accept connection"sv << std::endl;
        return;
    }

    agent.StartGame(socket, false);
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    // Создадим endpoint - объект с информацией об адресе и порте.
    // Для разбора IP-адреса пользуемся функцией net::ip::make_address.
    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec) {
        std::cout << "Wrong IP format"sv << std::endl;
        return;
    }

    net::io_context io_context;
    tcp::socket socket{io_context};
    socket.connect(endpoint, ec);

    if (ec)
    {
        std::cout << "Can't connect to server"sv << std::endl;
        return;
    }

    agent.StartGame(socket, true);
};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    } else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
