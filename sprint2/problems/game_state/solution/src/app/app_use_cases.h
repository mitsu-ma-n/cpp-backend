#pragma once

#include "model.h"
#include "players.h"
#include <string>

namespace app {

///  ---  ListMaps  ---  ///
class ListMapsUseCase {
public:
    // Связываем модель использования с игрой
    ListMapsUseCase(model::Game& game) 
        : game_{&game} {
    }

    // Получаем список карт
    const model::Game::Maps& GetMaps() {
        return game_->GetMaps();
    }

private:
    model::Game* game_;
};

///  ---  GetMap  ---  ///
class GetMapUseCase {
public:
    // Связываем модель использования с игрой
    GetMapUseCase(model::Game& game) 
        : game_{&game} {
    }

    // Получаем карту по её Id. @todo: Поработать с типом возвращаемого значения
    const model::Map& GetMap(const model::Map::Id &id) {
        return *game_->FindMap(id);
    }

private:
    model::Game* game_;
};


///  ---  JoinGameUseCase  ---  ///
enum class JoinGameErrorReason {
    InvalidName,
    InvalidMap
};

struct JoinGameError {
    JoinGameErrorReason reason_;
};

class JoinGameResult {
public:
    JoinGameResult(Token token, Player::Id player_id) 
        : token_{token}
        , player_id_{player_id} {
    }

    std::string GetTokenAsString() const {
        return *token_;
    }
    std::string GetPlayerIdAsString() const {
        return std::to_string(*player_id_);
    }

    auto GetPlayerId() const {
        return *player_id_;
    }

private:
    Token token_;
    Player::Id player_id_;
};


class JoinGameUseCase {
public:
    JoinGameUseCase(model::Game& game, PlayerTokens& player_tokens, Players& players);
    // Подключает игрока с указанным именем (пса) к указанной карте
    JoinGameResult JoinGame(model::Map::Id map_id, Player::Name name);

private:
    PlayerTokens* player_tokens_;
    Players* players_;
    model::Game* game_;
};

///  ---  ListPlayers  ---  ///
enum class ListPlayersErrorReason {
    InvalidToken
};

struct ListPlayersError {
    ListPlayersErrorReason reason_;
};

class PlayerInfo {
public:
    PlayerInfo(Player::Id id, Player::Name name) 
        : id_{id}
        , name_{name} {
    }

    std::string GetIdAsString() const {
        return std::to_string(*id_);
    }
    std::string GetNameAsString() const {
        return *name_;
    }

private:
    Player::Id id_;
    Player::Name name_;
};

using ListPlayersResult = std::vector<PlayerInfo>;

class ListPlayersUseCase {
public:
    ListPlayersUseCase(PlayerTokens& player_tokens, Players& players)
    : player_tokens_{&player_tokens}
    , players_{&players} {}

    // Подключает игрока с указанным именем (пса) к указанной карте
    ListPlayersResult GetPlayers(Token token);

private:
    PlayerTokens* player_tokens_;
    Players* players_;
};

///  ---  GetState  ---  ///
enum class GetStateErrorReason {
    InvalidToken
};

struct GetStateError {
    GetStateErrorReason reason_;
};

class StatePlayerInfo {
public:
    StatePlayerInfo(Player::Id id, model::Dog& dog) 
        : id_{id}
        , dog_{dog} {
    }

    std::string GetIdAsString() const {
        return std::to_string(*id_);
    }

private:
    Player::Id id_;
    model::Dog dog_;
};

using GetStateResult = std::vector<StatePlayerInfo>;

class GetStateUseCase {
public:
    GetStateUseCase(PlayerTokens& player_tokens, Players& players)
    : player_tokens_{&player_tokens}
    , players_{&players} {}

    // Подключает игрока с указанным именем (пса) к указанной карте
    GetStateResult GetState(Token token);

private:
    PlayerTokens* player_tokens_;
    Players* players_;
};

}  // namespace app
