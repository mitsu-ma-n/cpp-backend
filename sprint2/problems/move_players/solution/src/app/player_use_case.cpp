#include "player_use_case.h"
#include "model.h"
#include <cstddef>
#include <stdexcept>

namespace app {

PlayerActionUseCase::PlayerActionUseCase(PlayerTokens& player_tokens) 
    : player_tokens_{&player_tokens} {
}

// Изменяет направление движения пса
PlayerActionResult PlayerActionUseCase::ExecutePlayerAction(Token token, PlayerAction action) {
    if ( !action.IsValid() ) {
        throw PlayerActionError{PlayerActionErrorReason::InvalidMove};
    }

    auto move = action.GetMoveAsDirection();

    if ( auto self_player = player_tokens_->FindPlayerByToken(token) ) {
        double map_speed = self_player->GetSession()->GetMap().dogSpeed_;
        self_player->GetDog().SetSpeed(map_speed, move);
    } else {
        throw PlayerActionError{PlayerActionErrorReason::InvalidToken};
    }
   
    return PlayerActionResult{};
}


}  // namespace app
