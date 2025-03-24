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

#include "clientspersistence.h"

#include "../common/constants.h"

#include "../models/employermodel.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
ClientsPersistence::ClientsPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "ClientsPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "ClientsPersistence",
            databaseFilePath,
            rc,
            std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "ClientsPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "ClientsPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "ClientsPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "ClientsPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "ClientsPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);
        return;
    }

    SPDLOG_LOGGER_TRACE(
        pLogger, "\"{0}\" - SQLite instance initialized successfully", "ClientsPersistence");
}

ClientsPersistence::~ClientsPersistence()
{
    sqlite3_close(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "ClientsPersistence");
}

int ClientsPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::ClientModel>& clients)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginFilterEntities, "ClientsPersistence", "clients", searchTerm);

    sqlite3_stmt* stmt = nullptr;

    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::filter.c_str(),
        static_cast<int>(ClientsPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // client name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "ClientsPersistence", "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // client description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "description",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // linked employer name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "employer_name",
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

            Model::ClientModel model;
            int columnIndex = 0;

            model.ClientId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

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

            model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            clients.push_back(model);
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
            "ClientsPersistence",
            ClientsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "ClientsPersistence",
        clients.size(),
        searchTerm);

    return 0;
}

int ClientsPersistence::FilterByEmployerId(const std::int64_t employerId,
    std::vector<Model::ClientModel>& clients)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "ClientsPersistence",
        "clients by employer ID",
        employerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::filterByEmployerId.c_str(),
        static_cast<int>(ClientsPersistence::filterByEmployerId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::filterByEmployerId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // employer name
    rc = sqlite3_bind_int64(stmt, bindIndex, employerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "employer_id",
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

            Model::ClientModel model;
            int columnIndex = 0;

            model.ClientId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

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

            model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            clients.push_back(model);
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
            "ClientsPersistence",
            ClientsPersistence::filterByEmployerId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "ClientsPersistence",
        clients.size(),
        employerId);

    return 0;
}

int ClientsPersistence::GetById(const std::int64_t clientId, Model::ClientModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "ClientsPersistence", "client", clientId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::getById.c_str(),
        static_cast<int>(ClientsPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, clientId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "client_id",
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
            "ClientsPersistence",
            ClientsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.ClientId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

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

    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "ClientsPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndGetByIdEntity, "ClientsPersistence", clientId);

    return 0;
}

std::int64_t ClientsPersistence::Create(Model::ClientModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginCreateEntity, "ClientsPersistence", "client", model.Name);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::create.c_str(),
        static_cast<int>(ClientsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "ClientsPersistence", "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (model.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            model.Description.value().c_str(),
            static_cast<int>(model.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "description",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, model.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "employer_id",
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
            "ClientsPersistence",
            ClientsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "ClientsPersistence", rowId);

    return rowId;
}

int ClientsPersistence::Update(Model::ClientModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginUpdateEntity, "ClientsPersistence", "client", model.ClientId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::update.c_str(),
        static_cast<int>(ClientsPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "ClientsPersistence", "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    if (model.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            model.Description.value().c_str(),
            static_cast<int>(model.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
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
            "ClientsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, model.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "employer_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, model.ClientId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "client_id",
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
            "ClientsPersistence",
            ClientsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndUpdateEntity, "ClientsPersistence", model.ClientId);

    return 0;
}

int ClientsPersistence::Delete(const std::int64_t clientId)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginDeleteEntity, "ClientsPersistence", "client", clientId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientsPersistence::isActive.c_str(),
        static_cast<int>(ClientsPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ClientsPersistence",
            ClientsPersistence::isActive,
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
            "ClientsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, clientId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ClientsPersistence",
            "client_id",
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
            "ClientsPersistence",
            ClientsPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndDeleteEntity, "ClientsPersistence", clientId);

    return 0;
}

std::int64_t ClientsPersistence::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string ClientsPersistence::filter = "SELECT "
                                               "clients.client_id, "
                                               "clients.name AS client_name, "
                                               "clients.description AS client_description, "
                                               "clients.date_created, "
                                               "clients.date_modified, "
                                               "clients.is_active, "
                                               "clients.employer_id, "
                                               "employers.name AS employer_name "
                                               "FROM clients "
                                               "INNER JOIN employers "
                                               "ON clients.employer_id = employers.employer_id "
                                               "WHERE clients.is_active = 1 "
                                               "AND (client_name LIKE ? "
                                               "OR client_description LIKE ? "
                                               "OR employer_name LIKE ?); ";

const std::string ClientsPersistence::filterByEmployerId = "SELECT "
                                                           "clients.client_id, "
                                                           "clients.name, "
                                                           "clients.description, "
                                                           "clients.date_created, "
                                                           "clients.date_modified, "
                                                           "clients.is_active, "
                                                           "clients.employer_id "
                                                           "FROM clients "
                                                           "WHERE employer_id = ?";

const std::string ClientsPersistence::getById = "SELECT "
                                                "clients.client_id, "
                                                "clients.name, "
                                                "clients.description, "
                                                "clients.date_created, "
                                                "clients.date_modified, "
                                                "clients.is_active, "
                                                "clients.employer_id "
                                                "FROM clients "
                                                "WHERE clients.client_id = ?";

const std::string ClientsPersistence::create = "INSERT INTO "
                                               "clients "
                                               "("
                                               "name, "
                                               "description, "
                                               "employer_id"
                                               ") "
                                               "VALUES (?, ?, ?)";

const std::string ClientsPersistence::update = "UPDATE clients "
                                               "SET "
                                               "name = ?, "
                                               "description = ?, "
                                               "date_modified = ?, "
                                               "employer_id = ? "
                                               "WHERE client_id = ?";

const std::string ClientsPersistence::isActive = "UPDATE clients "
                                                 "SET "
                                                 "is_active = 0, "
                                                 "date_modified = ? "
                                                 "WHERE client_id = ?";
} // namespace tks::Persistence
