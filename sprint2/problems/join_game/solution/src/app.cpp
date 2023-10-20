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

Player& Players::Add(model::Dog dog, model::GameSession session) {
    Player player(dog,session);
    return player;
}

Player* Players::FinByDogAndMapId(model::Dog dog, model::Map::Id map) {
    return &players_[0];
}

}  // namespace model
