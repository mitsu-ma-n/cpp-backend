#include "app.h"

#include <stdexcept>

namespace app {
using namespace std::literals;

///  ---  PlayerTokens  ---  ///

Player* PlayerTokens::FindPlayerByToken(Token token) {
    // @todo: Обработка ошибок, если такого токена нет
    return token_to_player[token];
}

Token PlayerTokens::AddPlayer(Player& player) {
    Token token(GenerateToken());
    // @todo: Обработка ошибок
    token_to_player[token] = &player;
    return token;
}

///  ---  Players  ---  ///
Player& Players::Add(model::Dog* dog, model::GameSession& session) {
    auto n_players = players_.size();
    Player::Id id(std::to_string(n_players));
    // @todo: Обработка ошибок
    return players_.emplace_back(id, *dog, session);    // Передача аргументов в конструктор игрока "на месте"
}

Player* Players::FinByDogAndMapId(model::Dog dog, model::Map::Id map) {
    return &players_[0];
}


}  // namespace app
