#pragma once

#include "model.h"
#include "players.h"
#include <string>
#include <set>
#include <optional>

namespace app {

// Класс реализует хранение допустимых действий игрока
// и преобразование их в различное представление
class PlayerAction {
public:
    PlayerAction(std::string move) 
        : move_(std::move(move)) {
        }

    std::string GetMoveAsString() const {
        return move_;
    }

    std::optional<model::Direction> GetMoveAsDirection() const {
        if (this->IsDirection()) {
            return model::Direction(move_[0]);
        }
        return std::nullopt;
    }


    bool IsDirection() const {
        // Список допустимых направлений
        std::set<std::string> valid_directions = {
            {(char)model::Direction::NORTH}, 
            {(char)model::Direction::SOUTH}, 
            {(char)model::Direction::EAST}, 
            {(char)model::Direction::WEST}
        };
        return valid_directions.find(move_) != valid_directions.end();
    }

    bool IsStop() const {
        using namespace std::string_literals;
        return move_ == ""s;
    }

private:
    std::string move_;
};

enum class PlayerActionErrorReason {
    InvalidToken,
    InvalidMove
};

struct PlayerActionError {
    PlayerActionErrorReason reason_;
};

class PlayerActionResult {
public:
    PlayerActionResult() {}

private:
};


class PlayerActionUseCase {
public:
    PlayerActionUseCase(PlayerTokens& player_tokens);
    // Подключает игрока с указанным именем (пса) к указанной карте
    PlayerActionResult ExecutePlayerAction(Token token, PlayerAction action);

private:
    PlayerTokens* player_tokens_;
};

}  // namespace app
