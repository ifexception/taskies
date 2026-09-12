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
    pTaskHoursSummary = std::make_unique<TaskHoursSummary>();
}

TaskHoursProvider::~TaskHoursProvider() {}

SqliteResult TaskHoursProvider::GetTaskHoursByDate(const std::string& date)
{
    auto result = InternalTaskHoursByDateRange(
        getTasksHoursByDateRange, date, date, pTaskHoursSummary->DayTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::GetBillableTaskHoursByDate(const std::string& date)
{
    auto result = InternalTaskHoursByDateRange(
        getBillableTasksHoursByDateRange, date, date, pTaskHoursSummary->DayBillableTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::GetTaskHoursByWeek(const std::string& fromDate,
    const std::string& toDate,
    std::string value)
{
    auto result = InternalTaskHoursByDateRange(
        getTasksHoursByDateRange, fromDate, toDate, pTaskHoursSummary->WeekTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::GetBillableTaskHoursByWeek(const std::string& fromDate,
    const std::string& toDate,
    std::string value)
{
    auto result = InternalTaskHoursByDateRange(getBillableTasksHoursByDateRange,
        fromDate,
        toDate,
        pTaskHoursSummary->WeekBillableTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::GetTaskHoursByMonth(const std::string& fromDate,
    const std::string& toDate)
{
    auto result = InternalTaskHoursByDateRange(
        getTasksHoursByDateRange, fromDate, toDate, pTaskHoursSummary->MonthTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::GetBillableTaskHoursByMonth(const std::string& fromDate,
    const std::string& toDate)
{
    auto result = InternalTaskHoursByDateRange(getBillableTasksHoursByDateRange,
        fromDate,
        toDate,
        pTaskHoursSummary->MonthBillableTaskHours);
    return result;
}

SqliteResult TaskHoursProvider::InternalTaskHoursByDateRange(const std::string& sql,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& value)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, fromDate.c_str(), static_cast<int>(fromDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_text(
        stmt, bindIndex, toDate.c_str(), static_cast<int>(toDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;
    value = Utils::Sqlite::GetTextOrEmpty(stmt, columnIndex);

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, "Task hours total \"{0}\" from \"{1}\" to \"{2}\"", value, fromDate, toDate);

    return SqliteResult::OK();
}

std::string TaskHoursProvider::getTasksHoursByDateRange =
    "SELECT "
    "printf('%02d:%02d',"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) / 60,"
    "(SUM(CAST(tasks.hours AS INTEGER)) * 60 + SUM(CAST(tasks.minutes AS INTEGER))) % 60"
    ") AS total_task_hours "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date = ? "
    "AND tasks.is_active = 1;";

std::string TaskHoursProvider::getBillableTasksHoursByDateRange =
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
