#pragma once

#include "game_session.h"
#include "utils.h"
#include "tagged_uuid.h"
#include "serializer.h"

#include <random>
#include <memory>
#include <map>

namespace app {

///  ---  Player  ---  ///
class Player {
public:
    using Id = util::Tagged<int, Player>;
    using Name = util::Tagged<std::string, Player>;

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

    const model::Dog& GetDog() const {
        return *dog_;
    }

    void SetDogSpeed(model::DynamicDimension speed, std::optional<model::Direction> direction) {
        dog_->SetSpeed(speed, direction);
    }

    model::GameSession* GetSession() const {
        return session_;
    }

    serializer::SerPlayer GetSerPlayer() const {
        serializer::SerPlayer player;
        player.id = *id_;
        player.name = *name_;
        return player;
    }

    double GetPlayTime() const {
        return play_time_;
    }

    void AddPlayTime(double time) {
        play_time_ += time;
    }

    double GetSleepTime() const {
        return dog_->GetSleepTime();
    }

    bool HasRetirementState() const {
        // Собака не активна и время её бездействия превышает время бездействия на карте
        auto map = session_->GetMap();
        auto dog_sleep_time = dog_->GetSleepTime();
        //return !dog_->IsActive() && play_time_ - dog_sleep_time > RetirementTime();
        return false;
    }

private:
    Id id_;
    Name name_;

    // Игровая сессия, к которой подключён игрок
    model::GameSession* session_;
    // Собака, которой управляет игрок
    model::Dog* dog_;

    // Время в игре в секундах
    double play_time_{0.0};
};

namespace detail {
struct PlayerTag {};
}  // namespace detail

using PlayerId = util::TaggedUUID<detail::PlayerTag>;

struct PlayerRecordInfo {
    PlayerId id;
    std::string name;
    int score;
    double play_time;
};

struct PlayerStatInfo {
    std::string name;
    int score;
    double play_time;
};

class PlayerRepository {
public:
    virtual void Save(const PlayerRecordInfo& player) = 0;
    virtual std::vector<PlayerStatInfo> GetRecords(size_t start, size_t limit) = 0;

protected:
    ~PlayerRepository() = default;
};


///  ---  PlayerTokens  ---  ///
namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    using IdHasher = util::TaggedHasher<Player::Id>;
    using PlayersIdsToTokens = std::unordered_map<Player::Id, Token, IdHasher>;

    PlayerTokens() {}

    // Возвращает указатель на игрока с заданным token
    Player* FindPlayerByToken(Token token);
    // Генерирует для указанного игрока токен и сохраняет получившуюся пару у себя.
    // Затем возвращает сгенерированный токен
    Token AddPlayer(Player& player);
    // Добавляет ccылку на игрока с уже известным токеном
    void AddPlayer(Player* player, Token token) {
        token_to_player[token] = player;
    }
    // Возвращет балицу игроков в виде id - Token
    PlayersIdsToTokens GetPlayersTokens() const {
        std::unordered_map<Player::Id, Token, IdHasher> player_id_to_token;
        for (const auto& [token, player] : token_to_player) {
            player_id_to_token[player->GetId()] = token;
        }
        return player_id_to_token;
    };


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
    using PlayersContainer = std::vector<std::shared_ptr<Player>>;

    // Добавляет нового игрока, который будет управлять собакой dog в игровой сессии session
    Player& Add(model::Dog* dog, model::GameSession& session);
    // Добавляет существующего игрока, который будет управлять собакой dog в игровой сессии session
    void Add(std::unique_ptr<Player> player);
    // Возврщает указатель на игрока, который управляет собакой dog в игровой сессии session
    Player* FinByDog(const model::Dog& dog, const model::GameSession& session);

    serializer::SerPlayers GetSerPlayers() const {
        serializer::SerPlayers ser_players;
        for (auto player : players_) {
            ser_players.players.push_back(player->GetSerPlayer());
        }
        for (auto [name, index] : name_to_index_) {
            ser_players.names.push_back(*name);
            ser_players.indexes.push_back(index);
        }
        return ser_players;
    }

    const PlayersContainer& GetPlayers() const {
        return players_;
    }

private:
    // Пришлось перейти на multimap, потому что игроки с одинаковыми именами могут подключаться 
    // к разным картам, то есть находиться в разных сессиях
    using NameToIndex = std::multimap<model::Dog::Name, size_t>;

    PlayersContainer players_;
    NameToIndex name_to_index_;
};

}  // namespace app
