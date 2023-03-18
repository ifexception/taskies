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

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

std::unique_ptr<Model::EmployerModel> EmployerData::GetById(const int employerId)
{
    return std::unique_ptr<Model::EmployerModel>();
}

std::tuple<int, std::vector<Model::EmployerModel>> EmployerData::GetAll()
{
    std::vector<Model::EmployerModel> employers;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerData::getEmployers.c_str(), static_cast<int>(EmployerData::getEmployers.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return std::make_tuple(-1, std::vector<Model::EmployerModel>());
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::EmployerModel employer;
            int columnIndex = 0;
            employer.EmployerId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            employer.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                employer.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                employer.Description =
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            }
            employer.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            employer.DateModified = sqlite3_column_int(stmt, columnIndex++);
            employer.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
            employers.push_back(employer);
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
        pLogger->error("Error when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return std::make_tuple(-1, std::vector<Model::EmployerModel>());
    }

    sqlite3_finalize(stmt);

    return std::make_tuple(0, employers);
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

const std::string EmployerData::getEmployers = "SELECT employer_id, "
                                               "name, "
                                               "description, "
                                               "date_created, "
                                               "date_modified, "
                                               "is_active "
                                               "FROM employers "
                                               "WHERE is_active = 1;";
} // namespace tks::Data
