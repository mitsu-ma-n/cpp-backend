#include "players.h"

#include <boost/smart_ptr/make_shared_object.hpp>
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

    if (auto [it, inserted] = name_to_index_.emplace(name, index); !inserted) {
        // Игрок с таким именем уже есть
        throw std::invalid_argument("Name "s + *name + " already exists"s);
    } else {
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
}

void Players::Add(std::unique_ptr<Player> player) {
    auto name = model::Dog::Name(*player->GetName());
    auto index = *player->GetId(); 
    if (auto [it, inserted] = name_to_index_.emplace(name, index); !inserted) {
        // Игрок с таким именем уже есть
        throw std::invalid_argument("Name "s + *name + " already exists"s);
    } else {
        if ( index >= players_.size() ) {
            players_.resize(index + 1);
        }
        players_[index] =std::move(player);
    }
}



Player* Players::FinByDog(model::Dog dog) {
    auto name = dog.GetName();
    if (auto it = name_to_index_.find(name); it != name_to_index_.end()) {
        return players_.at(it->second).get();
    }

    return nullptr;
}
///  ---  Players  ---  ///


}  // namespace app
