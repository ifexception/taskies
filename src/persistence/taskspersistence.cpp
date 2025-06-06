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

#include "taskspersistence.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
TasksPersistence::TasksPersistence(const std::shared_ptr<spdlog::logger> logger,
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

TasksPersistence::~TasksPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int TasksPersistence::GetById(const std::int64_t taskId, Model::TaskModel& taskModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::getById.c_str(),
        static_cast<int>(TasksPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, TasksPersistence::getById, rc, error);

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
        pLogger->error(LogMessages::ExecStepTemplate, TasksPersistence::getById, rc, error);

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

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        taskModel.AttributeGroupId = std::nullopt;
    } else {
        taskModel.AttributeGroupId =
            std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }

    columnIndex++;

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

std::int64_t TasksPersistence::Create(Model::TaskModel& taskModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::create.c_str(),
        static_cast<int>(TasksPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, TasksPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex, taskModel.Billable);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "billable", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // unique identifier
    if (taskModel.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            taskModel.UniqueIdentifier.value().c_str(),
            static_cast<int>(taskModel.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "unique_identifier", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "hours", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "minutes", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        taskModel.Description.c_str(),
        static_cast<int>(taskModel.Description.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.CategoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.WorkdayId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "workday_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute_group_id
    if (taskModel.AttributeGroupId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.AttributeGroupId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, TasksPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "task", rowId);

    return rowId;
}

int TasksPersistence::Update(Model::TaskModel& taskModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::update.c_str(),
        static_cast<int>(TasksPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, TasksPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex, taskModel.Billable);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "billable", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // unique identifier
    if (taskModel.UniqueIdentifier.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            taskModel.UniqueIdentifier.value().c_str(),
            static_cast<int>(taskModel.UniqueIdentifier.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "unique_identifier", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int(stmt, bindIndex, taskModel.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "hours", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int(stmt, bindIndex, taskModel.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "minutes", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        taskModel.Description.c_str(),
        static_cast<int>(taskModel.Description.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.CategoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "category_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.WorkdayId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "workday_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute_group_id
    if (taskModel.AttributeGroupId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.AttributeGroupId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

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
    rc = sqlite3_bind_int64(stmt, bindIndex, taskModel.TaskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    assert(bindIndex == 11);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, TasksPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUpdated, "task", taskModel.TaskId);

    return 0;
}

int TasksPersistence::Delete(const std::int64_t taskId)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::isActive.c_str(),
        static_cast<int>(TasksPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, TasksPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

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
        pLogger->error(LogMessages::ExecStepTemplate, TasksPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityDeleted, "task", taskId);

    return 0;
}

int TasksPersistence::GetDescriptionById(const std::int64_t taskId, std::string& description) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::getDescriptionById.c_str(),
        static_cast<int>(TasksPersistence::getDescriptionById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::getDescriptionById,
            rc,
            error);

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
            LogMessages::ExecStepTemplate, TasksPersistence::getDescriptionById, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    description =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "task", taskId);

    return 0;
}

int TasksPersistence::IsDeleted(const std::int64_t taskId, bool& value)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::isDeleted.c_str(),
        static_cast<int>(TasksPersistence::isDeleted.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, TasksPersistence::isDeleted, rc, error);

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
        pLogger->error(LogMessages::ExecStepTemplate, TasksPersistence::isDeleted, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    value = !!sqlite3_column_int(stmt, columnIndex);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, "Checked if task \"{0}\" is deleted", taskId);

    return 0;
}

int TasksPersistence::GetTaskDurationsForDateRange(const std::string& startDate,
    const std::string& endDate,
    TaskDurationType type,
    std::vector<Model::TaskDurationModel>& models) const
{
    // clang-format off
    std::string sql = type == TaskDurationType::Default
        ? TasksPersistence::getAllHoursForDateRange
        : TasksPersistence::getBillableHoursForDateRange;

    std::size_t sqlSize = type == TaskDurationType::Default
        ? TasksPersistence::getAllHoursForDateRange.size()
        : TasksPersistence::getBillableHoursForDateRange.size();
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
            Model::TaskDurationModel model;
            rc = SQLITE_ROW;

            int columnIndex = 0;

            model.Hours = sqlite3_column_int(stmt, columnIndex++);
            model.Minutes = sqlite3_column_int(stmt, columnIndex++);

            models.push_back(model);
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
        models.size(),
        startDate,
        endDate);

    return 0;
}

int TasksPersistence::GetHoursForDateRangeGroupedByDate(const std::vector<std::string>& dates,
    std::map<std::string, std::vector<Model::TaskDurationModel>>& durationsGroupedByDate) const
{
    for (const auto& date : dates) {
        sqlite3_stmt* stmt = nullptr;
        std::vector<Model::TaskDurationModel> models;

        int rc = sqlite3_prepare_v2(pDb,
            TasksPersistence::getAllHoursForDate.c_str(),
            static_cast<int>(TasksPersistence::getAllHoursForDate.size()),
            &stmt,
            nullptr);

        if (rc != SQLITE_OK) {
            const char* error = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::PrepareStatementTemplate,
                TasksPersistence::getAllHoursForDate,
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
            pLogger->error(LogMessages::BindParameterTemplate, "date", bindIndex, rc, error);

            sqlite3_finalize(stmt);
            return -1;
        }

        bool done = false;
        while (!done) {
            switch (sqlite3_step(stmt)) {
            case SQLITE_ROW: {
                Model::TaskDurationModel model{};
                rc = SQLITE_ROW;

                int columnIndex = 0;

                model.Hours = sqlite3_column_int(stmt, columnIndex++);
                model.Minutes = sqlite3_column_int(stmt, columnIndex++);

                models.push_back(model);
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
            pLogger->error(
                LogMessages::ExecStepTemplate, TasksPersistence::getAllHoursForDate, rc, error);

            sqlite3_finalize(stmt);
            return -1;
        }

        durationsGroupedByDate[date] = models;

        sqlite3_finalize(stmt);
        SPDLOG_LOGGER_TRACE(pLogger, "Retreived \"tasks\" grouped by date \"{0}\"", date);
    }

    return 0;
}

std::string TasksPersistence::getById = "SELECT "
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
                                        "workday_id, "
                                        "attribute_group_id "
                                        "FROM tasks "
                                        "WHERE task_id = ?;";

std::string TasksPersistence::create = "INSERT INTO "
                                       "tasks "
                                       "("
                                       "billable, "
                                       "unique_identifier, "
                                       "hours, "
                                       "minutes, "
                                       "description, "
                                       "project_id, "
                                       "category_id, "
                                       "workday_id, "
                                       "attribute_group_id "
                                       ") "
                                       "VALUES (?,?,?,?,?,?,?,?,?)";

std::string TasksPersistence::update = "UPDATE tasks "
                                       "SET "
                                       "billable = ?, "
                                       "unique_identifier = ?, "
                                       "hours = ?, "
                                       "minutes = ?, "
                                       "description = ?, "
                                       "project_id = ?, "
                                       "category_id = ?, "
                                       "workday_id = ?, "
                                       "attribute_group_id = ?, "
                                       "date_modified = ? "
                                       "WHERE task_id = ?";

std::string TasksPersistence::isActive = "UPDATE tasks "
                                         "SET "
                                         "is_active = 0, "
                                         "date_modified = ? "
                                         "WHERE task_id = ?;";

std::string TasksPersistence::getDescriptionById = "SELECT "
                                                   "description "
                                                   "FROM tasks "
                                                   "WHERE task_id = ?;";

std::string TasksPersistence::isDeleted = "SELECT "
                                          "is_active "
                                          "FROM tasks "
                                          "WHERE task_id = ?;";

std::string TasksPersistence::getAllHoursForDateRange = "SELECT "
                                                        "hours, "
                                                        "minutes "
                                                        "FROM tasks "
                                                        "INNER JOIN workdays "
                                                        "ON tasks.workday_id = workdays.workday_id "
                                                        "WHERE workdays.date >= ? "
                                                        "AND workdays.date <= ? "
                                                        "AND tasks.is_active = 1";

std::string TasksPersistence::getBillableHoursForDateRange =
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

std::string TasksPersistence::getAllHoursForDate = "SELECT "
                                                   "hours, "
                                                   "minutes "
                                                   "FROM tasks "
                                                   "INNER JOIN workdays "
                                                   "ON tasks.workday_id = workdays.workday_id "
                                                   "WHERE workdays.date = ? "
                                                   "AND tasks.is_active = 1";
} // namespace tks::Persistence
