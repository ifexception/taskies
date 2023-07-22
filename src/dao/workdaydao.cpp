// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contact:
//     szymonwelgus at gmail dot com

#include "workdaydao.h"

#include "../common/constants.h"

#include "../models/workdaymodel.h"

#include "../utils/utils.h"

namespace tks::DAO
{
WorkdayDao::WorkdayDao(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "WorkdayDao", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "WorkdayDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "WorkdayDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "WorkdayDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "WorkdayDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "WorkdayDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "WorkdayDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

WorkdayDao::~WorkdayDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "WorkdayDao");
}

int WorkdayDao::GetByDate(const std::string& date, Model::WorkdayModel model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "WorkdayDao", "workday", date);

    GetWorkdayId(date);

    return 0;
}

int WorkdayDao::GetById(const std::int64_t taskId, Model::WorkdayModel model)
{
    return 0;
}

std::int64_t WorkdayDao::Create(const std::string& date)
{
    return std::int64_t();
}

std::int64_t WorkdayDao::GetWorkdayId(const std::string& date)
{
    std::int64_t workdayId = 0;

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, WorkdayDao::getWorkdayById.c_str(), static_cast<int>(WorkdayDao::getWorkdayById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "WorkdayDao", WorkdayDao::getWorkdayById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "WorkdayDao", "date", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "WorkdayDao", WorkdayDao::getWorkdayById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
        std::int64_t res = Create(date);

        if (res <= 0) {
            return -1;
        }
    } else {
        workdayId = sqlite3_column_int64(stmt, 0);
    }

    return workdayId;
}

const std::string WorkdayDao::getWorkdayId = "SELECT workday_id "
                                             "FROM workdays "
                                             "WHERE date = ?";
} // namespace tks::DAO
