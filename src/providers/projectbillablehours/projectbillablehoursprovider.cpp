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

#include "projectbillablehoursprovider.h"

#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

namespace tks::Services
{
ProjectBillableHoursProvider::ProjectBillableHoursProvider(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
{
}

ProjectBillableHoursProvider::~ProjectBillableHoursProvider() {}

SqliteResult ProjectBillableHoursProvider::CalculateTotalBillableHoursByProjectId(
    const std::string& monthStart,
    const std::string& monthEnd,
    const std::int64_t projectId,
    double& total)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectBillableHoursProvider::getTotalBillableHoursByProjectId.c_str(),
        static_cast<int>(ProjectBillableHoursProvider::getTotalBillableHoursByProjectId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            ProjectBillableHoursProvider::getTotalBillableHoursByProjectId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, monthStart.c_str(), static_cast<int>(monthStart.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_text(
        stmt, bindIndex, monthEnd.c_str(), static_cast<int>(monthEnd.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate,
            ProjectBillableHoursProvider::getTotalBillableHoursByProjectId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;

    total = sqlite3_column_double(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);

        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "projects", projectId);

    return SqliteResult::OK();
}

std::string ProjectBillableHoursProvider::getTotalBillableHoursByProjectId =
    "SELECT "
    "ROUND( "
    "SUM(CAST(tasks.hours AS INTEGER)) + "
    "SUM(CAST(tasks.minutes AS INTEGER)) / "
    "60.0 "
    ", 2 "
    ") AS total_billable_hours "
    "FROM tasks "
    "INNER JOIN projects "
    "ON tasks.project_id = projects.project_id "
    "INNER JOIN categories "
    "ON tasks.category_id = categories.category_id "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE projects.billable_hours > 0 "
    "AND categories.billable = 1 "
    "AND workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND projects.project_id = ? "
    "AND tasks.is_active = 1;";
} // namespace tks::Services
