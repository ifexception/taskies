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

#include "taskdurationservice.h"

#include "../../common/constants.h"
#include "../../common/logmessages.h"
#include "../../common/queryhelper.h"

#include "../../utils/utils.h"

namespace tks::Services
{
TaskDurationService::TaskDurationService(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, databaseFilePath, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Synchronous, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::TempStore, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::MmapSize, rc, error);

        return;
    }
}

TaskDurationService::~TaskDurationService()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int TaskDurationService::GetTaskDurationsForDateRange(const std::string& startDate,
    const std::string& endDate,
    TaskDurationType type,
    /*out*/ std::vector<TaskDurationViewModel>& taskDurationViewModels) const
{
    // clang-format off
    std::string sql = type == TaskDurationType::Default
        ? TaskDurationService::getAllHoursForDateRange
        : TaskDurationService::getBillableHoursForDateRange;

    std::size_t sqlSize = type == TaskDurationType::Default
        ? TaskDurationService::getAllHoursForDateRange.size()
        : TaskDurationService::getBillableHoursForDateRange.size();
    // clang-format on

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sqlSize), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, startDate.c_str(), static_cast<int>(startDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_text(
        stmt, bindIndex, endDate.c_str(), static_cast<int>(endDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            TaskDurationViewModel taskDurationViewModel;
            rc = SQLITE_ROW;

            int columnIndex = 0;

            taskDurationViewModel.Hours = sqlite3_column_int(stmt, columnIndex++);
            taskDurationViewModel.Minutes = sqlite3_column_int(stmt, columnIndex++);

            taskDurationViewModels.push_back(taskDurationViewModel);
            break;
        }
        case SQLITE_DONE:
            rc = SQLITE_DONE;
            done = true;
            break;
        default:
            break;
        }
    }

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger,
        "Retreived \"{0}\" \"tasks\" from \"{1}\" to \"{2}\"",
        taskDurationViewModels.size(),
        startDate,
        endDate);

    return 0;
}

int TaskDurationService::CalculateAndFormatDuration(const std::string& fromDate,
    const std::string& toDate,
    TaskDurationType type,
    std::string& formatDuration)
{
    std::vector<TaskDurationViewModel> taskDurations;
    int rc = GetTaskDurationsForDateRange(fromDate, toDate, type, taskDurations);
    if (rc != 0) {
        return rc;
    }

    formatDuration = CalculateTaskDurationTime(taskDurations);

    return rc;
}

std::string TaskDurationService::CalculateTaskDurationTime(
    const std::vector<TaskDurationViewModel>& taskDurations)
{
    int minutes = 0;
    int hours = 0;
    for (auto& duration : taskDurations) {
        hours += duration.Hours;
        minutes += duration.Minutes;
    }

    hours += (minutes / 60);
    minutes = minutes % 60;

    std::string formattedTotal = fmt::format("{0:02}:{1:02}", hours, minutes);
    return formattedTotal;
}

int TaskDurationService::GetTaskTimeByIdAndIncrementByValue(const std::int64_t taskId,
    const int value)
{
    TaskDurationViewModel taskDurationViewModel;
    int rc = GetTaskTimeById(taskId, taskDurationViewModel);
    if (rc != 0) {
        pLogger->error(
            "Failed to get task hours and minutes by taskId: \"{0}\". See earlier logs for detail",
            taskId);
        return rc;
    }

    IncrementTimeByValue(value, taskDurationViewModel);

    rc = UpdateTaskTime(taskId, taskDurationViewModel);
    if (rc != 0) {
        pLogger->error("Failed to update task hours and minutes with taskId: \"{0}\". See earlier "
                       "logs for detail",
            taskId);
        return rc;
    }

    return rc;
}

int TaskDurationService::GetTaskTimeById(const std::int64_t taskId,
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

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskDurationService::getTaskTimeById, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    taskDurationViewModel.Hours = sqlite3_column_int(stmt, columnIndex++);
    taskDurationViewModel.Minutes = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "tasks", taskId);

    return 0;
}

void TaskDurationService::IncrementTimeByValue(const int value,
    TaskDurationViewModel& taskDurationViewModel)
{
    int minutes = taskDurationViewModel.Minutes + value;
    if (minutes >= MAX_TASK_MINUTE_LIMIT) {
        minutes = 0;
        taskDurationViewModel.Minutes = minutes;

        int hours = taskDurationViewModel.Hours + 1;
        if (hours <= MAX_TASK_HOUR_LIMIT) {
            taskDurationViewModel.Hours = hours;
        }
    } else {
        taskDurationViewModel.Minutes = minutes;
    }
}

int TaskDurationService::UpdateTaskTime(const std::int64_t taskId,
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

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // hours
    rc = sqlite3_bind_int(stmt, bindIndex, taskDurationViewModel.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "hours", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int(stmt, bindIndex, taskDurationViewModel.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "minutes", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // task id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskDurationService::updateTaskTime, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUpdated, "task", taskId);

    return 0;
}

std::string TaskDurationService::getAllHoursForDateRange =
    "SELECT "
    "hours, "
    "minutes "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.is_active = 1";

std::string TaskDurationService::getBillableHoursForDateRange =
    "SELECT "
    "hours, "
    "minutes "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.billable = 1 "
    "AND tasks.is_active = 1";

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
