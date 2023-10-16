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

#include "taskrepository.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::repos
{
TaskRepository::TaskRepository(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "TaskRepository", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "TaskRepository", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskRepository", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskRepository", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskRepository", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskRepository", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskRepository", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

TaskRepository::~TaskRepository()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "TaskRepository");
}

int TaskRepository::FilterByDateRange(std::vector<std::string> dates,
    std::map<std::string, std::vector<TaskRepositoryModel>>& models)
{
    for (const auto& date : dates) {
        pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskRepository", "task", date);

        std::vector<TaskRepositoryModel> tasks;
        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(pDb,
            TaskRepository::filterByDate.c_str(),
            static_cast<int>(TaskRepository::filterByDate.size()),
            &stmt,
            nullptr);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(
                LogMessage::PrepareStatementTemplate, "TaskRepository", TaskRepository::filterByDate, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        rc = sqlite3_bind_text(stmt, 1, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::BindParameterTemplate, "TaskRepository", "date", 1, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        bool done = false;
        while (!done) {
            switch (sqlite3_step(stmt)) {
            case SQLITE_ROW: {
                TaskRepositoryModel model;
                rc = SQLITE_ROW;
                int columnIndex = 0;

                model.TaskId = sqlite3_column_int64(stmt, columnIndex++);
                model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
                if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                    model.UniqueIdentifier = std::nullopt;
                } else {
                    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                    model.UniqueIdentifier = std::make_optional(
                        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
                }
                columnIndex++;

                model.Hours = sqlite3_column_int(stmt, columnIndex++);
                model.Minutes = sqlite3_column_int(stmt, columnIndex++);
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
                model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
                model.DateModified = sqlite3_column_int(stmt, columnIndex++);
                model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
                model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
                model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
                model.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);
                res = sqlite3_column_text(stmt, columnIndex);
                model.ProjectName =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
                res = sqlite3_column_text(stmt, columnIndex);
                model.CategoryName =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

                tasks.push_back(model);
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
            pLogger->error(LogMessage::ExecStepTemplate, "TaskRepository", TaskRepository::filterByDate, rc, err);
            sqlite3_finalize(stmt);
            return -1;
        }

        models[date] = tasks;
        sqlite3_finalize(stmt);
        pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskRepository", date);
    }

    return 0;
}

int TaskRepository::FilterByDate(const std::string& date, std::vector<TaskRepositoryModel>& tasks)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskRepository", "task", date);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskRepository::filterByDate.c_str(),
        static_cast<int>(TaskRepository::filterByDate.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskRepository", TaskRepository::filterByDate, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, date.c_str(), static_cast<int>(date.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskRepository", "date", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            TaskRepositoryModel model;
            rc = SQLITE_ROW;
            int columnIndex = 0;

            model.TaskId = sqlite3_column_int64(stmt, columnIndex++);
            model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.UniqueIdentifier = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.UniqueIdentifier = std::make_optional(
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }
            columnIndex++;

            model.Hours = sqlite3_column_int(stmt, columnIndex++);
            model.Minutes = sqlite3_column_int(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Description =
                std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
            model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
            model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
            model.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);
            res = sqlite3_column_text(stmt, columnIndex);
            model.ProjectName =
                std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            res = sqlite3_column_text(stmt, columnIndex);
            model.CategoryName =
                std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            tasks.push_back(model);
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
        pLogger->error(LogMessage::ExecStepTemplate, "TaskRepository", TaskRepository::filterByDate, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int TaskRepository::GetById(const std::int64_t taskId, TaskRepositoryModel& taskModel)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskRepository", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, TaskRepository::getById.c_str(), static_cast<int>(TaskRepository::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskRepository", TaskRepository::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskRepository", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskRepository", TaskRepository::getById, rc, err);
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
        taskModel.UniqueIdentifier = std::make_optional(
            std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;

    taskModel.Hours = sqlite3_column_int(stmt, columnIndex++);
    taskModel.Minutes = sqlite3_column_int(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    taskModel.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    taskModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    taskModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    taskModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    taskModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
    taskModel.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    taskModel.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);
    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.ProjectName = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    res = sqlite3_column_text(stmt, columnIndex);
    taskModel.CategoryName = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskRepository", taskId);

    return 0;
}

const std::string TaskRepository::filterByDate = "SELECT "
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
                                                 "projects.display_name,"
                                                 "categories.name "
                                                 "FROM tasks "
                                                 "INNER JOIN workdays "
                                                 "ON tasks.workday_id = workdays.workday_id "
                                                 "INNER JOIN projects "
                                                 "ON tasks.project_id = projects.project_id "
                                                 "INNER JOIN categories "
                                                 "ON tasks.category_id = categories.category_id "
                                                 "WHERE workdays.date = ?;";

const std::string TaskRepository::getById = "SELECT "
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
                                            "projects.display_name,"
                                            "categories.name "
                                            "FROM tasks "
                                            "INNER JOIN projects "
                                            "ON tasks.project_id = projects.project_id "
                                            "INNER JOIN categories "
                                            "ON tasks.category_id = categories.category_id "
                                            "WHERE tasks.task_id = ?;";
} // namespace tks::repos
