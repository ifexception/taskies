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

#include "attributespersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
AttributesPersistence::AttributesPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
    , mClassName("AttributesPersistence")
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, mClassName, databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::OpenDatabaseTemplate, mClassName, databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            mClassName,
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            mClassName,
            Utils::sqlite::pragmas::JournalMode,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            mClassName,
            Utils::sqlite::pragmas::Synchronous,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, mClassName, Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, mClassName, Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }

    SPDLOG_LOGGER_TRACE(pLogger, "SQLite instance initialized successfully \"{0}\"", mClassName);
}

AttributesPersistence::~AttributesPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, mClassName);
}

int AttributesPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::AttributeModel>& attributeModels)
{
    return 0;
}

int AttributesPersistence::GetById(const std::int64_t attributeId,
    Model::AttributeModel& attributeModel)
{
    return 0;
}

std::int64_t AttributesPersistence::Create(const Model::AttributeModel& attributeModel)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginCreateEntity, mClassName, "attribute", attributeModel.Name);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        AttributesPersistence::create.c_str(),
        static_cast<int>(AttributesPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            mClassName,
            AttributesPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
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
        pLogger->error(LogMessage::BindParameterTemplate, mClassName, "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is required
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.IsRequired);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, mClassName, "is_required", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
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
        pLogger->error(
            LogMessage::BindParameterTemplate, mClassName, "description", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute group id
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.AttributeGroupId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            mClassName,
            "attribute_group_id",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // attribute type id
    rc = sqlite3_bind_int(stmt, bindIndex, attributeModel.AttributeTypeId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            mClassName,
            "attribute_type_id",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, mClassName, AttributesPersistence::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, mClassName, "attribute", rowId);

    return rowId;
}

int AttributesPersistence::Update(Model::AttributeModel attributeModel)
{
    return 0;
}

int AttributesPersistence::Delete(const std::int64_t attributeId)
{
    return 0;
}

const std::string AttributesPersistence::filter = "SELECT "
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

const std::string AttributesPersistence::getById = "SELECT "
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

const std::string AttributesPersistence::create = "INSERT INTO "
                                                  "attributes "
                                                  "("
                                                  "name, "
                                                  "is_required, "
                                                  "description, "
                                                  "attribute_group_id, "
                                                  "attribute_type_id "
                                                  ") "
                                                  "VALUES (?, ?, ?, ?, ?);";

const std::string AttributesPersistence::update = "UPDATE attributes "
                                                  "SET "
                                                  "name = ?, "
                                                  "is_required = ?, "
                                                  "description = ?, "
                                                  "attribute_group_id = ?,"
                                                  "attribute_type_id = ?, "
                                                  "date_modified = ? "
                                                  "WHERE attribute_id = ?";

const std::string AttributesPersistence::isActive = "UPDATE attributes "
                                                    "SET "
                                                    "is_active = 0, "
                                                    "date_modified = ? "
                                                    "WHERE attribute_id = ?";
} // namespace tks::Persistence
