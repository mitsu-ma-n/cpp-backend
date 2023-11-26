#pragma once

#include "model.h"
#include "players.h"
#include <string>
#include <set>
#include <optional>

namespace app {

class PlayerAction {
public:
    PlayerAction(std::string move) 
        : move_(std::move(move)) {
        }

    std::string GetMove() const {
        return move_;
    }

    std::optional<model::Direction> GetMoveAsDirection() const {
        if (this->IsValid()) {
            return model::Direction(move_[0]);
        }
        return std::nullopt;
    }


    bool IsValid() const {
        // Список допустимых направлений
        std::set<std::string> valid_directions = {
            {(char)model::Direction::NORTH}, 
            {(char)model::Direction::SOUTH}, 
            {(char)model::Direction::EAST}, 
            {(char)model::Direction::WEST}
        };
        return valid_directions.find(move_) != valid_directions.end();
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
