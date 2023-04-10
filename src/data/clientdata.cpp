// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

#include "clientdata.h"

#include "../core/environment.h"
#include "../utils/utils.h"
#include "../models/employermodel.h"

namespace tks::Data
{
ClientData::ClientData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData - Failed to open database\n {0} - {1}", rc, err);
    }
}

ClientData::~ClientData()
{
    sqlite3_close(pDb);
}

std::int64_t ClientData::Create(Model::ClientModel& model)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(
        pDb, ClientData::createClient.c_str(), static_cast<int>(ClientData::createClient.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Create - Failed to prepare \"createClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    rc = sqlite3_bind_text(stmt, bindIdx++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Create - Failed to bind paramater \"Name\" in \"createClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (model.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIdx,
            model.Description.value().c_str(),
            static_cast<int>(model.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIdx);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Create - Failed to bind paramater \"Description\" in \"createClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIdx++;
    rc = sqlite3_bind_int64(stmt, bindIdx++, model.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Create - Failed to bind paramater \"EmployerId\" in \"createClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Create - Failed to execute \"createClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

int ClientData::Filter(const std::string& searchTerm, std::vector<Model::ClientModel>& clients)
{
    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(
        pDb, ClientData::filterClients.c_str(), static_cast<int>(ClientData::filterClients.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Filter - Failed to prepare \"filterClients\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    // client name
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Filter - Failed to bind parameter \"SearchTerm:Name\" in \"filterClients\" "
                       "statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // client description
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Filter - Failed to bind parameter \"SearchTerm:Description\" in \"filterClients\" "
                       "statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // linked employer name
    rc = sqlite3_bind_text(
        stmt, bindIdx++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Filter - Failed to bind parameter \"SearchTerm:EmployerName\" in \"filterClients\" "
                       "statement\n {0} - {1}",
            rc,
            err);
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
        pLogger->error("ClientData::Filter - Failed to execute \"filterClients\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ClientData::GetById(const std::int64_t clientId, Model::ClientModel& model)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(
        pDb, ClientData::getClientById.c_str(), static_cast<int>(ClientData::getClientById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::GetById - Failed to prepare \"getClientById\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, clientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::GetById - Failed to bind parameter \"client_id\" \"getClientById\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::GetById - Failed to execute \"getClientById\" statement\n {0} - {1}", rc, err);
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
        pLogger->error("ClientData::GetById - Statement \"getClientById\" returned more than 1 row of data (expected "
                       "1)\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ClientData::Update(Model::ClientModel& model)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(
        pDb, ClientData::updateClient.c_str(), static_cast<int>(ClientData::updateClient.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Update - Failed to prepare \"updateClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;
    rc = sqlite3_bind_text(stmt, bindIdx++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Update - Failed to bind parameter \"name\" in \"updateClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (model.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIdx,
            model.Description.value().c_str(),
            static_cast<int>(model.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIdx);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Update - Failed to bind paramater \"description\" in \"updateClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIdx++;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Update - Failed to bind paramater \"date_modified\" in \"updateClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx, model.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Update - Failed to bind paramater \"employer_id\" in \"updateClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Update - Failed to execute \"updateClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ClientData::Delete(const std::int64_t clientId)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(
        pDb, ClientData::deleteClient.c_str(), static_cast<int>(ClientData::deleteClient.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Delete - Failed to prepare \"deleteClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Delete - Failed to bind paramater \"date_modified\" in \"deleteClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, clientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            "ClientData::Delete - Failed to bind paramater \"client_id\" in \"deleteClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ClientData::Delete - Failed to execute \"deleteClient\" statement\n {0} - {1}", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ClientData::FilterByEmployerId(const std::int64_t employerId, std::vector<Model::ClientModel>& clients)
{
    return 0;
}

std::int64_t ClientData::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string ClientData::createClient = "INSERT INTO "
                                             "clients (name, description, employer_id) "
                                             "VALUES (?, ?, ?)";

const std::string ClientData::filterClients = "SELECT clients.client_id, "
                                              "clients.name AS client_name, "
                                              "clients.description AS client_description, "
                                              "clients.date_created, "
                                              "clients.date_modified, "
                                              "clients.is_active, "
                                              "clients.employer_id, "
                                              "clients.name AS employer_name "
                                              "FROM clients "
                                              "INNER JOIN employers "
                                              "ON clients.employer_id = employers.employer_id "
                                              "WHERE clients.is_active = 1 "
                                              "AND (client_name LIKE ? "
                                              "OR client_description LIKE ? "
                                              "OR employer_name LIKE ?); ";

const std::string ClientData::getClientById = "SELECT clients.client_id, "
                                              "clients.name AS client_name, "
                                              "clients.description, "
                                              "clients.date_created, "
                                              "clients.date_modified, "
                                              "clients.is_active, "
                                              "clients.employer_id "
                                              "FROM clients "
                                              "WHERE clients.client_id = ?";

const std::string ClientData::updateClient = "UPDATE clients "
                                             "SET name = ?, description = ?, date_modified = ?, employer_id = ? "
                                             "WHERE client_id = ?";

const std::string ClientData::deleteClient = "UPDATE clients "
                                             "SET is_active = 0, date_modified = ? "
                                             "WHERE client_id = ?";
} // namespace tks::Data
