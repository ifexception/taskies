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
        LogMessage::InfoOpenDatabaseConnection, "AttributeGroupsPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "AttributeGroupsPersistence",
            databaseFilePath,
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeGroupsPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            err);
        return;
    }

    SPDLOG_LOGGER_TRACE(
        pLogger, "SQLite instance initialized successfully \"{0}\"", "AttributeGroupsPersistence");
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
    return 0;
}

int AttributeGroupsPersistence::GetById(const std::int64_t employerId,
    Model::AttributeGroupModel& attributeGroupModel)
{
    return 0;
}

std::int64_t AttributeGroupsPersistence::Create(
    const Model::AttributeGroupModel& attributeGroupModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginCreateEntity,
        "AttributeGroupsPersistence",
        "Attribute Group",
        attributeGroupModel.Name);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        AttributeGroupsPersistence::create.c_str(),
        static_cast<int>(AttributeGroupsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::create,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex++,
        attributeGroupModel.Name.c_str(),
        static_cast<int>(attributeGroupModel.Name.size()),
        SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "AttributeGroupsPersistence", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (attributeGroupModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            attributeGroupModel.Description.value().c_str(),
            static_cast<int>(attributeGroupModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeGroupsPersistence",
            "description",
            3,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate,
            "AttributeGroupsPersistence",
            AttributeGroupsPersistence::create,
            rc,
            err);
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
    return 0;
}

int AttributeGroupsPersistence::Delete(const std::int64_t attributeGroupId)
{
    return 0;
}

const std::string AttributeGroupsPersistence::filter = "SELECT "
                                                       "attribute_group_id, "
                                                       "name, "
                                                       "description, "
                                                       "date_created, "
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
                                                        "date_created, "
                                                        "date_modified, "
                                                        "is_active "
                                                        "FROM attribute_groups "
                                                        "WHERE attribute_group_id = ?";

const std::string AttributeGroupsPersistence::create = "INSERT INTO "
                                                       "attribute_groups "
                                                       "("
                                                       "name, "
                                                       "description"
                                                       ") "
                                                       "VALUES (?, ?);";

const std::string AttributeGroupsPersistence::update = "UPDATE attribute_groups "
                                                       "SET "
                                                       "name = ?, "
                                                       "is_default = ?, "
                                                       "description = ?, "
                                                       "date_modified = ? "
                                                       "WHERE attribute_group_id = ?";

const std::string AttributeGroupsPersistence::isActive = "UPDATE attribute_groups "
                                                         "SET "
                                                         "is_active = 0, "
                                                         "date_modified = ? "
                                                         "WHERE attribute_group_id = ?";
} // namespace tks::Persistence
