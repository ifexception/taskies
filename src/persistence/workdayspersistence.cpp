// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include "workdayspersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
WorkdaysPersistence::WorkdaysPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "WorkdaysPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "WorkdaysPersistence",
            databaseFilePath,
            rc,
            std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "WorkdaysPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "WorkdaysPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "WorkdaysPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "WorkdaysPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "WorkdaysPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);
        return;
    }
}

WorkdaysPersistence::~WorkdaysPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "WorkdaysPersistence");
}

int WorkdaysPersistence::FilterByDate(const std::string& date, Model::WorkdayModel model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "WorkdaysPersistence", "workday", date);

    GetWorkdayIdByDate(date);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::filterByDate.c_str(),
        static_cast<int>(WorkdaysPersistence::filterByDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::filterByDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "WorkdaysPersistence", "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::filterByDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Date =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "WorkdaysPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "WorkdaysPersistence", date);

    return 0;
}

std::int64_t WorkdaysPersistence::GetWorkdayIdByDate(const std::string& date)
{
    std::int64_t workdayId = 0;

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::getWorkdayIdByDate.c_str(),
        static_cast<int>(WorkdaysPersistence::getWorkdayIdByDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::getWorkdayIdByDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex++, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "WorkdaysPersistence", "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::getWorkdayIdByDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
        std::int64_t res = Create(date);

        if (res <= 0) {
            return -1;
        }

        workdayId = res;
        sqlite3_finalize(stmt);

        SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "WorkdaysPersistence", date);

        return workdayId;
    } else {
        workdayId = sqlite3_column_int64(stmt, 0);
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "WorkdaysPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "WorkdaysPersistence", date);

    return workdayId;
}

std::int64_t WorkdaysPersistence::Create(const std::string& date)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginCreateEntity, "WorkdaysPersistence", "workday", date);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::create.c_str(),
        static_cast<int>(WorkdaysPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "WorkdaysPersistence", "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "WorkdaysPersistence",
            WorkdaysPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "WorkdaysPersistence", rowId);

    return rowId;
}

const std::string WorkdaysPersistence::create = "INSERT INTO "
                                               "workdays (date) "
                                               "VALUES (?)";

const std::string WorkdaysPersistence::filterByDate = "SELECT workday_id, "
                                                     "date, "
                                                     "date_created "
                                                     "FROM workdays "
                                                     "WHERE date = ?";

const std::string WorkdaysPersistence::getWorkdayIdByDate = "SELECT workday_id "
                                                           "FROM workdays "
                                                           "WHERE date = ?";
} // namespace tks::Persistence
