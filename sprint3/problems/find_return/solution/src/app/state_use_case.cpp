#include "state_use_case.h"
#include <stdexcept>

namespace app {

// Для игрока с указанным токеном выдаёт список игроков, которые находятся вместе с ним в одной сессии
GetStateResult GetStateUseCase::GetState(Token token) {
    GetStateResult res;
    // Получаем игрока с заданным токеном
    if ( auto self_player = player_tokens_->FindPlayerByToken(token) ) {
        // Получаем сессию, к которой подключен игрок и список собак в сессии
        auto session = self_player->GetSession();
        auto dogs = session->GetDogs();

        // Для каждой собаки находим игрока и складываем в результат
        for ( auto dog : dogs ) {
            auto dog_name = dog->GetName();
            auto player = players_->FinByDog(*dog);
            res.players_.push_back({player->GetId(), *dog});
        }

        // Для карты выдаём список лута на ней
        res.items_ = session->GetItems();
    } else {
        throw GetStateError{GetStateErrorReason::InvalidToken};
    }

    return res;
}

}  // namespace app
