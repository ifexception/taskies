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

#include "employerpersistence.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
EmployerPersistence::EmployerPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "EmployerPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "EmployerPersistence",
            databaseFilePath,
            rc,
            std::string(error));

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployerPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployerPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployerPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployerPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);

        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "EmployerPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);

        return;
    }
}

EmployerPersistence::~EmployerPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "EmployerPersistence");
}

int EmployerPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::EmployerModel>& employers)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "EmployerPersistence",
        "employers",
        searchTerm);

    auto formatedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::filter.c_str(),
        static_cast<int>(EmployerPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::filter,
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

        pLogger->error(
            LogMessage::BindParameterTemplate, "EmployerPersistence", "name", bindIndex, rc, error);

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
            "EmployerPersistence",
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
            Model::EmployerModel model;

            int columnIndex = 0;

            model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            model.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

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
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "EmployerPersistence",
            EmployerPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "EmployerPersistence",
        employers.size(),
        searchTerm);

    return 0;
}

int EmployerPersistence::GetById(const std::int64_t employerId, Model::EmployerModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "EmployerPersistence", "model", employerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::getById.c_str(),
        static_cast<int>(EmployerPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::getById,
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
            "EmployerPersistence",
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
            "EmployerPersistence",
            EmployerPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    model.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

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
    model.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "EmployerPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndGetByIdEntity, "EmployerPersistence", employerId);

    return 0;
}

std::int64_t EmployerPersistence::Create(const Model::EmployerModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginCreateEntity, "EmployerPersistence", "model", model.Name);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::create.c_str(),
        static_cast<int>(EmployerPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(
        stmt, bindIndex, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "EmployerPersistence", "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, model.IsDefault);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectPersistence",
            "is_default",
            bindIndex,
            rc,
            error);
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
            "EmployerPersistence",
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
            "EmployerPersistence",
            EmployerPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "EmployerPersistence", rowId);

    return rowId;
}

int EmployerPersistence::Update(Model::EmployerModel model)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginUpdateEntity,
        "EmployerPersistence",
        "model",
        model.EmployerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::update.c_str(),
        static_cast<int>(EmployerPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::update,
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
            LogMessage::BindParameterTemplate, "EmployerPersistence", "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, model.IsDefault);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectPersistence",
            "is_default",
            bindIndex,
            rc,
            error);

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
            "EmployerPersistence",
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
            "EmployerPersistence",
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
            "EmployerPersistence",
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
            "EmployerPersistence",
            EmployerPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndUpdateEntity, "EmployerPersistence", model.EmployerId);

    return 0;
}

int EmployerPersistence::Delete(const std::int64_t employerId)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginDeleteEntity, "EmployerPersistence", "employers", employerId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::isActive.c_str(),
        static_cast<int>(EmployerPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::isActive,
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
            "EmployerPersistence",
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
            "EmployerPersistence",
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
            "EmployerPersistence",
            EmployerPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndDeleteEntity, "EmployerPersistence", employerId);

    return 0;
}

int EmployerPersistence::UnsetDefault()
{
    SPDLOG_LOGGER_TRACE(pLogger, "EmployerPersistence - Unset default employer (if any)");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::unsetDefault.c_str(),
        static_cast<int>(EmployerPersistence::unsetDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::unsetDefault,
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
            "EmployerPersistence",
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
            "EmployerPersistence",
            EmployerPersistence::unsetDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, "EmployerPersistence - Unsetted default employer (if any)");

    return 0;
}

int EmployerPersistence::TrySelectDefault(Model::EmployerModel& model)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, "{0} - Retrieve default \"{1}\"", "EmployerPersistence", "employer");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        EmployerPersistence::selectDefault.c_str(),
        static_cast<int>(EmployerPersistence::selectDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "EmployerPersistence",
            EmployerPersistence::selectDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        SPDLOG_LOGGER_TRACE(pLogger, "{0} - No default employer found", "EmployerPersistence");

        return 0;
    } else if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "EmployerPersistence",
            EmployerPersistence::selectDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    model.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

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
    model.IsActive = sqlite3_column_int(stmt, columnIndex++);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "EmployerPersistence", rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, "{0} - Retreived default entity (if any)", "EmployerPersistence");

    return 0;
}

std::int64_t EmployerPersistence::GetLastInsertId() const
{
    return sqlite3_last_insert_rowid(pDb);
}

const std::string EmployerPersistence::filter = "SELECT "
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

const std::string EmployerPersistence::getById = "SELECT "
                                                 "employer_id, "
                                                 "name, "
                                                 "is_default, "
                                                 "description, "
                                                 "date_created, "
                                                 "date_modified, "
                                                 "is_active "
                                                 "FROM employers "
                                                 "WHERE employer_id = ?";

const std::string EmployerPersistence::create = "INSERT INTO "
                                                "employers "
                                                "("
                                                "name, "
                                                "is_default, "
                                                "description"
                                                ") "
                                                "VALUES (?, ?, ?);";

const std::string EmployerPersistence::update = "UPDATE employers "
                                                "SET "
                                                "name = ?, "
                                                "is_default = ?, "
                                                "description = ?, "
                                                "date_modified = ? "
                                                "WHERE employer_id = ?";

const std::string EmployerPersistence::isActive = "UPDATE employers "
                                                  "SET "
                                                  "is_active = 0, "
                                                  "date_modified = ? "
                                                  "WHERE employer_id = ?";

const std::string EmployerPersistence::unsetDefault = "UPDATE employers "
                                                      "SET "
                                                      "is_default = 0, "
                                                      "date_modified = ?";

const std::string EmployerPersistence::selectDefault = "SELECT "
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
