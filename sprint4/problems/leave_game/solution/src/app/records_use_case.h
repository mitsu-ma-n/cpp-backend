#pragma once

#include "game.h"

namespace app {

class RecordsUseCase {
public:
    RecordsUseCase(model::Game& game) 
        : game_{&game} {
    }

    // Получаем список карт
    const model::Game::Maps& GetMaps() {
        return game_->GetMaps();
    }

private:
    model::Game* game_;
};

}  // namespace app
