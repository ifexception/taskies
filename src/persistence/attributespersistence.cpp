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

#include "attributespersistence.h"

#include "../common/logmessages.h"

#include "../common/messages/sqlitemessages.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
AttributesPersistence::AttributesPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : PersistenceBase(logger, databaseFilePath)
{
}

AttributesPersistence::~AttributesPersistence() {}

SqliteResult AttributesPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::AttributeModel>& attributeModels) const
{
    auto formatedSearchTerm = Utils::FormatSqlSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::filter.c_str(),
        static_cast<int>(AttributesPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formatedSearchTerm.c_str(),
        static_cast<int>(formatedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formatedSearchTerm.c_str(),
        static_cast<int>(formatedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::AttributeModel attributeModel;

            int columnIndex = 0;
            attributeModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            attributeModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            attributeModel.IsRequired = sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                attributeModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                attributeModel.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }
            columnIndex++;

            attributeModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);
            attributeModel.AttributeTypeId = sqlite3_column_int64(stmt, columnIndex++);

            attributeModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

            attributeModels.push_back(attributeModel);
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
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, attributeModels.size(), searchTerm);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::FilterByAttributeGroupId(const std::int64_t attributeGroupId,
    std::vector<Model::AttributeModel>& attributeModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::filterByAttributeGroupId.c_str(),
        static_cast<int>(AttributesPersistence::filterByAttributeGroupId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            AttributesPersistence::filterByAttributeGroupId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::AttributeModel attributeModel;

            int columnIndex = 0;

            attributeModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            attributeModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attributeModel.IsRequired = sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                attributeModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                attributeModel.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }

            columnIndex++;

            attributeModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);
            attributeModel.AttributeTypeId = sqlite3_column_int64(stmt, columnIndex++);

            attributeModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

            attributeModels.push_back(attributeModel);
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
        pLogger->error(LogMessages::ExecStepTemplate,
            AttributesPersistence::filterByAttributeGroupId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::EntityGetById, "attribute_group_id", attributeGroupId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::FilterByAttributeGroupIdAndIsStatic(
    const std::int64_t attributeGroupId,
    std::vector<Model::AttributeModel>& attributeModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::filterByAttributeGroupIdAndIsStatic.c_str(),
        static_cast<int>(AttributesPersistence::filterByAttributeGroupIdAndIsStatic.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            AttributesPersistence::filterByAttributeGroupIdAndIsStatic,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::AttributeModel attributeModel;

            int columnIndex = 0;

            attributeModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            attributeModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attributeModel.IsRequired = sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                attributeModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                attributeModel.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }

            columnIndex++;

            attributeModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);
            attributeModel.AttributeTypeId = sqlite3_column_int64(stmt, columnIndex++);

            attributeModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            attributeModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

            attributeModels.push_back(attributeModel);
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
        pLogger->error(LogMessages::ExecStepTemplate,
            AttributesPersistence::filterByAttributeGroupIdAndIsStatic,
            rc,
            error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessages::EntityGetById,
        "attribute_group_id && is_static = 1",
        attributeGroupId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::GetById(const std::int64_t attributeId,
    Model::AttributeModel& attributeModel) const
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::getById.c_str(),
        static_cast<int>(AttributesPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::getById, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;
    attributeModel.AttributeId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    attributeModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    attributeModel.IsRequired = sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        attributeModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        attributeModel.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;

    attributeModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);
    attributeModel.AttributeTypeId = sqlite3_column_int64(stmt, columnIndex++);

    attributeModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    attributeModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    attributeModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "attributes", attributeId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::Create(std::int64_t& attributeId,
    const Model::AttributeModel& attributeModel) const
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::create.c_str(),
        static_cast<int>(AttributesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attributeModel.Name.c_str(),
        static_cast<int>(attributeModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // is required
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.IsRequired);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "is_required", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // description
    if (attributeModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeModel.Description.value().c_str(),
            static_cast<int>(attributeModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute group id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeGroupId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute type id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeTypeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_type_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    attributeId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "attribute", attributeId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::Update(Model::AttributeModel attributeModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::update.c_str(),
        static_cast<int>(AttributesPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attributeModel.Name.c_str(),
        static_cast<int>(attributeModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // is_required
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.IsRequired);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "is_required", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // description
    if (attributeModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeModel.Description.value().c_str(),
            static_cast<int>(attributeModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute group id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeGroupId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_group_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute type id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeTypeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::BindParameterTemplate, "attribute_type_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::update, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::EntityUpdated, "attribute", attributeModel.AttributeId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::UpdateIfInUse(Model::AttributeModel attributeModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::updateIfInUse.c_str(),
        static_cast<int>(AttributesPersistence::updateIfInUse.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::updateIfInUse, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attributeModel.Name.c_str(),
        static_cast<int>(attributeModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // is_required
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.IsRequired);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "is_required", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // description
    if (attributeModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeModel.Description.value().c_str(),
            static_cast<int>(attributeModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // attribute id
    rc = sqlite3_bind_int64(stmt, bindIndex, attributeModel.AttributeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, AttributesPersistence::updateIfInUse, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::EntityUpdated, "attribute", attributeModel.AttributeId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::Delete(const std::int64_t attributeId) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::isActive.c_str(),
        static_cast<int>(AttributesPersistence::isActive.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityDeleted, "attribute", attributeId);

    return SqliteResult::OK();
}

SqliteResult AttributesPersistence::CheckAttributeUsage(const std::int64_t attributeId,
    bool& value) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::checkUsage.c_str(),
        static_cast<int>(AttributesPersistence::checkUsage.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttributesPersistence::checkUsage, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "attribute_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, AttributesPersistence::checkUsage, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    int columnIndex = 0;

    value = !!sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::StepStatementReturnedMultipleRowsMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUsage, "attribute", attributeId, value);

    return SqliteResult::OK();
}

std::string AttributesPersistence::filter = "SELECT "
                                            "attribute_id, "
                                            "name, "
                                            "is_required, "
                                            "description, "
                                            "attribute_group_id, "
                                            "attribute_type_id, "
                                            "date_created, "
                                            "date_modified, "
                                            "is_active "
                                            "FROM attributes "
                                            "WHERE is_active = 1 "
                                            "AND (name LIKE ? "
                                            "OR description LIKE ?)";

std::string AttributesPersistence::filterByAttributeGroupId = "SELECT "
                                                              "attribute_id, "
                                                              "name, "
                                                              "is_required, "
                                                              "description, "
                                                              "attribute_group_id, "
                                                              "attribute_type_id, "
                                                              "date_created, "
                                                              "date_modified, "
                                                              "is_active "
                                                              "FROM attributes "
                                                              "WHERE is_active = 1 "
                                                              "AND attribute_group_id = ?";

std::string AttributesPersistence::filterByAttributeGroupIdAndIsStatic =
    "SELECT "
    "attributes.attribute_id, "
    "attributes.name, "
    "attributes.is_required, "
    "attributes.description, "
    "attributes.attribute_group_id, "
    "attributes.attribute_type_id, "
    "attributes.date_created, "
    "attributes.date_modified, "
    "attributes.is_active "
    "FROM attributes "
    "INNER JOIN attribute_groups "
    "ON attributes.attribute_group_id = attribute_groups.attribute_group_id "
    "WHERE attributes.is_active = 1 "
    "AND attribute_groups.is_static = 1 "
    "AND attributes.attribute_group_id = ?";

std::string AttributesPersistence::getById = "SELECT "
                                             "attribute_id, "
                                             "name, "
                                             "is_required, "
                                             "description, "
                                             "attribute_group_id, "
                                             "attribute_type_id, "
                                             "date_created, "
                                             "date_modified, "
                                             "is_active "
                                             "FROM attributes "
                                             "WHERE attribute_id = ?";

std::string AttributesPersistence::create = "INSERT INTO "
                                            "attributes "
                                            "("
                                            "name, "
                                            "is_required, "
                                            "description, "
                                            "attribute_group_id, "
                                            "attribute_type_id "
                                            ") "
                                            "VALUES (?, ?, ?, ?, ?);";

std::string AttributesPersistence::update = "UPDATE attributes "
                                            "SET "
                                            "name = ?, "
                                            "is_required = ?, "
                                            "description = ?, "
                                            "attribute_group_id = ?,"
                                            "attribute_type_id = ?, "
                                            "date_modified = ? "
                                            "WHERE attribute_id = ?";

std::string AttributesPersistence::updateIfInUse = "UPDATE attributes "
                                                   "SET "
                                                   "name = ?, "
                                                   "is_required = ?, "
                                                   "description = ?, "
                                                   "date_modified = ? "
                                                   "WHERE attribute_id = ?";

std::string AttributesPersistence::isActive = "UPDATE attributes "
                                              "SET "
                                              "is_active = 0, "
                                              "date_modified = ? "
                                              "WHERE attribute_id = ?";

std::string AttributesPersistence::checkUsage =
    "SELECT "
    "CASE "
    "WHEN "
    "COUNT(*) >= 1 "
    "THEN 1 "
    "ELSE 0 "
    "END AS UsageCount "
    "FROM attributes "
    "INNER JOIN task_attribute_values "
    "ON attributes.attribute_id = task_attribute_values.attribute_id "
    "WHERE attributes.attribute_id = ? ";
} // namespace tks::Persistence
