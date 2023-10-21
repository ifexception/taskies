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

#include "taskdao.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::DAO
{
TaskDao::TaskDao(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "TaskDao", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "TaskDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

TaskDao::~TaskDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "TaskDao");
}

int TaskDao::GetById(const std::int64_t taskId, Model::TaskModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskDao", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, TaskDao::getById.c_str(), static_cast<int>(TaskDao::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

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
    model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
    model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    model.WorkdayId = sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskDao", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskDao", taskId);

    return 0;
}

std::int64_t TaskDao::Create(Model::TaskModel& model)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "TaskDao", "task", "");
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, TaskDao::create.c_str(), static_cast<int>(TaskDao::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "billable", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // unique identifier
    if (model.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            model.UniqueIdentifier.value().c_str(),
            static_cast<int>(model.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "unique_identifier", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Hours);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "hours", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.Minutes);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "minutes", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, model.Description.c_str(), static_cast<int>(model.Description.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "description", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.WorkdayId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "workday_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "TaskDao", rowId);

    return rowId;
}

int TaskDao::Update(Model::TaskModel& task)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "TaskDao", "task", task.TaskId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, TaskDao::update.c_str(), static_cast<int>(TaskDao::update.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, task.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "billable", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // unique identifier
    if (task.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            task.UniqueIdentifier.value().c_str(),
            static_cast<int>(task.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "unique_identifier", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.Hours);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "hours", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.Minutes);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "minutes", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, task.Description.c_str(), static_cast<int>(task.Description.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "description", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "project_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "category_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex++, task.WorkdayId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "workday_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "TaskDao", task.TaskId);

    return 0;
}

int TaskDao::Delete(const std::int64_t taskId)
{
    pLogger->info(LogMessage::InfoBeginDeleteEntity, "TaskDao", "task", taskId);
    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, TaskDao::isActive.c_str(), static_cast<int>(TaskDao::isActive.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "task_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndDeleteEntity, "TaskDao", taskId);

    return 0;
}

int TaskDao::GetDescriptionById(const std::int64_t taskId, std::string& description)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "TaskDao", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, TaskDao::getDescriptionById.c_str(), static_cast<int>(TaskDao::getDescriptionById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "TaskDao", TaskDao::getDescriptionById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, taskId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "TaskDao", "task_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "TaskDao", TaskDao::getDescriptionById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TaskDao", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "TaskDao", taskId);

    return 0;
}

const std::string TaskDao::getById = "SELECT "
                                     "task_id, "
                                     "billable, "
                                     "unique_identifier, "
                                     "hours, "
                                     "minutes, "
                                     "description, "
                                     "date_created, "
                                     "date_modified, "
                                     "is_active, "
                                     "project_id, "
                                     "category_id, "
                                     "workday_id "
                                     "FROM tasks "
                                     "WHERE task_id = ?;";

const std::string TaskDao::create = "INSERT INTO "
                                    "tasks "
                                    "("
                                    "billable, "
                                    "unique_identifier, "
                                    "hours, "
                                    "minutes, "
                                    "description, "
                                    "project_id, "
                                    "category_id, "
                                    "workday_id "
                                    ") "
                                    "VALUES (?,?,?,?,?,?,?,?)";

const std::string TaskDao::update = "UPDATE tasks "
                                    "SET "
                                    "billable = ?, "
                                    "unique_identifier = ?, "
                                    "hours = ?, "
                                    "minutes = ?, "
                                    "description = ?, "
                                    "project_id = ?, "
                                    "category_id = ?, "
                                    "workday_id = ? "
                                    "WHERE task_id = ?;";

const std::string TaskDao::isActive = "UPDATE tasks "
                                      "SET "
                                      "is_active = 0, "
                                      "date_modified = ? "
                                      "WHERE task_id = ?;";

const std::string TaskDao::getDescriptionById = "SELECT "
                                                "description "
                                                "FROM tasks "
                                                "WHERE task_id = ?;";
} // namespace tks::DAO
