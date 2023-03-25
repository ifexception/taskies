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
#include "../utils/utils.h"

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
        pLogger->error("EmployerData - Failed to open database {0}", std::string(err));
    }
}

EmployerData::~EmployerData()
{
    sqlite3_close(pDb);
}

std::int64_t EmployerData::Create(const Model::EmployerModel& employer)
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

    rc = sqlite3_bind_text(stmt, 1, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    if (employer.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            2,
            employer.Description.value().c_str(),
            static_cast<int>(employer.Description.value().size()),
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

int EmployerData::GetById(const std::int64_t employerId, Model::EmployerModel& employer)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerData::getEmployer.c_str(), static_cast<int>(EmployerData::getEmployer.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::GetById - Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, employerId);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::GetById - Failed to bind parameter {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::GetById - Error when stepping into result {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    employer.EmployerId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    employer.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        employer.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        employer.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;
    employer.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    employer.DateModified = sqlite3_column_int(stmt, columnIndex++);
    employer.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        pLogger->error("EmployerData::GetById - Statement EmployerData::getEmployer returned more than 1 row");
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

std::tuple<int, std::vector<Model::EmployerModel>> EmployerData::Filter(const std::string& searchTerm)
{
    std::vector<Model::EmployerModel> employers;

    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        EmployerData::filterEmployers.c_str(),
        static_cast<int>(EmployerData::filterEmployers.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return std::make_tuple(-1, std::vector<Model::EmployerModel>());
    }

    rc = sqlite3_bind_text(
        stmt, 1, formatedSearchTerm.c_str(), static_cast<int>(formatedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0}", std::string(err));
        sqlite3_finalize(stmt);
        return std::make_tuple(-1, std::vector<Model::EmployerModel>());
    }

    rc = sqlite3_bind_text(
        stmt, 2, formatedSearchTerm.c_str(), static_cast<int>(formatedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0}", std::string(err));
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
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }
            columnIndex++;
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

int EmployerData::Update(Model::EmployerModel employer)
{
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(pDb,
        EmployerData::updateEmployer.c_str(),
        static_cast<int>(EmployerData::updateEmployer.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Update - Failed to bind to \"name\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    if (employer.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            2,
            employer.Description.value().c_str(),
            static_cast<int>(employer.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, 2);
    }
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Update - Failed to bind to \"description\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 3, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Update - Failed to bind to \"date_modified\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 4, employer.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Update - Failed to bind to \"employer_id\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Update - Error occured when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int EmployerData::Delete(const std::int64_t employerId)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerData::deleteEmployer.c_str(),
        static_cast<int>(EmployerData::deleteEmployer.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Delete - Error when preparing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Delete - Failed to bind to \"date_modified\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 2, employerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Delete - Failed to bind to \"employer_id\" property {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("EmployerData::Delete - Error occured when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

std::int64_t EmployerData::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string EmployerData::createEmployer = "INSERT INTO "
                                                 "employers (name, description) "
                                                 "VALUES (?, ?);";

const std::string EmployerData::filterEmployers = "SELECT employer_id, "
                                                  "name, "
                                                  "description, "
                                                  "date_created, "
                                                  "date_modified, "
                                                  "is_active "
                                                  "FROM employers "
                                                  "WHERE is_active = 1 "
                                                  "AND (name LIKE ? "
                                                  "OR description LIKE ?)";

const std::string EmployerData::getEmployer = "SELECT employer_id, "
                                              "name, "
                                              "description, "
                                              "date_created, "
                                              "date_modified, "
                                              "is_active "
                                              "FROM employers "
                                              "WHERE employer_id = ?";

const std::string EmployerData::updateEmployer = "UPDATE employers "
                                                 "SET name = ?, "
                                                 "description = ?, "
                                                 "date_modified = ? "
                                                 "WHERE employer_id = ?";

const std::string EmployerData::deleteEmployer = "UPDATE employers "
                                                 "SET is_active = 0, date_modified = ? "
                                                 "WHERE employer_id = ?";
} // namespace tks::Data
