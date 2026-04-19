// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "../common/logmessages.h"

#include "../common/messages/sqlitemessages.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
WorkdaysPersistence::WorkdaysPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
    , pLogger(logger)
{
}

SqliteResult WorkdaysPersistence::FilterByDate(const std::string& date,
    Model::WorkdayModel model) const
{
    std::int64_t workdayId = -1;
    auto sqliteResult = GetWorkdayIdByDate(workdayId, date);
    if (!sqliteResult.Success) {
        pLogger->warn(
            "An error occured getting workday ID by date \"{0}\". See earlier logs for detail",
            date);
        return sqliteResult;
    }

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::filterByDate.c_str(),
        static_cast<int>(WorkdaysPersistence::filterByDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, WorkdaysPersistence::filterByDate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, WorkdaysPersistence::filterByDate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::BindStatementMessage, rc, std::string(error));
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
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "workday", date);

    return SqliteResult::OK();
}

SqliteResult WorkdaysPersistence::GetWorkdayIdByDate(std::int64_t& workdayId,
    const std::string& date) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::getWorkdayIdByDate.c_str(),
        static_cast<int>(WorkdaysPersistence::getWorkdayIdByDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            WorkdaysPersistence::getWorkdayIdByDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex++, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, WorkdaysPersistence::getWorkdayIdByDate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementMessage, rc, std::string(error));
    }

    if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
        std::int64_t workdayId = -1;
        auto sqliteResult = Create(workdayId, date);

        if (!sqliteResult.Success) {
            return sqliteResult;
        }

        sqlite3_finalize(stmt);
        SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "workday", date);

        return SqliteResult::OK();
    } else {
        workdayId = sqlite3_column_int64(stmt, 0);
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "workdays", date);

    return SqliteResult::OK();
}

SqliteResult WorkdaysPersistence::Create(std::int64_t& workdayId,
    const std::string& date) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        WorkdaysPersistence::create.c_str(),
        static_cast<int>(WorkdaysPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, WorkdaysPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, WorkdaysPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::BindStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);

    workdayId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "workday", workdayId);

    return SqliteResult::OK();
}

std::string WorkdaysPersistence::create = "INSERT INTO "
                                          "workdays (date) "
                                          "VALUES (?)";

std::string WorkdaysPersistence::filterByDate = "SELECT workday_id, "
                                                "date, "
                                                "date_created "
                                                "FROM workdays "
                                                "WHERE date = ?";

std::string WorkdaysPersistence::getWorkdayIdByDate = "SELECT workday_id "
                                                      "FROM workdays "
                                                      "WHERE date = ?";
} // namespace tks::Persistence
