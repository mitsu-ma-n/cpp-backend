#pragma once

#include "model.h"
#include "app.h"

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


enum JoinGameErrorReason {
    InvalidName,
    InvalidMap
};

struct JoinGameError {
    JoinGameErrorReason reason_;
};

///  ---  JoinGameResult  ---  ///
class JoinGameResult {
public:
    JoinGameResult(Token token, Player::Id player_id) 
        : token_{token}
        , player_id_{player_id} {
    }
private:
    Token token_;
    Player::Id player_id_;
};


///  ---  JoinGameUseCase  ---  ///
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
class ListPlayersUseCase {
public:
    ListPlayersUseCase(PlayerTokens& player_tokens, Players& players);
    // Подключает игрока с указанным именем (пса) к указанной карте
    JoinGameResult JoinGame(model::Map::Id map_id, Player::Name name);

private:
    PlayerTokens* player_tokens_;
    Players* players_;
};

///  ---  Application  ---  ///
class Application {
public:
    Application(model::Game& game)
    : join_game_{game, tokens_, players_}
    , list_maps_{game}
    , get_map_{game} {

    }

    // Выдаёт список доступных карт 
    model::Game::Maps ListMaps();
    // Выдаёт ссылку на карту по её ID
    const model::Map& FindMap(model::Map::Id map_id);
    // Подключает игрока к указанной карте @todo: Какого игрока???
    JoinGameResult JoinGame(model::Map::Id map_id);

private:
    Players players_;
    PlayerTokens tokens_;
    JoinGameUseCase join_game_;
    ListMapsUseCase list_maps_;
    GetMapUseCase get_map_;
};

}  // namespace app
