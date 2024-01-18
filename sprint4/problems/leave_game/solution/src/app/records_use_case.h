#pragma once

#include "players.h"
#include "postgres/postgres.h"

namespace app {

struct RecorsInfo {
    size_t start;
    size_t limit;
};

struct RecordsResult {
    std::vector<PlayerStatInfo> records;
};
   

class RecordsUseCase {
public:
    RecordsUseCase(Players& players, postgres::Database& db) 
        : players_{&players}
        , db_{&db} {
    }

    // Получаем список карт
    RecordsResult GetRecords(RecorsInfo info) {
        return RecordsResult(db_->GetPlayers().GetRecords(info.start, info.limit));
    }

private:
    Players* players_;
    postgres::Database* db_;
};

}  // namespace app
