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

#include "EmployersPersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
EmployersPersistence::EmployersPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "EmployersPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "EmployersPersistence",
            databaseFilePath,
            rc,
            std::string(error));

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployersPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployersPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployersPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployersPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployersPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);

        return;
    }
}

EmployersPersistence::~EmployersPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "EmployersPersistence");
}

int EmployersPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::EmployerModel>& employerModels)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "EmployersPersistence",
        "employers",
        searchTerm);

    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::filter.c_str(),
        static_cast<int>(EmployersPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formatedSearchTerm.c_str(),
        static_cast<int>(formatedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formatedSearchTerm.c_str(),
        static_cast<int>(formatedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "description",
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
            Model::EmployerModel employerModel;

            int columnIndex = 0;

            employerModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            employerModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            employerModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                employerModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                employerModel.Description = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
            }

            columnIndex++;

            employerModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            employerModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            employerModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            employerModels.push_back(employerModel);
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
            "EmployersPersistence",
            EmployersPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "EmployersPersistence",
        employerModels.size(),
        searchTerm);

    return 0;
}

int EmployersPersistence::GetById(const std::int64_t employerId,
    Model::EmployerModel& employerModel)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "EmployersPersistence", "model", employerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::getById.c_str(),
        static_cast<int>(EmployersPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, employerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "employer_id",
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
            "EmployersPersistence",
            EmployersPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    employerModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    employerModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    employerModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        employerModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        employerModel.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }

    columnIndex++;

    employerModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    employerModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    employerModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "EmployersPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndGetByIdEntity, "EmployersPersistence", employerId);

    return 0;
}

std::int64_t EmployersPersistence::Create(const Model::EmployerModel& employerModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginCreateEntity,
        "EmployersPersistence",
        "employer",
        employerModel.Name);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::create.c_str(),
        static_cast<int>(EmployersPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        employerModel.Name.c_str(),
        static_cast<int>(employerModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "name",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, employerModel.IsDefault);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "is_default",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (employerModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            employerModel.Description.value().c_str(),
            static_cast<int>(employerModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "description",
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
            "EmployersPersistence",
            EmployersPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "EmployersPersistence", rowId);

    return rowId;
}

int EmployersPersistence::Update(Model::EmployerModel employerModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginUpdateEntity,
        "EmployersPersistence",
        "employer",
        employerModel.EmployerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::update.c_str(),
        static_cast<int>(EmployersPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(stmt,
        bindIndex,
        employerModel.Name.c_str(),
        static_cast<int>(employerModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
            "name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, employerModel.IsDefault);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "is_default",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    if (employerModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            employerModel.Description.value().c_str(),
            static_cast<int>(employerModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
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
            "EmployersPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, employerModel.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
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
            "EmployersPersistence",
            EmployersPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndUpdateEntity, "EmployersPersistence", employerModel.EmployerId);

    return 0;
}

int EmployersPersistence::Delete(const std::int64_t employerId)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginDeleteEntity, "EmployersPersistence", "employer", employerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::isActive.c_str(),
        static_cast<int>(EmployersPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::isActive,
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
            "EmployersPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, employerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "EmployersPersistence",
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
            "EmployersPersistence",
            EmployersPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndDeleteEntity, "EmployersPersistence", employerId);

    return 0;
}

int EmployersPersistence::UnsetDefault()
{
    SPDLOG_LOGGER_TRACE(pLogger, "EmployersPersistence - Unset default employer");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::unsetDefault.c_str(),
        static_cast<int>(EmployersPersistence::unsetDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::unsetDefault,
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
            "EmployersPersistence",
            "date_modified",
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
            "EmployersPersistence",
            EmployersPersistence::unsetDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, "EmployersPersistence - Completed unsetting default employer");

    return 0;
}

int EmployersPersistence::SelectDefault(Model::EmployerModel& employerModel)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, "{0} - Retrieve default \"{1}\"", "EmployersPersistence", "employer");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployersPersistence::selectDefault.c_str(),
        static_cast<int>(EmployersPersistence::selectDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployersPersistence",
            EmployersPersistence::selectDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        SPDLOG_LOGGER_TRACE(pLogger, "{0} - No default employer found", "EmployersPersistence");

        return 0;
    } else if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "EmployersPersistence",
            EmployersPersistence::selectDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    employerModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    employerModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    employerModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        employerModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        employerModel.Description = std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }

    columnIndex++;

    employerModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    employerModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    employerModel.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "EmployersPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, "{0} - Retreived default entity (if any)", "EmployersPersistence");

    return 0;
}

std::int64_t EmployersPersistence::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string EmployersPersistence::filter = "SELECT "
                                                 "employer_id, "
                                                 "name, "
                                                 "is_default, "
                                                 "description, "
                                                 "date_created, "
                                                 "date_modified, "
                                                 "is_active "
                                                 "FROM employers "
                                                 "WHERE is_active = 1 "
                                                 "AND (name LIKE ? "
                                                 "OR description LIKE ?)";

const std::string EmployersPersistence::getById = "SELECT "
                                                  "employer_id, "
                                                  "name, "
                                                  "is_default, "
                                                  "description, "
                                                  "date_created, "
                                                  "date_modified, "
                                                  "is_active "
                                                  "FROM employers "
                                                  "WHERE employer_id = ?";

const std::string EmployersPersistence::create = "INSERT INTO "
                                                 "employers "
                                                 "("
                                                 "name, "
                                                 "is_default, "
                                                 "description"
                                                 ") "
                                                 "VALUES (?, ?, ?);";

const std::string EmployersPersistence::update = "UPDATE employers "
                                                 "SET "
                                                 "name = ?, "
                                                 "is_default = ?, "
                                                 "description = ?, "
                                                 "date_modified = ? "
                                                 "WHERE employer_id = ?";

const std::string EmployersPersistence::isActive = "UPDATE employers "
                                                   "SET "
                                                   "is_active = 0, "
                                                   "date_modified = ? "
                                                   "WHERE employer_id = ?";

const std::string EmployersPersistence::unsetDefault = "UPDATE employers "
                                                       "SET "
                                                       "is_default = 0, "
                                                       "date_modified = ?";

const std::string EmployersPersistence::selectDefault = "SELECT "
                                                        "employer_id, "
                                                        "name, "
                                                        "is_default, "
                                                        "description, "
                                                        "date_created, "
                                                        "date_modified, "
                                                        "is_active "
                                                        "FROM employers "
                                                        "WHERE is_default = 1";
} // namespace tks::Persistence
