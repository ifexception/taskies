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

#include "attributegroupspersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
AttributeGroupsPersistence::AttributeGroupsPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoOpenDatabaseConnection,
        "AttributeGroupsPersistence",
        databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "AttributeGroupsPersistence",
            databaseFilePath,
            rc,
            std::string(error));

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);

        return;
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "\"{0}\" - SQLite instance initialized successfully",
        "AttributeGroupsPersistence");
}

AttributeGroupsPersistence::~AttributeGroupsPersistence()
{
    sqlite3_close(pDb);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoCloseDatabaseConnection, "AttributeGroupsPersistence");
}

int AttributeGroupsPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::AttributeGroupModel>& attributeGroupModels)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "AttributeGroupsPersistence",
        "attribute groups",
        searchTerm);

    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::filter.c_str(),
        static_cast<int>(AttributeGroupsPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
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

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
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

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "description",
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
            rc = SQLITE_ROW;
            Model::AttributeGroupModel model;

            int columnIndex = 0;
            model.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            model.IsStaticGroup = sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }

            columnIndex++;

            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            attributeGroupModels.push_back(model);
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
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "AttributeGroupsPersistence",
        attributeGroupModels.size(),
        searchTerm);

    return 0;
}

int AttributeGroupsPersistence::GetById(const std::int64_t attributeGroupId,
    Model::AttributeGroupModel& attributeGroupModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginGetByIdEntity,
        "AttributeGroupsPersistence",
        "attribute group",
        attributeGroupId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::getById.c_str(),
        static_cast<int>(AttributeGroupsPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "attribute_group_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    attributeGroupModel.AttributeGroupId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    attributeGroupModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    attributeGroupModel.IsStaticGroup = sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        attributeGroupModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        attributeGroupModel.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }

    columnIndex++;

    attributeGroupModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    attributeGroupModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    attributeGroupModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate,
            "AttributeGroupsPersistence",
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndGetByIdEntity, "AttributeGroupsPersistence", attributeGroupId);

    return 0;
}

std::int64_t AttributeGroupsPersistence::Create(
    const Model::AttributeGroupModel& attributeGroupModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginCreateEntity,
        "AttributeGroupsPersistence",
        "attribute group",
        attributeGroupModel.Name);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::create.c_str(),
        static_cast<int>(AttributeGroupsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attributeGroupModel.Name.c_str(),
        static_cast<int>(attributeGroupModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is_static_group
    rc = sqlite3_bind_int(stmt, bindIndex, attributeGroupModel.IsStaticGroup);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "is_static_group",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (attributeGroupModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeGroupModel.Description.value().c_str(),
            static_cast<int>(attributeGroupModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "description",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndCreateEntity, "AttributeGroupsPersistence", rowId);

    return rowId;
}

int AttributeGroupsPersistence::Update(Model::AttributeGroupModel attributeGroupModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginUpdateEntity,
        "AttributeGroupsPersistence",
        "attribute group",
        attributeGroupModel.AttributeGroupId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::update.c_str(),
        static_cast<int>(AttributeGroupsPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attributeGroupModel.Name.c_str(),
        static_cast<int>(attributeGroupModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is_static_group
    rc = sqlite3_bind_int(stmt, bindIndex, attributeGroupModel.IsStaticGroup);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "is_static_group",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (attributeGroupModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeGroupModel.Description.value().c_str(),
            static_cast<int>(attributeGroupModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "description",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupModel.AttributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
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

        pLogger->error(LogMessage::ExecStepTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndUpdateEntity,
        "AttributeGroupsPersistence",
        attributeGroupModel.AttributeGroupId);

    return 0;
}

int AttributeGroupsPersistence::Delete(const std::int64_t attributeGroupId)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginDeleteEntity,
        "AttributeGroupsPersistence",
        "attribute group",
        attributeGroupId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::isActive.c_str(),
        static_cast<int>(AttributeGroupsPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::isActive,
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
            "AttributeGroupsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, attributeGroupId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
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

        pLogger->error(LogMessage::ExecStepTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndDeleteEntity, "AttributeGroupsPersistence", attributeGroupId);

    return 0;
}

const std::string AttributeGroupsPersistence::filter = "SELECT "
                                                       "attribute_group_id, "
                                                       "name, "
                                                       "description, "
                                                       "date_created, "
                                                       "is_static_group, "
                                                       "date_modified, "
                                                       "is_active "
                                                       "FROM attribute_groups "
                                                       "WHERE is_active = 1 "
                                                       "AND (name LIKE ? "
                                                       "OR description LIKE ?)";

const std::string AttributeGroupsPersistence::getById = "SELECT "
                                                        "attribute_group_id, "
                                                        "name, "
                                                        "description, "
                                                        "is_static_group, "
                                                        "date_created, "
                                                        "date_modified, "
                                                        "is_active "
                                                        "FROM attribute_groups "
                                                        "WHERE attribute_group_id = ?";

const std::string AttributeGroupsPersistence::create = "INSERT INTO "
                                                       "attribute_groups "
                                                       "("
                                                       "name, "
                                                       "description, "
                                                       "is_static_group "
                                                       ") "
                                                       "VALUES (?,?,?);";

const std::string AttributeGroupsPersistence::update = "UPDATE attribute_groups "
                                                       "SET "
                                                       "name = ?, "
                                                       "description = ?, "
                                                       "is_static_group = ?, "
                                                       "date_modified = ? "
                                                       "WHERE attribute_group_id = ?";

const std::string AttributeGroupsPersistence::isActive = "UPDATE attribute_groups "
                                                         "SET "
                                                         "is_active = 0, "
                                                         "date_modified = ? "
                                                         "WHERE attribute_group_id = ?";
} // namespace tks::Persistence
