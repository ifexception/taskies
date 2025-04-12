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

#include "../common/constants.h"

#include "../utils/utils.h"

// TODO: use a static_assert to check bindIndex with number of expected parameters

namespace tks::Persistence
{
TasksPersistence::TasksPersistence(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "TasksPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "TasksPersistence",
            databaseFilePath,
            rc,
            std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TasksPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TasksPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TasksPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TasksPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "TasksPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);
        return;
    }
}

TasksPersistence::~TasksPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "TasksPersistence");
}

int TasksPersistence::GetById(const std::int64_t taskId, Model::TaskModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "TasksPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::getById.c_str(),
        static_cast<int>(TasksPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::ExecStepTemplate, "TasksPersistence", TasksPersistence::getById, rc, error);

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
        model.UniqueIdentifier = std::make_optional(std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
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

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.AttributeGroupId = std::nullopt;
    } else {
        model.AttributeGroupId =
            std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }

    columnIndex++;

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TasksPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "TasksPersistence", taskId);

    return 0;
}

std::int64_t TasksPersistence::Create(Model::TaskModel& model)
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoBeginCreateEntity, "TasksPersistence", "task", "");
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::create.c_str(),
        static_cast<int>(TasksPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex, model.Billable);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "billable",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

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
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "unique_identifier",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int64(stmt, bindIndex, model.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "hours", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int64(stmt, bindIndex, model.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "minutes", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        model.Description.c_str(),
        static_cast<int>(model.Description.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "description",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex, model.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "project_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex, model.CategoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "category_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex, model.WorkdayId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "workday_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute_group_id
    if (model.AttributeGroupId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, model.AttributeGroupId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "attribute_group_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::ExecStepTemplate, "TasksPersistence", TasksPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "TasksPersistence", rowId);

    return rowId;
}

int TasksPersistence::Update(Model::TaskModel& task)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginUpdateEntity, "TasksPersistence", "task", task.TaskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::update.c_str(),
        static_cast<int>(TasksPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex, task.Billable);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "billable",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

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
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "unique_identifier",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // hours
    rc = sqlite3_bind_int(stmt, bindIndex, task.Hours);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "hours", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // minutes
    rc = sqlite3_bind_int(stmt, bindIndex, task.Minutes);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "minutes", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        task.Description.c_str(),
        static_cast<int>(task.Description.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "description",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // project id
    rc = sqlite3_bind_int64(stmt, bindIndex, task.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "project_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // category id
    rc = sqlite3_bind_int64(stmt, bindIndex, task.CategoryId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "category_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // workday id
    rc = sqlite3_bind_int64(stmt, bindIndex, task.WorkdayId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "workday_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute_group_id
    if (task.AttributeGroupId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, task.AttributeGroupId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "attribute_group_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // task id
    rc = sqlite3_bind_int64(stmt, bindIndex, task.TaskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::ExecStepTemplate, "TasksPersistence", TasksPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndUpdateEntity, "TasksPersistence", task.TaskId);

    return 0;
}

int TasksPersistence::Delete(const std::int64_t taskId)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginDeleteEntity, "TasksPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::isActive.c_str(),
        static_cast<int>(TasksPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TasksPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "TasksPersistence",
            TasksPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndDeleteEntity, "TasksPersistence", taskId);

    return 0;
}

int TasksPersistence::GetDescriptionById(const std::int64_t taskId, std::string& description)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "TasksPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::getDescriptionById.c_str(),
        static_cast<int>(TasksPersistence::getDescriptionById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
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

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "TasksPersistence",
            TasksPersistence::getDescriptionById,
            rc,
            error);

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

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TasksPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "TasksPersistence", taskId);

    return 0;
}

int TasksPersistence::IsDeleted(const std::int64_t taskId, bool& value)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "TasksPersistence", "task", taskId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TasksPersistence::isDeleted.c_str(),
        static_cast<int>(TasksPersistence::isDeleted.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TasksPersistence",
            TasksPersistence::isDeleted,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "TasksPersistence",
            TasksPersistence::isDeleted,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    value = !!sqlite3_column_int(stmt, columnIndex);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "TasksPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "TasksPersistence", taskId);

    return 0;
}

int TasksPersistence::GetTaskDurationsForDateRange(const std::string& startDate,
    const std::string& endDate,
    TaskDurationType type,
    std::vector<Model::TaskDurationModel>& models)
{
    auto paramFmt = startDate + "|" + endDate;
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "TasksPersistence", "task", paramFmt);

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

        pLogger->error(LogMessage::PrepareStatementTemplate, "TasksPersistence", sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, startDate.c_str(), static_cast<int>(startDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_text(
        stmt, bindIndex, endDate.c_str(), static_cast<int>(endDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "TasksPersistence", "date", bindIndex, rc, error);

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

        pLogger->error(LogMessage::ExecStepTemplate, "TasksPersistence", sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "TasksPersistence", paramFmt);

    return 0;
}

int TasksPersistence::GetHoursForDateRangeGroupedByDate(const std::vector<std::string>& dates,
    std::map<std::string, std::vector<Model::TaskDurationModel>>& durationsGroupedByDate)
{
    for (const auto& date : dates) {
        SPDLOG_LOGGER_TRACE(
            pLogger, LogMessage::InfoBeginGetByIdEntity, "TasksPersistence", "task", date);

        sqlite3_stmt* stmt = nullptr;
        std::vector<Model::TaskDurationModel> models;

        int rc = sqlite3_prepare_v2(pDb,
            TasksPersistence::getAllHoursForDate.c_str(),
            static_cast<int>(TasksPersistence::getAllHoursForDate.size()),
            &stmt,
            nullptr);

        if (rc != SQLITE_OK) {
            const char* error = sqlite3_errmsg(pDb);

            pLogger->error(LogMessage::PrepareStatementTemplate,
                "TasksPersistence",
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

            pLogger->error(LogMessage::BindParameterTemplate,
                "TasksPersistence",
                "date",
                bindIndex,
                rc,
                error);

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

            pLogger->error(LogMessage::ExecStepTemplate,
                "TasksPersistence",
                TasksPersistence::getAllHoursForDate,
                rc,
                error);

            sqlite3_finalize(stmt);
            return -1;
        }

        durationsGroupedByDate[date] = models;

        sqlite3_finalize(stmt);
        SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "TasksPersistence", date);
    }

    return 0;
}

const std::string TasksPersistence::getById = "SELECT "
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

const std::string TasksPersistence::create = "INSERT INTO "
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

const std::string TasksPersistence::update = "UPDATE tasks "
                                             "SET "
                                             "billable = ?, "
                                             "unique_identifier = ?, "
                                             "hours = ?, "
                                             "minutes = ?, "
                                             "description = ?, "
                                             "project_id = ?, "
                                             "category_id = ?, "
                                             "workday_id = ?, "
                                             "workday_id = ?, "
                                             "attribute_group_id = ? "
                                             "WHERE task_id = ?;";

const std::string TasksPersistence::isActive = "UPDATE tasks "
                                               "SET "
                                               "is_active = 0, "
                                               "date_modified = ? "
                                               "WHERE task_id = ?;";

const std::string TasksPersistence::getDescriptionById = "SELECT "
                                                         "description "
                                                         "FROM tasks "
                                                         "WHERE task_id = ?;";

const std::string TasksPersistence::isDeleted = "SELECT "
                                                "is_active "
                                                "FROM tasks "
                                                "WHERE task_id = ?;";

const std::string TasksPersistence::getAllHoursForDateRange =
    "SELECT "
    "hours, "
    "minutes "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.is_active = 1";

const std::string TasksPersistence::getBillableHoursForDateRange =
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

const std::string TasksPersistence::getAllHoursForDate =
    "SELECT "
    "hours, "
    "minutes "
    "FROM tasks "
    "INNER JOIN workdays "
    "ON tasks.workday_id = workdays.workday_id "
    "WHERE workdays.date = ? "
    "AND tasks.is_active = 1";
} // namespace tks::Persistence
