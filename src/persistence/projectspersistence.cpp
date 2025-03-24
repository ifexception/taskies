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

#include "projectspersistence.h"

#include "../common/constants.h"

#include "../models/projectmodel.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
ProjectsPersistence::ProjectsPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoOpenDatabaseConnection, "ProjectsPersistence", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "ProjectsPersistence",
            databaseFilePath,
            rc,
            std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "ProjectsPersistence",
            Utils::sqlite::pragmas::ForeignKeys,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "ProjectsPersistence",
            Utils::sqlite::pragmas::JournalMode,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "ProjectsPersistence",
            Utils::sqlite::pragmas::Synchronous,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "ProjectsPersistence",
            Utils::sqlite::pragmas::TempStore,
            rc,
            error);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecQueryTemplate,
            "ProjectsPersistence",
            Utils::sqlite::pragmas::MmapSize,
            rc,
            error);
        return;
    }
}

ProjectsPersistence::~ProjectsPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoCloseDatabaseConnection, "ProjectsPersistence");
}

int ProjectsPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::ProjectModel>& projectModels)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginFilterEntities,
        "ProjectsPersistence",
        "projects",
        searchTerm);

    sqlite3_stmt* stmt = nullptr;

    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::filter.c_str(),
        static_cast<int>(ProjectsPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // project name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(
            LogMessage::BindParameterTemplate, "ProjectsPersistence", "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // display name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "display_name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // project description
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
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
            "ProjectsPersistence",
            "employer_name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // (optional) linked client name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        formattedSearchTerm.c_str(),
        static_cast<int>(formattedSearchTerm.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "client_name",
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

            Model::ProjectModel projectModel;
            int columnIndex = 0;

            projectModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            projectModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            const unsigned char* res2 = sqlite3_column_text(stmt, columnIndex);
            projectModel.DisplayName = std::string(
                reinterpret_cast<const char*>(res2), sqlite3_column_bytes(stmt, columnIndex++));

            projectModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                projectModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                projectModel.Description = std::make_optional(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }

            columnIndex++;

            projectModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            projectModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            projectModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            projectModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                projectModel.ClientId = std::nullopt;
            } else {
                projectModel.ClientId = std::make_optional(sqlite3_column_int64(stmt, columnIndex));
            }

            projectModels.push_back(projectModel);
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
            "ProjectsPersistence",
            ProjectsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "ProjectsPersistence",
        projectModels.size(),
        searchTerm);

    return 0;
}

int ProjectsPersistence::FilterByEmployerIdOrClientId(std::optional<std::int64_t> employerId,
    std::optional<std::int64_t> clientId,
    std::vector<Model::ProjectModel>& projectModels)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginFilterEntities, "ProjectsPersistence", "projects", "");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::filterByEmployerOrClientId.c_str(),
        static_cast<int>(ProjectsPersistence::filterByEmployerOrClientId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::filterByEmployerOrClientId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // employer id
    if (employerId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, employerId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "employer_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // client id
    if (clientId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, clientId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "client_id",
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

            Model::ProjectModel projectModel;
            int columnIndex = 0;

            projectModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            projectModel.Name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            const unsigned char* res2 = sqlite3_column_text(stmt, columnIndex);
            projectModel.DisplayName = std::string(
                reinterpret_cast<const char*>(res2), sqlite3_column_bytes(stmt, columnIndex++));

            projectModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                projectModel.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                projectModel.Description = std::make_optional(std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }

            columnIndex++;

            projectModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            projectModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            projectModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            projectModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                projectModel.ClientId = std::nullopt;
            } else {
                projectModel.ClientId = std::make_optional(sqlite3_column_int64(stmt, columnIndex));
            }

            projectModels.push_back(projectModel);
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
            "ProjectsPersistence",
            ProjectsPersistence::filter,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoEndFilterEntities,
        "ProjectsPersistence",
        projectModels.size(),
        "");

    return 0;
}

int ProjectsPersistence::GetById(const std::int64_t projectId, Model::ProjectModel& projectModel)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginGetByIdEntity, "ProjectsPersistence", "project", projectId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::getById.c_str(),
        static_cast<int>(ProjectsPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "project_id",
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
            "ProjectsPersistence",
            ProjectsPersistence::getById,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    projectModel.ProjectId = sqlite3_column_int64(stmt, columnIndex++);

    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    projectModel.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

    const unsigned char* res2 = sqlite3_column_text(stmt, columnIndex);
    projectModel.DisplayName =
        std::string(reinterpret_cast<const char*>(res2), sqlite3_column_bytes(stmt, columnIndex++));

    projectModel.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        projectModel.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        projectModel.Description = std::make_optional(std::string(
            reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }

    columnIndex++;

    projectModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    projectModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
    projectModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

    projectModel.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        projectModel.ClientId = std::nullopt;
    } else {
        projectModel.ClientId = std::make_optional(sqlite3_column_int64(stmt, columnIndex));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "ProjectsPersistence", rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndGetByIdEntity, "ProjectsPersistence", projectId);

    return 0;
}

std::int64_t ProjectsPersistence::Create(Model::ProjectModel& projectModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginCreateEntity,
        "ProjectsPersistence",
        "project",
        projectModel.Name);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::create.c_str(),
        static_cast<int>(ProjectsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        projectModel.Name.c_str(),
        static_cast<int>(projectModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "ProjectsPersistence", "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // display name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        projectModel.DisplayName.c_str(),
        static_cast<int>(projectModel.DisplayName.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "display_name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, projectModel.IsDefault);

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

    // description
    if (projectModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            projectModel.Description.value().c_str(),
            static_cast<int>(projectModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "description",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex++, projectModel.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "employer_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // client id
    if (projectModel.ClientId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.ClientId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
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
            "ProjectsPersistence",
            ProjectsPersistence::create,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    auto rowId = sqlite3_last_insert_rowid(pDb);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndCreateEntity, "ProjectsPersistence", rowId);

    return rowId;
}

int ProjectsPersistence::Update(Model::ProjectModel& projectModel)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessage::InfoBeginUpdateEntity,
        "ProjectsPersistence",
        "project",
        projectModel.ProjectId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::update.c_str(),
        static_cast<int>(ProjectsPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        projectModel.Name.c_str(),
        static_cast<int>(projectModel.Name.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "ProjectsPersistence", "name", bindIndex, rc, error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // display name
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        projectModel.DisplayName.c_str(),
        static_cast<int>(projectModel.DisplayName.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "display_name",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, projectModel.IsDefault);

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

    // description
    if (projectModel.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            projectModel.Description.value().c_str(),
            static_cast<int>(projectModel.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "description",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "employer_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // client id
    if (projectModel.ClientId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.ClientId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "client_id",
            bindIndex,
            rc,
            error);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "project_id",
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
            "ProjectsPersistence",
            ProjectsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoEndUpdateEntity, "ProjectsPersistence", projectModel.ProjectId);

    return 0;
}

int ProjectsPersistence::Delete(const std::int64_t projectId)
{
    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessage::InfoBeginDeleteEntity, "ProjectsPersistence", "project", projectId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::isActive.c_str(),
        static_cast<int>(ProjectsPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::isActive,
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
            "ProjectsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::BindParameterTemplate,
            "ProjectsPersistence",
            "project_id",
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
            "ProjectsPersistence",
            ProjectsPersistence::isActive,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessage::InfoEndDeleteEntity, "ProjectsPersistence", projectId);

    return 0;
}

int ProjectsPersistence::UnsetDefault()
{
    SPDLOG_LOGGER_TRACE(pLogger, "ProjectsPersistence - Unset default project");

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::unsetDefault.c_str(),
        static_cast<int>(ProjectsPersistence::unsetDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::PrepareStatementTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::unsetDefault,
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
            "ProjectsPersistence",
            "date_modified",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessage::ExecStepTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::unsetDefault,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, "ProjectsPersistence - Completed unsetting default project");

    return 0;
}

const std::string ProjectsPersistence::filter =
    "SELECT "
    "projects.project_id, "
    "projects.name AS project_name, "
    "projects.display_name, "
    "projects.is_default, "
    "projects.description AS project_description, "
    "projects.date_created, "
    "projects.date_modified, "
    "projects.is_active, "
    "projects.employer_id, "
    "projects.client_id, "
    "employers.name AS employer_name, "
    "clients.name AS client_name "
    "FROM projects "
    "INNER JOIN employers ON projects.employer_id = employers.employer_id "
    "LEFT JOIN clients ON projects.client_id = clients.client_id "
    "WHERE projects.is_active = 1 "
    "AND (project_name LIKE ? "
    "OR display_name LIKE ? "
    "OR project_description LIKE ? "
    "OR employer_name LIKE ? "
    "OR client_name LIKE ?);";

const std::string ProjectsPersistence::getById = "SELECT "
                                                 "projects.project_id, "
                                                 "projects.name, "
                                                 "projects.display_name, "
                                                 "projects.is_default, "
                                                 "projects.description, "
                                                 "projects.date_created, "
                                                 "projects.date_modified, "
                                                 "projects.is_active, "
                                                 "projects.employer_id, "
                                                 "projects.client_id "
                                                 "FROM projects "
                                                 "WHERE projects.project_id = ?;";

const std::string ProjectsPersistence::create = "INSERT INTO "
                                                "projects"
                                                "("
                                                "name, "
                                                "display_name, "
                                                "is_default, "
                                                "description, "
                                                "employer_id, "
                                                "client_id"
                                                ") "
                                                "VALUES(?, ?, ?, ?, ?, ?)";

const std::string ProjectsPersistence::update = "UPDATE projects "
                                                "SET "
                                                "name = ?,"
                                                "display_name = ?,"
                                                "is_default = ?,"
                                                "description = ?,"
                                                "date_modified = ?,"
                                                "employer_id = ?,"
                                                "client_id = ? "
                                                "WHERE project_id = ?";

const std::string ProjectsPersistence::isActive = "UPDATE projects "
                                                  "SET "
                                                  "is_active = 0, "
                                                  "date_modified = ? "
                                                  "WHERE project_id = ?";

const std::string ProjectsPersistence::unsetDefault = "UPDATE projects "
                                                      "SET "
                                                      "is_default = 0, "
                                                      "date_modified = ?";

const std::string ProjectsPersistence::filterByEmployerOrClientId = "SELECT "
                                                                    "projects.project_id, "
                                                                    "projects.name, "
                                                                    "projects.display_name, "
                                                                    "projects.is_default, "
                                                                    "projects.description, "
                                                                    "projects.date_created, "
                                                                    "projects.date_modified, "
                                                                    "projects.is_active, "
                                                                    "projects.employer_id, "
                                                                    "projects.client_id "
                                                                    "FROM projects "
                                                                    "WHERE projects.is_active = 1 "
                                                                    "AND employer_id IS ? "
                                                                    "AND client_id IS ?;";
} // namespace tks::Persistence
