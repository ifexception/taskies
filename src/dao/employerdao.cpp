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

#include "employerdao.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::DAO
{
EmployerDao::EmployerDao(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "EmployerDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "EmployerDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "EmployerDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "EmployerDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "EmployerDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "EmployerDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

EmployerDao::~EmployerDao()
{
    sqlite3_close(pDb);
}

std::int64_t EmployerDao::Create(const Model::EmployerModel& employer)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerDao::create.c_str(), static_cast<int>(EmployerDao::create.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "EmployerDao", EmployerDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "name", 1, rc, err);
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
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "EmployerDao", EmployerDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

int EmployerDao::GetById(const std::int64_t employerId, Model::EmployerModel& employer)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerDao::getById.c_str(), static_cast<int>(EmployerDao::getById.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "EmployerDao", EmployerDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, employerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "employer_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "EmployerDao", EmployerDao::getById, rc, err);
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "EmployerDao", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int EmployerDao::Filter(const std::string& searchTerm, std::vector<Model::EmployerModel>& employers)
{
    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerDao::filter.c_str(), static_cast<int>(EmployerDao::filter.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "EmployerDao", EmployerDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // name
    rc = sqlite3_bind_text(
        stmt, 1, formatedSearchTerm.c_str(), static_cast<int>(formatedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    rc = sqlite3_bind_text(
        stmt, 2, formatedSearchTerm.c_str(), static_cast<int>(formatedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::EmployerModel model;
            int columnIndex = 0;
            model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);
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

            employers.push_back(model);
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
        pLogger->error(LogMessage::ExecStepTemplate, "EmployerDao", EmployerDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int EmployerDao::Update(Model::EmployerModel employer)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, EmployerDao::update.c_str(), static_cast<int>(EmployerDao::update.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "EmployerDao", EmployerDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "name", 1, rc, err);
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
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 3, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "date_modified", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 4, employer.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "employer_id", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "EmployerDao", EmployerDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int EmployerDao::Delete(const std::int64_t employerId)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, EmployerDao::isActive.c_str(), static_cast<int>(EmployerDao::isActive.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "EmployerDao", EmployerDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 2, employerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "EmployerDao", "employer_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "EmployerDao", EmployerDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

std::int64_t EmployerDao::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string EmployerDao::create = "INSERT INTO "
                                         "employers "
                                         "("
                                         "name, "
                                         "description"
                                         ") "
                                         "VALUES (?, ?);";

const std::string EmployerDao::filter = "SELECT "
                                         "employer_id, "
                                         "name, "
                                         "description, "
                                         "date_created, "
                                         "date_modified, "
                                         "is_active "
                                         "FROM employers "
                                         "WHERE is_active = 1 "
                                         "AND (name LIKE ? "
                                         "OR description LIKE ?)";

const std::string EmployerDao::getById = "SELECT "
                                          "employer_id, "
                                          "name, "
                                          "description, "
                                          "date_created, "
                                          "date_modified, "
                                          "is_active "
                                          "FROM employers "
                                          "WHERE employer_id = ?";

const std::string EmployerDao::update = "UPDATE employers "
                                         "SET "
                                         "name = ?, "
                                         "description = ?, "
                                         "date_modified = ? "
                                         "WHERE employer_id = ?";

const std::string EmployerDao::isActive = "UPDATE employers "
                                           "SET "
                                           "is_active = 0, "
                                           "date_modified = ? "
                                           "WHERE employer_id = ?";
} // namespace tks::Data
