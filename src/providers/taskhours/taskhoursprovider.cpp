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

#include "taskhoursprovider.h"

#include "../../common/constants.h"
#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

#include "../../utils/sqlite_helpers.h"

namespace tks::Providers
{
TaskHoursProvider::TaskHoursProvider(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
{
}

TaskHoursProvider::~TaskHoursProvider() {}

TaskHoursProviderResult TaskHoursProvider::GetTaskHoursByDate(const std::string& date)
{
    return InternalTaskHoursQuery(sqlTaskHoursDateRange, date, date);
}

TaskHoursProviderResult TaskHoursProvider::GetBillableTaskHoursByDate(const std::string& date)
{
    return InternalTaskHoursQuery(sqlBillableTaskHoursDateRange, date, date);
}

TaskHoursProviderResult TaskHoursProvider::GetTaskHoursByWeek(const std::string& fromDate,
    const std::string& toDate)
{
    return InternalTaskHoursQuery(sqlTaskHoursDateRange, fromDate, toDate);
}

TaskHoursProviderResult TaskHoursProvider::GetBillableTaskHoursByWeek(const std::string& fromDate,
    const std::string& toDate)
{
    return InternalTaskHoursQuery(sqlBillableTaskHoursDateRange, fromDate, toDate);
}

TaskHoursProviderResult TaskHoursProvider::GetTaskHoursByMonth(const std::string& fromDate,
    const std::string& toDate)
{
    return InternalTaskHoursQuery(sqlTaskHoursDateRange, fromDate, toDate);
}

TaskHoursProviderResult TaskHoursProvider::GetBillableTaskHoursByMonth(const std::string& fromDate,
    const std::string& toDate)
{
    return InternalTaskHoursQuery(sqlBillableTaskHoursDateRange, fromDate, toDate);
}

TaskHoursProviderResult TaskHoursProvider::InternalTaskHoursQuery(const std::string& sql,
    const std::string& fromDate,
    const std::string& toDate)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return { SqliteResult::FailDetailed(Messages::PrepareStatementMessage, rc, error), "" };
    }

    auto stmt_deleter = [](sqlite3_stmt* s) { sqlite3_finalize(s); };
    std::unique_ptr<sqlite3_stmt, decltype(stmt_deleter)> stmtGuard(stmt, stmt_deleter);

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, fromDate.c_str(), static_cast<int>(fromDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        return { SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, error), "" };
    }

    bindIndex++;

    rc = sqlite3_bind_text(
        stmt, bindIndex, toDate.c_str(), static_cast<int>(toDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        return { SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, error), "" };
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        return { SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, error), "" };
    }

    int columnIndex = 0;
    std::string outValue = Utils::Sqlite::GetTextOrEmpty(stmt, columnIndex);

    SPDLOG_LOGGER_TRACE(
        pLogger, "Task hours total \"{0}\" from \"{1}\" to \"{2}\"", outValue, fromDate, toDate);

    return { SqliteResult::OK(), outValue };
}

const std::string TaskHoursProvider::sqlTaskHoursDateRange =
    "SELECT "
    "printf('%02d:%02d',"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) / 60,"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) % 60"
    ") AS total_task_hours "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.is_active = 1;";

const std::string TaskHoursProvider::sqlBillableTaskHoursDateRange =
    "SELECT "
    "printf('%02d:%02d',"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) / 60,"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) % 60"
    ") AS total_task_hours "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.billable = 1 "
    "AND tasks.is_active = 1;";
} // namespace tks::Providers
