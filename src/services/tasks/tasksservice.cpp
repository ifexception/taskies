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

#include "tasksservice.h"

#include "../../common/logmessages.h"
#include "../../common/queryhelper.h"

#include "../../utils/utils.h"

namespace tks::Services
{
TasksService::TasksService(const std::shared_ptr<spdlog::logger> logger,
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

TasksService::~TasksService()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int TasksService::FilterByDateRange(std::vector<std::string> dates,
    std::map<std::string, std::vector<TaskViewModel>>& taskViewModelsMap) const
{
    for (const auto& date : dates) {
        std::vector<TaskViewModel> tasks;
        int rc = FilterByDate(date, tasks);
        if (rc != 0) {
            return rc;
        }

        taskViewModelsMap[date] = tasks;
    }

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::FilterEntities, taskViewModelsMap.size(), "[date range]");

    return 0;
}

int TasksService::FilterByDate(const std::string& date,
    std::vector<TaskViewModel>& taskViewModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksService::filterByDate.c_str(),
        static_cast<int>(TasksService::filterByDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, TasksService::filterByDate, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date", 1, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            TaskViewModel taskViewModel;

            int columnIndex = 0;

            taskViewModel.TaskId = sqlite3_column_int64(stmt, columnIndex++);

            taskViewModel.Billable = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                taskViewModel.UniqueIdentifier = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                taskViewModel.UniqueIdentifier = std::make_optional(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }
            columnIndex++;

            taskViewModel.Hours = sqlite3_column_int(stmt, columnIndex++);
            taskViewModel.Minutes = sqlite3_column_int(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            taskViewModel.Description = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            taskViewModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            taskViewModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            taskViewModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            taskViewModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
            taskViewModel.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
            taskViewModel.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);

            res = sqlite3_column_text(stmt, columnIndex);
            taskViewModel.ProjectName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            taskViewModel.ProjectDisplayName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            taskViewModel.CategoryName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                taskViewModel.ClientName = std::nullopt;
            } else {
                res = sqlite3_column_text(stmt, columnIndex);
                taskViewModel.ClientName = std::make_optional(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }
            columnIndex++;

            res = sqlite3_column_text(stmt, columnIndex);
            taskViewModel.EmployerName = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            taskViewModels.push_back(taskViewModel);
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, TasksService::filterByDate, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, taskViewModels.size(), "");
    return 0;
}

int TasksService::GetById(const std::int64_t taskId, TaskViewModel& taskModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksService::getById.c_str(),
        static_cast<int>(TasksService::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, TasksService::getById, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", 1, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, TasksService::getById, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    taskModel.TaskId = sqlite3_column_int64(stmt, columnIndex++);

    taskModel.Billable = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        taskModel.UniqueIdentifier = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        taskModel.UniqueIdentifier = std::make_optional(std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;

    taskModel.Hours = sqlite3_column_int(stmt, columnIndex++);
    taskModel.Minutes = sqlite3_column_int(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    taskModel.Description =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    taskModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    taskModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    taskModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

    taskModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
    taskModel.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    taskModel.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);

    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.ProjectName =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.ProjectDisplayName =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.CategoryName =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        taskModel.ClientName = std::nullopt;
    } else {
        res = sqlite3_column_text(stmt, columnIndex);
        taskModel.ClientName = std::make_optional(std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;

    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.EmployerName =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "tasks", taskId);

    return 0;
}

std::string TasksService::filterByDate = "SELECT "
                                         "tasks.task_id, "
                                         "tasks.billable, "
                                         "tasks.unique_identifier, "
                                         "tasks.hours, "
                                         "tasks.minutes, "
                                         "tasks.description, "
                                         "tasks.date_created, "
                                         "tasks.date_modified, "
                                         "tasks.is_active, "
                                         "tasks.project_id, "
                                         "tasks.category_id, "
                                         "tasks.workday_id, "
                                         "projects.name,"
                                         "projects.display_name,"
                                         "categories.name, "
                                         "clients.name, "
                                         "employers.name "
                                         "FROM tasks "
                                         "INNER JOIN workdays "
                                         "ON tasks.workday_id = workdays.workday_id "
                                         "INNER JOIN projects "
                                         "ON tasks.project_id = projects.project_id "
                                         "INNER JOIN categories "
                                         "ON tasks.category_id = categories.category_id "
                                         "LEFT JOIN clients "
                                         "ON projects.client_id = clients.client_id "
                                         "INNER JOIN employers "
                                         "ON projects.employer_id = employers.employer_id "
                                         "WHERE workdays.date = ? "
                                         "AND tasks.is_active = 1;";

std::string TasksService::getById = "SELECT "
                                    "tasks.task_id, "
                                    "tasks.billable, "
                                    "tasks.unique_identifier, "
                                    "tasks.hours, "
                                    "tasks.minutes, "
                                    "tasks.description, "
                                    "tasks.date_created, "
                                    "tasks.date_modified, "
                                    "tasks.is_active, "
                                    "tasks.project_id, "
                                    "tasks.category_id, "
                                    "tasks.workday_id, "
                                    "projects.name,"
                                    "projects.display_name,"
                                    "categories.name, "
                                    "clients.name, "
                                    "employers.name "
                                    "FROM tasks "
                                    "INNER JOIN projects "
                                    "ON tasks.project_id = projects.project_id "
                                    "INNER JOIN categories "
                                    "ON tasks.category_id = categories.category_id "
                                    "LEFT JOIN clients "
                                    "ON projects.client_id = clients.client_id "
                                    "INNER JOIN employers "
                                    "ON projects.employer_id = employers.employer_id "
                                    "WHERE tasks.task_id = ? "
                                    "AND tasks.is_active = 1;";
} // namespace tks::Services
