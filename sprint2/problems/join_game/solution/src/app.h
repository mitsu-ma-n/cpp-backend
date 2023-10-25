#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <sstream>
#include <iostream>

#include "tagged.h"
#include "model.h"

namespace app {

///  ---  Player  ---  ///
class Player {
public:
    using Id = util::Tagged<std::string, Player>;
    using Name = std::string;

    Player(Id id, model::Dog& dog, model::GameSession& session) noexcept
        : id_{id}
        , dog_(&dog)
        , session_(&session) {
    }

    Id GetId() {
        return id_;
    }

    Name GetName() {
        return name_;
    }


private:
    Id id_;
    Name name_;

    // Игровая сессия, к которой подключён игрок
    model::GameSession* session_;
    // Собака, которой управляет игрок
    model::Dog* dog_;
};

///  ---  PlayerTokens  ---  ///
namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    PlayerTokens() {

    }

    // Возвращает указатель на игрока с заданным token
    Player* FindPlayerByToken(Token token);
    // Генерирует для указанного игрока токен и сохраняет получившуюся пару у себя.
    // Затем возвращает сгенерированный токен
    Token AddPlayer(Player& player);

private:
    Token GenerateToken() {
        std::stringstream ss;
        ss << std::hex << generator1_() << generator2_();
        return Token(ss.str());
    }

private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};

    using TokenHasher = util::TaggedHasher<Token>;
    using TokenToPlayer = std::unordered_map<Token, Player*, TokenHasher>;

    TokenToPlayer token_to_player;
};

///  ---  Players  ---  ///
class Players {
public:
    using PlayersContainer = std::vector<Player>;

    // Добавляет нового игрока, который будет управлять собакой dog в игровой сессии session
    Player& Add(model::Dog* dog, model::GameSession& session);
    // Возврщает указатель на игрока, который управляет собакой dog на карте map
    Player* FinByDogAndMapId(model::Dog dog, model::Map::Id map);
/*
    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }
*/
private:
    using TokenHasher = util::TaggedHasher<Token>;
    using TokenToIndex = std::unordered_map<Token, size_t, TokenHasher>;

    PlayersContainer players_;
    TokenToIndex token_to_index_;
};

}  // namespace app
