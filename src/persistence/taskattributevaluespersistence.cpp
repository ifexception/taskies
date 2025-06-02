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

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
TaskAttributeValuesPersistence::TaskAttributeValuesPersistence(
    std::shared_ptr<spdlog::logger> logger,
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

TaskAttributeValuesPersistence::~TaskAttributeValuesPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

std::int64_t TaskAttributeValuesPersistence::Create(
    Model::TaskAttributeValueModel& taskAttributeValueModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskAttributeValuesPersistence::create.c_str(),
        static_cast<int>(TaskAttributeValuesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
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
        pLogger->error(LogMessages::BindParameterTemplate, "text_value", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "boolean_value", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "numeric_value", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskAttributeValueModel.TaskId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, taskAttributeValueModel.AttributeId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    assert(bindIndex == 5);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskAttributeValuesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    std::int64_t rowId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "task_attribute_value", rowId);

    return rowId;
}

int TaskAttributeValuesPersistence::CreateMany(
    std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels) const
{
    for (auto& taskAttributeValueModel : taskAttributeValueModels) {
        auto rc = Create(taskAttributeValueModel);
        if (rc < 1) {
            return -1;
        }
    }

    return 0;
}

int TaskAttributeValuesPersistence::GetByTaskId(const std::int64_t taskId,
    std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        TaskAttributeValuesPersistence::getByTaskId.c_str(),
        static_cast<int>(TaskAttributeValuesPersistence::getByTaskId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            TaskAttributeValuesPersistence::getByTaskId,
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

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            Model::TaskAttributeValueModel taskAttributeValueModel;

            int columnIndex = 0;

            taskAttributeValueModel.TaskAttributeValueId =
                sqlite3_column_int64(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                taskAttributeValueModel.TextValue = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                taskAttributeValueModel.TextValue = std::make_optional<std::string>(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }

            columnIndex++;

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                taskAttributeValueModel.BooleanValue = std::nullopt;
            } else {
                taskAttributeValueModel.BooleanValue =
                    std::make_optional<bool>(sqlite3_column_int(stmt, columnIndex));
            }

            columnIndex++;

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                taskAttributeValueModel.NumericValue = std::nullopt;
            } else {
                taskAttributeValueModel.NumericValue =
                    std::make_optional<int>(sqlite3_column_int(stmt, columnIndex));
            }

            columnIndex++;

            taskAttributeValueModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            taskAttributeValueModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            taskAttributeValueModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

            taskAttributeValueModel.TaskId = sqlite3_column_int64(stmt, columnIndex++);
            taskAttributeValueModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);

            taskAttributeValueModels.push_back(taskAttributeValueModel);
            break;
        }
        case SQLITE_DONE: {
            rc = SQLITE_DONE;
            done = true;
            break;
        }
        default:
            break;
        }
    }
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, TaskAttributeValuesPersistence::getByTaskId, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "task_attribute_values", taskId);

    return 0;
}

std::string TaskAttributeValuesPersistence::getByTaskId = "SELECT "
                                                          "task_attribute_value_id, "
                                                          "text_value, "
                                                          "boolean_value, "
                                                          "numeric_value, "
                                                          "date_created, "
                                                          "date_modified, "
                                                          "is_active, "
                                                          "task_id, "
                                                          "attribute_id "
                                                          "FROM task_attribute_values "
                                                          "WHERE task_id = ? "
                                                          "AND is_active = 1";

std::string TaskAttributeValuesPersistence::create = "INSERT INTO "
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
} // namespace tks::Persistence
