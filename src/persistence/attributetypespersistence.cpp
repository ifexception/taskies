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

#include "attributetypespersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
AttributeTypesPersistence::AttributeTypesPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoOpenDatabaseConnection,
        "AttributeTypesPersistence",
        databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "AttributeTypesPersistence",
            databaseFilePath,
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeTypesPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeTypesPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeTypesPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeTypesPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "AttributeTypesPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            err);
        return;
    }

    SPDLOG_LOGGER_TRACE(
        pLogger, "SQLite instance initialized successfully \"{0}\"", "AttributeTypesPersistence");
}

AttributeTypesPersistence::~AttributeTypesPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoCloseDatabaseConnection, "AttributeTypesPersistence");
}

int AttributeTypesPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::AttributeTypeModel>& attributeTypeModels)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "AttributeGroupsPersistence",
        "attribute groups",
        searchTerm);

    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttributeTypesPersistence::filter.c_str(),
        static_cast<int>(AttributeTypesPersistence::filter.length()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "AttributeTypesPersistence",
            AttributeTypesPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formatedSearchTerm.c_str(),
        static_cast<int>(formatedSearchTerm.length()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "AttributeTypesPersistence",
            "name",
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
            Model::AttributeTypeModel model;

            int columnIndex = 0;
            model.AttributeTypeId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attributeTypeModels.push_back(model);
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
            "AttributeTypesPersistence",
            AttributeTypesPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "AttributeTypesPersistence",
        attributeTypeModels.size(),
        searchTerm);

    return 0;
}

int AttributeTypesPersistence::GetById(const std::int64_t attributeTypepId,
    Model::AttributeTypeModel& attributeTypeModel)
{
    return 0;
}

const std::string AttributeTypesPersistence::filter = "SELECT "
                                                      "attribute_type_id, "
                                                      "name "
                                                      "FROM attribute_types "
                                                      "WHERE name LIKE ?";

const std::string AttributeTypesPersistence::getById = "SELECT "
                                                       "attribute_type_id, "
                                                       "name "
                                                       "FROM attribute_types "
                                                       "WHERE attribute_type_id = ?";
} // namespace tks::Persistence
