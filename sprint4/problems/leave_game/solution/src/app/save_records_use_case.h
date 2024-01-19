#pragma once

#include "players.h"
#include "use_cases_impl.h"

namespace app {

class SaveRecordsUseCase {
public:
    SaveRecordsUseCase(UseCasesImpl& db_use_cases_);
    void SaveRecords(const PlayerRecordInfo& player_record);

private:
    UseCasesImpl* db_use_cases_;

};

}  // namespace app
