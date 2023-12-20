#pragma once

#include "tagged.h"
#include "model.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <sstream>

namespace app {

///  ---  Player  ---  ///
class Player {
public:
    using Id = util::Tagged<int, Player>;
    using Name = util::Tagged<std::string, Player>;
    using Bag = std::vector<model::Item*>;

    Player(Id id, model::Dog& dog, model::GameSession& session) noexcept
        : id_{id}
        , name_(*dog.GetName())
        , dog_(&dog)
        , session_(&session) {
    }

    Id GetId() const {
        return id_;
    }

    Name GetName() const {
        return name_;
    }

    model::Dog& GetDog() {
        return *dog_;
    }

    model::GameSession* GetSession() const {
        return session_;
    }

    Bag GetBag() {
        return bag_;
    }

    void TakeItem(model::Item* item) {
        bag_.push_back(item);
    }

    void ClearBag() {
        bag_.clear();
    }

private:
    Id id_;
    Name name_;

    // Игровая сессия, к которой подключён игрок
    model::GameSession* session_;
    // Собака, которой управляет игрок
    model::Dog* dog_;
    // Рюкзак игрока
    Bag bag_;
};

///  ---  PlayerTokens  ---  ///
namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    PlayerTokens() {}

    // Возвращает указатель на игрока с заданным token
    Player* FindPlayerByToken(Token token);
    // Генерирует для указанного игрока токен и сохраняет получившуюся пару у себя.
    // Затем возвращает сгенерированный токен
    Token AddPlayer(Player& player);

private:
    Token GenerateToken() {
        std::stringstream ss;
        utils::FormattedOutput formatted_out(ss, 16);
        formatted_out << generator1_() << generator2_();
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
    using PlayersContainer = std::vector<Player*>;

    ~Players() {
        for (auto player : players_) {
            delete player;
        }
    }

    // Добавляет нового игрока, который будет управлять собакой dog в игровой сессии session
    Player& Add(model::Dog* dog, model::GameSession& session);
    // Возврщает указатель на игрока, который управляет собакой dog на карте map
    Player* FinByDog(model::Dog dog);

private:
    using NameHasher = util::TaggedHasher<model::Dog::Name>;
    using NameToIndex = std::unordered_map<model::Dog::Name, size_t, NameHasher>;

    PlayersContainer players_;
    NameToIndex name_to_index_;
};

}  // namespace app
