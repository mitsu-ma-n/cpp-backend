#include "save_records_use_case.h"

namespace app {

SaveRecordsUseCase::SaveRecordsUseCase(UseCasesImpl& db_use_cases_)
    : db_use_cases_{&db_use_cases_} {
}

void SaveRecordsUseCase::SaveRecords(const PlayerRecordInfo& player_record) {
    db_use_cases_-> (player_record);
}

}