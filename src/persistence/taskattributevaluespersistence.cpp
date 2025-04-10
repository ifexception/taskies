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

#include "taskattributevaluespersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
TaskAttributeValuesPersistence::TaskAttributeValuesPersistence(
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoOpenDatabaseConnection,
        "TaskAttributeValuesPersistence",
        databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "TaskAttributeValuesPersistence",
            databaseFilePath,
            rc,
            std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TaskAttributeValuesPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TaskAttributeValuesPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TaskAttributeValuesPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TaskAttributeValuesPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "TaskAttributeValuesPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);
        return;
    }
}

TaskAttributeValuesPersistence::~TaskAttributeValuesPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoCloseDatabaseConnection, "TaskAttributeValuesPersistence");
}

std::int64_t TaskAttributeValuesPersistence::Create(
    Model::TaskAttributeValueModel& taskAttributeValueModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginCreateEntity,
        "TaskAttributeValuesPersistence",
        "task_attribute_value",
        "<blank>");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskAttributeValuesPersistence::create.c_str(),
        static_cast<int>(TaskAttributeValuesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "TaskAttributeValuesPersistence",
            TaskAttributeValuesPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    if (taskAttributeValueModel.TextValue.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            taskAttributeValueModel.TextValue.value().c_str(),
            static_cast<int>(taskAttributeValueModel.TextValue.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TaskAttributeValuesPersistence",
            "text_value",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (taskAttributeValueModel.BooleanValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, taskAttributeValueModel.BooleanValue.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TaskAttributeValuesPersistence",
            "boolean_value",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (taskAttributeValueModel.NumericValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, taskAttributeValueModel.NumericValue.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TaskAttributeValuesPersistence",
            "numeric_value",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskAttributeValueModel.TaskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TaskAttributeValuesPersistence",
            "task_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskAttributeValueModel.AttributeId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "TaskAttributeValuesPersistence",
            "attribute_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    assert(bindIndex == 5);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "TaskAttributeValuesPersistence",
            TaskAttributeValuesPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    std::int64_t rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndCreateEntity, "TaskAttributeValuesPersistence", rowId);

    return rowId;
}

int TaskAttributeValuesPersistence::CreateMany(
    std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels)
{
    for (auto& taskAttributeValueModel : taskAttributeValueModels) {
        auto rc = Create(taskAttributeValueModel);
        if (rc < 1) {
            return -1;
        }
    }

    return 0;
}

const std::string TaskAttributeValuesPersistence::filter = "SELECT ";

const std::string TaskAttributeValuesPersistence::getByIds = "SELECT ";

const std::string TaskAttributeValuesPersistence::create = "INSERT INTO "
                                                           "task_attribute_values "
                                                           "("
                                                           "text_value, "
                                                           "boolean_value, "
                                                           "numeric_value, "
                                                           "task_id, "
                                                           "attribute_id "
                                                           ")"
                                                           " VALUES "
                                                           "(?, ?, ?, ?, ?)";

const std::string TaskAttributeValuesPersistence::update = "UPDATE ";

const std::string TaskAttributeValuesPersistence::isActive = "UPDATE ";
} // namespace tks::Persistence
