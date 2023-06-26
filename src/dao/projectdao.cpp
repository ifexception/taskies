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

#include "projectdao.h"

#include "../common/constants.h"

#include "../models/projectmodel.h"

#include "../utils/utils.h"

namespace tks::DAO
{
ProjectDao::ProjectDao(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "ProjectDao", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "ProjectDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ProjectDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ProjectDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ProjectDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ProjectDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ProjectDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

ProjectDao::~ProjectDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "ProjectDao");
}

std::int64_t ProjectDao::Create(Model::ProjectModel& model)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::create.c_str(), static_cast<int>(ProjectDao::create.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // name
    rc =
        sqlite3_bind_text(stmt, bindIndex++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // display name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, model.DisplayName.c_str(), static_cast<int>(model.DisplayName.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "display_name", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex++, model.IsDefault);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "is_default", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
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
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "description", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "employer_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // client id
    if (model.ClientId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, model.ClientId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "client_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::create, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);

    return rowId;
}

int ProjectDao::Filter(const std::string& searchTerm, std::vector<Model::ProjectModel>& projects)
{
    sqlite3_stmt* stmt = nullptr;
    auto formattedSearchTerm = Utils::sqlite::FormatSearchTerm(searchTerm);

    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::filter.c_str(), static_cast<int>(ProjectDao::filter.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // project name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // display name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "display_name", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project description
    rc = sqlite3_bind_text(
        stmt, bindIndex++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "description", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // linked employer name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "employer_name", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // (optional) linked client name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, formattedSearchTerm.c_str(), static_cast<int>(formattedSearchTerm.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "client_name", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::ProjectModel model;
            int columnIndex = 0;

            model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
            const unsigned char* res2 = sqlite3_column_text(stmt, columnIndex);
            model.DisplayName =
                std::string(reinterpret_cast<const char*>(res2), sqlite3_column_bytes(stmt, columnIndex++));
            model.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.Description = std::nullopt;
            } else {
                const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
                model.Description = std::make_optional(
                    std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
            }
            columnIndex++;
            model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            model.DateModified = sqlite3_column_int(stmt, columnIndex++);
            model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
            model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);
            if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
                model.ClientId = std::nullopt;
            } else {
                model.ClientId = std::make_optional(sqlite3_column_int64(stmt, columnIndex));
            }

            projects.push_back(model);
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
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::filter, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ProjectDao::GetById(const std::int64_t projectId, Model::ProjectModel& model)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::getById.c_str(), static_cast<int>(ProjectDao::getById.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, projectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "project_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::getById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    model.ProjectId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    const unsigned char* res2 = sqlite3_column_text(stmt, columnIndex);
    model.DisplayName = std::string(reinterpret_cast<const char*>(res2), sqlite3_column_bytes(stmt, columnIndex++));
    model.IsDefault = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.Description = std::make_optional(
            std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex)));
    }
    columnIndex++;
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.ClientId = std::nullopt;
    } else {
        model.ClientId = std::make_optional(sqlite3_column_int64(stmt, columnIndex));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "ProjectDao", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ProjectDao::Update(Model::ProjectModel& project)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::update.c_str(), static_cast<int>(ProjectDao::update.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.Name.c_str(), static_cast<int>(project.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // display name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.DisplayName.c_str(), static_cast<int>(project.DisplayName.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "display_name", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex++, project.IsDefault);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "is_default", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // description
    if (project.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            project.Description.value().c_str(),
            static_cast<int>(project.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "description", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }
    bindIndex++;

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "date_modified", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex++, project.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "employer_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // client id
    if (project.ClientId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, project.ClientId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "client_id", 7, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }
    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, project.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "project_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::update, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ProjectDao::Delete(const std::int64_t projectId)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::isActive.c_str(), static_cast<int>(ProjectDao::isActive.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIdx = 1;

    rc = sqlite3_bind_int64(stmt, bindIdx++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIdx++, projectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "project_id", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::isActive, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ProjectDao::UnmarkDefault()
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(
        pDb, ProjectDao::unmarkDefault.c_str(), static_cast<int>(ProjectDao::unmarkDefault.size()), &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ProjectDao", ProjectDao::unmarkDefault, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "ProjectDao", "date_modified", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ProjectDao", ProjectDao::unmarkDefault, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    return 0;
}

const std::string ProjectDao::create = "INSERT INTO "
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

const std::string ProjectDao::filter = "SELECT "
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

const std::string ProjectDao::getById = "SELECT "
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
                                        "INNER JOIN employers ON projects.employer_id = employers.employer_id "
                                        "LEFT JOIN clients ON projects.client_id = clients.client_id "
                                        "WHERE projects.project_id = ?;";

const std::string ProjectDao::update = "UPDATE projects "
                                       "SET "
                                       "name = ?,"
                                       "display_name = ?,"
                                       "is_default = ?,"
                                       "description = ?,"
                                       "date_modified = ?,"
                                       "employer_id = ?,"
                                       "client_id = ? "
                                       "WHERE project_id = ?";

const std::string ProjectDao::isActive = "UPDATE projects "
                                         "SET "
                                         "is_active = 0, "
                                         "date_modified = ? "
                                         "WHERE project_id = ?";

const std::string ProjectDao::unmarkDefault = "UPDATE projects "
                                              "SET "
                                              "is_default = 0, "
                                              "date_modified = ?";
} // namespace tks::DAO
