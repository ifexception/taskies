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

#include "employerdata.h"

#include "../core/environment.h"

namespace tks::Data
{
EmployerData::EmployerData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to open database {0}", std::string(err));
    }
}

EmployerData::~EmployerData()
{
    sqlite3_close(pDb);
}

std::int64_t EmployerData::Create(std::unique_ptr<Model::EmployerModel> employer)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pDb,
        EmployerData::createEmployer.c_str(),
        static_cast<int>(EmployerData::createEmployer.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, employer->Name.c_str(), static_cast<int>(employer->Name.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    if (employer->Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            2,
            employer->Description.value().c_str(),
            static_cast<int>(employer->Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, 2);
    }
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

std::unique_ptr<Model::EmployerModel> EmployerData::GetById(const int employerId)
{
    return std::unique_ptr<Model::EmployerModel>();
}

std::vector<std::unique_ptr<Model::EmployerModel>> EmployerData::GetAll()
{
    return std::vector<std::unique_ptr<Model::EmployerModel>>();
}

void EmployerData::Update(std::unique_ptr<Model::EmployerModel> employer) {}

void EmployerData::Delete(const int employerId) {}

std::int64_t EmployerData::GetLastInsertId() const
{
    return std::int64_t();
}

const std::string EmployerData::createEmployer = "INSERT INTO "
                                                 "employers (name, description) "
                                                 "VALUES (?, ?);";
} // namespace tks::Data
