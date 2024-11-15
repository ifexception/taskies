// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "clientdao.h"

#include "../common/constants.h"

#include "../models/employermodel.h"

#include "../utils/utils.h"

namespace tks::DAO
{
ClientDao::ClientDao(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "ClientDao", databaseFilePath);
    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "ClientDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ClientDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ClientDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ClientDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ClientDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ClientDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

ClientDao::~ClientDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "ClientDao");
}

int ClientDao::Filter(const std::string& searchTerm, std::vector<Model::ClientModel>& clients)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "ClientDao", "clients", searchTerm);

    sqlite3_stmt* stmt = nullptr;
    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    int rc =
        sqlite3_prepare_v2(pDb, ClientDao::filter.c_str(), static_cast<int>(ClientDao::filter.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    // client name
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // client description
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // linked employer name
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "employer_name", 3, rc, err);
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
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndFilterEntities, "ClientDao", clients.size(), searchTerm);

    return 0;
}

int ClientDao::FilterByEmployerId(const std::int64_t employerId, std::vector<Model::ClientModel>& clients)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "ClientDao", "clients by employer ID", employerId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ClientDao::filterByEmployerId.c_str(),
        static_cast<int>(ClientDao::filterByEmployerId.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::filterByEmployerId, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    // employer name
    rc = sqlite3_bind_int64(stmt, bindIdx++, employerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "employer_id", 1, rc, err);
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
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::filterByEmployerId, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndFilterEntities, "ClientDao", clients.size(), employerId);

    return 0;
}

int ClientDao::GetById(const std::int64_t clientId, Model::ClientModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "ClientDao", "client", clientId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ClientDao::getById.c_str(), static_cast<int>(ClientDao::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, clientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "client_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    model.ClientId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "ClientDao", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "ClientDao", clientId);

    return 0;
}

std::int64_t ClientDao::Create(Model::ClientModel& model)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "ClientDao", "client", model.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, ClientDao::create.c_str(), static_cast<int>(ClientDao::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc =
        sqlite3_bind_text(stmt, bindIndex++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "name", 1, rc, err);
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "employer_id", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "ClientDao", rowId);

    return rowId;
}

int ClientDao::Update(Model::ClientModel& model)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "ClientDao", "client", model.ClientId);
    sqlite3_stmt* stmt = nullptr;

    int rc =
        sqlite3_prepare_v2(pDb, ClientDao::update.c_str(), static_cast<int>(ClientDao::update.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc =
        sqlite3_bind_text(stmt, bindIndex++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "name", 1, rc, err);
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "date_modified", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, model.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "employer_id", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, model.ClientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "client_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "ClientDao", model.ClientId);

    return 0;
}

int ClientDao::Delete(const std::int64_t clientId)
{
    pLogger->info(LogMessage::InfoBeginDeleteEntity, "ClientDao", "client", clientId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ClientDao::isActive.c_str(), static_cast<int>(ClientDao::isActive.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ClientDao", ClientDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, clientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ClientDao", "client_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ClientDao", ClientDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndDeleteEntity, "ClientDao", clientId);

    return 0;
}

std::int64_t ClientDao::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string ClientDao::filter = "SELECT "
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

const std::string ClientDao::filterByEmployerId = "SELECT "
                                                  "clients.client_id, "
                                                  "clients.name, "
                                                  "clients.description, "
                                                  "clients.date_created, "
                                                  "clients.date_modified, "
                                                  "clients.is_active, "
                                                  "clients.employer_id "
                                                  "FROM clients "
                                                  "WHERE employer_id = ?";

const std::string ClientDao::getById = "SELECT "
                                       "clients.client_id, "
                                       "clients.name, "
                                       "clients.description, "
                                       "clients.date_created, "
                                       "clients.date_modified, "
                                       "clients.is_active, "
                                       "clients.employer_id "
                                       "FROM clients "
                                       "WHERE clients.client_id = ?";

const std::string ClientDao::create = "INSERT INTO "
                                      "clients "
                                      "("
                                      "name, "
                                      "description, "
                                      "employer_id"
                                      ") "
                                      "VALUES (?, ?, ?)";

const std::string ClientDao::update = "UPDATE clients "
                                      "SET "
                                      "name = ?, "
                                      "description = ?, "
                                      "date_modified = ?, "
                                      "employer_id = ? "
                                      "WHERE client_id = ?";

const std::string ClientDao::isActive = "UPDATE clients "
                                        "SET "
                                        "is_active = 0, "
                                        "date_modified = ? "
                                        "WHERE client_id = ?";
} // namespace tks::DAO
