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

#include "taskdurationservice.h"

#include "../../common/constants.h"
#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

#include "../../utils/utils.h"

namespace tks::Services
{
TaskDurationService::TaskDurationService(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
{
}

TaskDurationService::~TaskDurationService() {}

SqliteResult TaskDurationService::GetTaskTimeByIdAndIncrementByValue(const std::int64_t taskId,
    const int value)
{
    TaskDurationViewModel taskDurationViewModel;
    auto sqliteResult = GetTaskTimeById(taskId, taskDurationViewModel);
    if (!sqliteResult.Success) {
        pLogger->error(
            "Failed to get task duration by taskId: \"{0}\". See earlier logs for detail", taskId);
        return sqliteResult;
    }

    IncrementTimeByValue(value, taskDurationViewModel);

    sqliteResult = UpdateTaskTime(taskId, taskDurationViewModel);
    if (!sqliteResult.Success) {
        pLogger->error(
            "Failed to update task duration with taskId: \"{0}\". See earlier logs for detail",
            taskId);
        return sqliteResult;
    }

    return sqliteResult;
}

SqliteResult TaskDurationService::GetTaskTimeById(const std::int64_t taskId,
    TaskDurationViewModel& taskDurationViewModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskDurationService::getTaskTimeById.c_str(),
        static_cast<int>(TaskDurationService::getTaskTimeById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, TaskDurationService::getTaskTimeById, rc, error);

        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    auto stmt_deleter = [](sqlite3_stmt* s) { sqlite3_finalize(s); };
    std::unique_ptr<sqlite3_stmt, decltype(stmt_deleter)> stmtGuard(stmt, stmt_deleter);

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskDurationService::getTaskTimeById, rc, error);

        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;

    taskDurationViewModel.Hours = sqlite3_column_int(stmt, columnIndex++);
    taskDurationViewModel.Minutes = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "tasks", taskId);

    return SqliteResult::OK();
}

void TaskDurationService::IncrementTimeByValue(const int value,
    TaskDurationViewModel& taskDurationViewModel)
{
    int totalMinutes = (taskDurationViewModel.Hours * 60) + taskDurationViewModel.Minutes + value;
    if (totalMinutes < 0) {
        totalMinutes = 0;
    }

    int computedHours = totalMinutes / 60;
    int computedMinutes = totalMinutes % 60;

    if (computedHours <= 16) {
        taskDurationViewModel.Hours = computedHours;
        taskDurationViewModel.Minutes = computedMinutes;
    } else {
        taskDurationViewModel.Hours = 16;
        taskDurationViewModel.Minutes = 0;
    }
}

SqliteResult TaskDurationService::UpdateTaskTime(const std::int64_t taskId,
    TaskDurationViewModel& taskDurationViewModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskDurationService::updateTaskTime.c_str(),
        static_cast<int>(TaskDurationService::updateTaskTime.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, TaskDurationService::updateTaskTime, rc, error);

        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    auto stmt_deleter = [](sqlite3_stmt* s) { sqlite3_finalize(s); };
    std::unique_ptr<sqlite3_stmt, decltype(stmt_deleter)> stmtGuard(stmt, stmt_deleter);

    int bindIndex = 1;

    // hours
    rc = sqlite3_bind_int(stmt, bindIndex, taskDurationViewModel.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "hours", bindIndex, rc, error);

        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int(stmt, bindIndex, taskDurationViewModel.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "minutes", bindIndex, rc, error);

        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // task id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskDurationService::updateTaskTime, rc, error);

        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUpdated, "task", taskId);

    return SqliteResult::OK();
}

std::string TaskDurationService::getTaskTimeById = "SELECT "
                                                   "hours, "
                                                   "minutes "
                                                   "FROM "
                                                   "tasks "
                                                   "WHERE task_id = ?";

std::string TaskDurationService::updateTaskTime = "UPDATE tasks "
                                                  "SET "
                                                  "hours = ?, "
                                                  "minutes = ?, "
                                                  "date_modified = ? "
                                                  "WHERE task_id = ?";
} // namespace tks::Services
