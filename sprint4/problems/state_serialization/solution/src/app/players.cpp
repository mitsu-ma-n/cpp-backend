#include "players.h"
#include "game_session.h"

#include <boost/smart_ptr/make_shared_object.hpp>
#include <iostream>
#include <stdexcept>

namespace app {
using namespace std::literals;

///  ---  PlayerTokens  ---  ///

Player* PlayerTokens::FindPlayerByToken(Token token) {
    if ( auto it = token_to_player.find(token); it != token_to_player.end() ) {
        return token_to_player[token];
    }
    return nullptr;
}

Token PlayerTokens::AddPlayer(Player& player) {
    Token token(GenerateToken());
    // @todo: Обработка ошибок. Вдруг токены закончились и попался повторяющийся
    token_to_player[token] = &player;
    return token;
}
///  ---  PlayerTokens  ---  ///

///  ---  Players  ---  ///

Player& Players::Add(model::Dog* dog, model::GameSession& session) {
    auto index = players_.size();
    Player::Id id(index);

    auto name = dog->GetName();

    auto it = name_to_index_.emplace(name, index);

    try {
        // Добавляем игрока
        players_.emplace_back(std::make_shared<Player>(id, *dog, session));
        return *players_.back();
    } catch (...) {
        // Не получилось. Откатываем изменения в name_to_index_
        name_to_index_.erase(it);
        throw;
    }
}

void Players::Add(std::unique_ptr<Player> player) {
    auto name = model::Dog::Name(*player->GetName());
    auto index = *player->GetId(); 

    auto it = name_to_index_.emplace(name, index);

    if ( index >= players_.size() ) {
        players_.resize(index + 1);
    }
    players_[index] =std::move(player);
}



Player* Players::FinByDog(const model::Dog& dog, const model::GameSession& session) {
    const auto& name = dog.GetName();

    // Ищем всех игроков с таким именем
    for (auto [itr, rangeEnd] = name_to_index_.equal_range(name); itr != rangeEnd; ++itr) {
        const auto& player = players_.at(itr->second);
        // Выбираем того игрока, который подключен к заданной сессии
        if ( player->GetSession() == &session ) {
            return player.get();
        }
    }

    return nullptr;
}
///  ---  Players  ---  ///


}  // namespace app
