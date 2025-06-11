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

#include "staticattributevaluespersistence.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
StaticAttributeValuesPersistence::StaticAttributeValuesPersistence(
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

StaticAttributeValuesPersistence::~StaticAttributeValuesPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

std::int64_t StaticAttributeValuesPersistence::Create(
    const Model::StaticAttributeValueModel& staticAttributeValueModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        StaticAttributeValuesPersistence::create.c_str(),
        static_cast<int>(StaticAttributeValuesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            StaticAttributeValuesPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    if (staticAttributeValueModel.TextValue.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            staticAttributeValueModel.TextValue.value().c_str(),
            static_cast<int>(staticAttributeValueModel.TextValue.value().size()),
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

    if (staticAttributeValueModel.BooleanValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, staticAttributeValueModel.BooleanValue.value());
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

    if (staticAttributeValueModel.NumericValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, staticAttributeValueModel.NumericValue.value());
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

    rc = sqlite3_bind_int64(stmt, bindIndex, staticAttributeValueModel.AttributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, staticAttributeValueModel.AttributeId);

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
            LogMessages::ExecStepTemplate, StaticAttributeValuesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    std::int64_t rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "static_attribute_value", rowId);

    return rowId;
}

int StaticAttributeValuesPersistence::CreateMultiple(
    const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const
{
    for (const auto& staticAttributeValueModel : staticAttributeValueModels) {
        std::int64_t rc = Create(staticAttributeValueModel);
        if (rc < 1) {
            return -1;
        }
    }

    return 0;
}

int StaticAttributeValuesPersistence::FilterByAttributeGroupId(const std::int64_t attributeGroupId,
    std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        StaticAttributeValuesPersistence::filterByAttributeGroupId.c_str(),
        static_cast<int>(StaticAttributeValuesPersistence::filterByAttributeGroupId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            StaticAttributeValuesPersistence::filterByAttributeGroupId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            Model::StaticAttributeValueModel staticAttributeValueModel;

            int columnIndex = 0;

            staticAttributeValueModel.StaticAttributeValueId =
                sqlite3_column_int64(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                staticAttributeValueModel.TextValue = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                staticAttributeValueModel.TextValue = std::make_optional<std::string>(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }

            columnIndex++;

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                staticAttributeValueModel.BooleanValue = std::nullopt;
            } else {
                staticAttributeValueModel.BooleanValue =
                    std::make_optional<bool>(sqlite3_column_int(stmt, columnIndex));
            }

            columnIndex++;

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                staticAttributeValueModel.NumericValue = std::nullopt;
            } else {
                staticAttributeValueModel.NumericValue =
                    std::make_optional<int>(sqlite3_column_int(stmt, columnIndex));
            }

            columnIndex++;

            staticAttributeValueModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            staticAttributeValueModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            staticAttributeValueModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

            staticAttributeValueModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);
            staticAttributeValueModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);

            staticAttributeValueModels.push_back(staticAttributeValueModel);
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
        pLogger->error(LogMessages::ExecStepTemplate,
            StaticAttributeValuesPersistence::filterByAttributeGroupId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::EntityGetById, "static_attribute_values", attributeGroupId);

    return 0;
}

int StaticAttributeValuesPersistence::Update(
    const Model::StaticAttributeValueModel& staticAttributeValueModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        StaticAttributeValuesPersistence::update.c_str(),
        static_cast<int>(StaticAttributeValuesPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            StaticAttributeValuesPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    if (staticAttributeValueModel.TextValue.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            staticAttributeValueModel.TextValue.value().c_str(),
            static_cast<int>(staticAttributeValueModel.TextValue.value().size()),
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

    if (staticAttributeValueModel.BooleanValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, staticAttributeValueModel.BooleanValue.value());
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

    if (staticAttributeValueModel.NumericValue.has_value()) {
        rc = sqlite3_bind_int(stmt, bindIndex, staticAttributeValueModel.NumericValue.value());
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

    rc = sqlite3_bind_int64(stmt, bindIndex, staticAttributeValueModel.AttributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, staticAttributeValueModel.AttributeId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, staticAttributeValueModel.StaticAttributeValueId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "static_attribute_value_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    assert(bindIndex == 7);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, StaticAttributeValuesPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int StaticAttributeValuesPersistence::UpdateMultiple(
    const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const
{
    for (const auto& staticAttributeValueModel : staticAttributeValueModels) {
        int rc = Update(staticAttributeValueModel);
        if (rc != 0) {
            return -1;
        }
    }

    return 0;
}

int StaticAttributeValuesPersistence::Delete(
    const std::vector<std::int64_t>& staticAttributeValueIds) const
{
    std::string csvIds = Utils::ConvertListIdsToCommaDelimitedString(staticAttributeValueIds);
    std::string sql = StaticAttributeValuesPersistence::isActive + "(" + csvIds + ")";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

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

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityDeleted, "static_attribute_values", csvIds);

    return 0;
}

std::string StaticAttributeValuesPersistence::create = "INSERT INTO "
                                                       "static_attribute_values "
                                                       "("
                                                       "text_value, "
                                                       "boolean_value, "
                                                       "numeric_value, "
                                                       "attribute_group_id, "
                                                       "attribute_id "
                                                       ")"
                                                       " VALUES "
                                                       "(?, ?, ?, ?, ?)";

std::string StaticAttributeValuesPersistence::filterByAttributeGroupId =
    "SELECT "
    "static_attribute_value_id, "
    "text_value, "
    "boolean_value, "
    "numeric_value, "
    "date_created, "
    "date_modified, "
    "is_active, "
    "attribute_group_id, "
    "attribute_id "
    "FROM static_attribute_values "
    "WHERE is_active = 1 "
    "AND attribute_group_id = ?";

std::string StaticAttributeValuesPersistence::update = "UPDATE static_attribute_values "
                                                       "SET "
                                                       "text_value = ?, "
                                                       "boolean_value = ?, "
                                                       "numeric_value = ?, "
                                                       "attribute_group_id = ?, "
                                                       "attribute_id = ?, "
                                                       "date_modified = ? "
                                                       "WHERE static_attribute_value_id = ? ";

std::string StaticAttributeValuesPersistence::isActive = "UPDATE static_attribute_values "
                                                         "SET date_modified = ? "
                                                         "WHERE static_attribute_value_id IN ";
} // namespace tks::Persistence
