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
        pLogger->error(
            "ClientData::Create - Failed to execute \"createClient\" statement\n {0} - {1}",
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

int ClientData::GetById(const std::int64_t clientId, Model::ClientModel& client)
{
    return 0;
}

int ClientData::Filter(const std::string& searchTerm, std::vector<Model::ClientModel>& clients)
{
    return 0;
}

int ClientData::Update(Model::ClientModel& client)
{
    return 0;
}

int ClientData::Delete(const std::int64_t clientId)
{
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
} // namespace tks::Data
