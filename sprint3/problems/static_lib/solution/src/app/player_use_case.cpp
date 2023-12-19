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
    if ( auto self_player = player_tokens_->FindPlayerByToken(token) ) {
        std::optional<model::Direction> move;
        double dog_speed;
        if ( action.IsStop() ) {
            dog_speed = 0.0;
        } else if ( action.IsDirection() ) {
            move = action.GetMoveAsDirection();
        } else {
            throw PlayerActionError{PlayerActionErrorReason::InvalidMove};
        }

        dog_speed = self_player->GetSession()->GetMap().GetDogSpeed().value();
        self_player->GetDog().SetSpeed(dog_speed, move);
    } else {
        throw PlayerActionError{PlayerActionErrorReason::InvalidToken};
    }
   
    return PlayerActionResult{};
}


}  // namespace app
