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

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
ProjectsPersistence::ProjectsPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, databaseFilePath, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Synchronous, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::TempStore, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::MmapSize, rc, error);

        return;
    }
}

ProjectsPersistence::~ProjectsPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int ProjectsPersistence::Filter(const std::string& searchTerm,
    std::vector<Model::ProjectModel>& projectModels) const
{
    sqlite3_stmt* stmt = nullptr;

    auto formattedSearchTerm = Utils::FormatSqlSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::filter.c_str(),
        static_cast<int>(ProjectsPersistence::filter.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::filter, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "display_name", bindIndex, rc, error);

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

        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "employer_name", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "client_name", bindIndex, rc, error);

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
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, projectModels.size(), searchTerm);

    return 0;
}

int ProjectsPersistence::FilterByEmployerIdOrClientId(std::optional<std::int64_t> employerId,
    std::optional<std::int64_t> clientId,
    std::vector<Model::ProjectModel>& projectModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::filterByEmployerOrClientId.c_str(),
        static_cast<int>(ProjectsPersistence::filterByEmployerOrClientId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
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
        pLogger->error(LogMessages::BindParameterTemplate, "employer_id", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "client_id", bindIndex, rc, error);

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
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::filter, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    std::string searched =
        fmt::format("{0} - {1}", employerId, clientId.has_value() ? clientId.value() : -1);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, projectModels.size(), searched);

    return 0;
}

int ProjectsPersistence::GetById(const std::int64_t projectId,
    Model::ProjectModel& projectModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::getById.c_str(),
        static_cast<int>(ProjectsPersistence::getById.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::getById, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::getById, rc, error);

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
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "projects", projectId);

    return 0;
}

std::int64_t ProjectsPersistence::Create(const Model::ProjectModel& projectModel)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::create.c_str(),
        static_cast<int>(ProjectsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::create, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "display_name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, projectModel.IsDefault);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "is_default", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "employer_id", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "client_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    auto rowId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "project", rowId);

    return rowId;
}

int ProjectsPersistence::Update(const Model::ProjectModel& projectModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::update.c_str(),
        static_cast<int>(ProjectsPersistence::update.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::update, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "display_name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex, projectModel.IsDefault);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "is_default", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "description", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);

        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.EmployerId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "employer_id", bindIndex, rc, error);

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
        pLogger->error(LogMessages::BindParameterTemplate, "client_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectModel.ProjectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate,
            "ProjectsPersistence",
            ProjectsPersistence::update,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityUpdated, "project", projectModel.ProjectId);

    return 0;
}

int ProjectsPersistence::Delete(const std::int64_t projectId) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::isActive.c_str(),
        static_cast<int>(ProjectsPersistence::isActive.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, projectId);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "project_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::isActive, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityDeleted, "project", projectId);

    return 0;
}

int ProjectsPersistence::UnsetDefault() const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        ProjectsPersistence::unsetDefault.c_str(),
        static_cast<int>(ProjectsPersistence::unsetDefault.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, ProjectsPersistence::unsetDefault, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_int64(stmt, bindIndex, Utils::UnixTimestamp());

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "date_modified", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, ProjectsPersistence::unsetDefault, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger, "Unset default project");

    return 0;
}

std::string ProjectsPersistence::filter =
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

std::string ProjectsPersistence::getById = "SELECT "
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

std::string ProjectsPersistence::create = "INSERT INTO "
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

std::string ProjectsPersistence::update = "UPDATE projects "
                                          "SET "
                                          "name = ?,"
                                          "display_name = ?,"
                                          "is_default = ?,"
                                          "description = ?,"
                                          "date_modified = ?,"
                                          "employer_id = ?,"
                                          "client_id = ? "
                                          "WHERE project_id = ?";

std::string ProjectsPersistence::isActive = "UPDATE projects "
                                            "SET "
                                            "is_active = 0, "
                                            "date_modified = ? "
                                            "WHERE project_id = ?";

std::string ProjectsPersistence::unsetDefault = "UPDATE projects "
                                                "SET "
                                                "is_default = 0, "
                                                "date_modified = ?";

std::string ProjectsPersistence::filterByEmployerOrClientId = "SELECT "
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
