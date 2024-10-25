// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "setupwizardrepository.h"

#include "../common/constants.h"

#include "../utils/utils.h"

namespace tks::repos
{
SetupWizardRepository::SetupWizardRepository(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "SetupWizardRepository", databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::OpenDatabaseTemplate, "SetupWizardRepository", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

SetupWizardRepository::~SetupWizardRepository()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "TaskRepository");
}

int SetupWizardRepository::BeginTransaction()
{
    int rc = sqlite3_exec(pDb, SetupWizardRepository::beginTransaction.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", SetupWizardRepository::beginTransaction, rc, err);
    }
    return rc;
}

int SetupWizardRepository::CommitTransaction()
{
    int rc = sqlite3_exec(pDb, SetupWizardRepository::commitTransaction.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "SetupWizardRepository", SetupWizardRepository::commitTransaction, rc, err);
    }
    return rc;
}

int SetupWizardRepository::RollbackTransaction()
{
    int rc = sqlite3_exec(pDb, SetupWizardRepository::rollbackTransaction.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::rollbackTransaction,
            rc,
            err);
    }
    return rc;
}

std::int64_t SetupWizardRepository::CreateEmployer(const Model::EmployerModel& employer)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "SetupWizardRepository", "employer", employer.Name);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::createEmployer.c_str(),
        static_cast<int>(SetupWizardRepository::createEmployer.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::createEmployer,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
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
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::createEmployer, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "SetupWizardRepository", rowId);

    return rowId;
}

int SetupWizardRepository::GetByEmployerId(const std::int64_t employerId, Model::EmployerModel& employer)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "SetupWizardRepository", "employer", employerId);

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::getByEmployerId.c_str(),
        static_cast<int>(SetupWizardRepository::getByEmployerId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::getByEmployerId,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, employerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::getByEmployerId, rc, err);
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
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "SetupWizardRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "SetupWizardRepository", employerId);

    return 0;
}

int SetupWizardRepository::UpdateEmployer(const Model::EmployerModel& employer)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "SetupWizardRepository", "employer", employer.EmployerId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::updateEmployer.c_str(),
        static_cast<int>(SetupWizardRepository::updateEmployer.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::updateEmployer,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    rc = sqlite3_bind_text(
        stmt, bindIndex++, employer.Name.c_str(), static_cast<int>(employer.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "date_modified", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, employer.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::updateEmployer, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "EmplyerDao", employer.EmployerId);

    return 0;
}

std::int64_t SetupWizardRepository::CreateClient(const Model::ClientModel& client)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "SetupWizardRepository", "client", client.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::createClient.c_str(),
        static_cast<int>(SetupWizardRepository::createClient.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::createClient,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(
        stmt, bindIndex++, client.Name.c_str(), static_cast<int>(client.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (client.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            client.Description.value().c_str(),
            static_cast<int>(client.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "description", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;
    rc = sqlite3_bind_int64(stmt, bindIndex++, client.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::createClient, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "SetupWizardRepository", rowId);

    return rowId;
}

int SetupWizardRepository::GetByClientId(const std::int64_t clientId, Model::ClientModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "SetupWizardRepository", "client", clientId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::getByClientId.c_str(),
        static_cast<int>(SetupWizardRepository::getByClientId.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::getByClientId,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, clientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "client_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::getByClientId, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;
    model.ClientId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    model.EmployerId = sqlite3_column_int64(stmt, columnIndex++);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "SetupWizardRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "SetupWizardRepository", clientId);

    return 0;
}

int SetupWizardRepository::UpdateClient(const Model::ClientModel& client)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "SetupWizardRepository", "client", client.ClientId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::updateClient.c_str(),
        static_cast<int>(SetupWizardRepository::updateClient.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::updateClient,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(
        stmt, bindIndex++, client.Name.c_str(), static_cast<int>(client.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "date_modified", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, client.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, bindIndex++, client.ClientId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "client_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::updateClient, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "ClientDao", client.ClientId);

    return 0;
}

std::int64_t SetupWizardRepository::CreateProject(const Model::ProjectModel& project)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "SetupWizardRepository", "project", project.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::createProject.c_str(),
        static_cast<int>(SetupWizardRepository::createProject.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::createProject,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.Name.c_str(), static_cast<int>(project.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // display name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.DisplayName.c_str(), static_cast<int>(project.DisplayName.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "display_name", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex++, project.IsDefault);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "is_default", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex++, project.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 4, rc, err);
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
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "client_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::createProject, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "SetupWizardRepository", rowId);

    return rowId;
}

int SetupWizardRepository::GetByProjectId(const std::int64_t projectId, Model::ProjectModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "SetupWizardRepository", "project", projectId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::getByProjectId.c_str(),
        static_cast<int>(SetupWizardRepository::getByProjectId.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::getByProjectId,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, projectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::getByProjectId, rc, err);
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
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "SetupWizardRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "SetupWizardRepository", projectId);

    return 0;
}

int SetupWizardRepository::UpdateProject(const Model::ProjectModel& project)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "SetupWizardRepository", "project", project.ProjectId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::updateProject.c_str(),
        static_cast<int>(SetupWizardRepository::updateProject.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::updateProject,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.Name.c_str(), static_cast<int>(project.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // display name
    rc = sqlite3_bind_text(
        stmt, bindIndex++, project.DisplayName.c_str(), static_cast<int>(project.DisplayName.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "display_name", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // is default
    rc = sqlite3_bind_int(stmt, bindIndex++, project.IsDefault);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "is_default", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // date modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "date_modified", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // employer id
    rc = sqlite3_bind_int64(stmt, bindIndex++, project.EmployerId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "employer_id", 5, rc, err);
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
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "client_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }
    bindIndex++;

    rc = sqlite3_bind_int64(stmt, bindIndex, project.ProjectId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 8, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::updateProject, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndUpdateEntity, "ProjectDao", project.ProjectId);

    return 0;
}

std::int64_t SetupWizardRepository::CreateCategory(const Model::CategoryModel& category)
{
    pLogger->info(LogMessage::InfoBeginCreateEntity, "SetupWizardRepository", "category", category.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::createCategory.c_str(),
        static_cast<int>(SetupWizardRepository::createCategory.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::createCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(
        stmt, bindIndex++, category.Name.c_str(), static_cast<int>(category.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, static_cast<int>(category.Color));
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, category.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "billable", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    if (category.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, category.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::createCategory, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "SetupWizardRepository", rowId);

    return rowId;
}

int SetupWizardRepository::GetByCategoryId(const std::int64_t categoryId, Model::CategoryModel& model)
{
    pLogger->info(LogMessage::InfoBeginGetByIdEntity, "SetupWizardRepository", "category", categoryId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::getCategoryById.c_str(),
        static_cast<int>(SetupWizardRepository::getCategoryById.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::getCategoryById,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, categoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "category_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::getCategoryById, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    model.Color = sqlite3_column_int(stmt, columnIndex++);
    model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.Description = std::nullopt;
    } else {
        const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
        model.Description = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex));
    }
    columnIndex++;
    model.DateCreated = sqlite3_column_int(stmt, columnIndex++);
    model.DateModified = sqlite3_column_int(stmt, columnIndex++);
    model.IsActive = !!sqlite3_column_int(stmt, columnIndex++);
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.ProjectId = std::nullopt;
    } else {
        model.ProjectId = std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }
    columnIndex++;

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "SetupWizardRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "SetupWizardRepository", categoryId);

    return 0;
}

int SetupWizardRepository::UpdateCategory(const Model::CategoryModel& model)
{
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "SetupWizardRepository", "category", model.CategoryId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardRepository::updateCategory.c_str(),
        static_cast<int>(SetupWizardRepository::updateCategory.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardRepository::updateCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // name
    rc =
        sqlite3_bind_text(stmt, bindIndex++, model.Name.c_str(), static_cast<int>(model.Name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // color
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Color);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "billable", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // date_modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "date_modified", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // project_id
    if (model.ProjectId.has_value()) {
        rc = sqlite3_bind_int64(stmt, bindIndex, model.ProjectId.value());
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category_id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "category_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecStepTemplate, "SetupWizardRepository", SetupWizardRepository::updateCategory, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "Category", model.CategoryId);

    return 0;
}

const std::string SetupWizardRepository::beginTransaction = "BEGIN TRANSACTION";

const std::string SetupWizardRepository::commitTransaction = "COMMIT";

const std::string SetupWizardRepository::rollbackTransaction = "ROLLBACK";

const std::string SetupWizardRepository::createEmployer = "INSERT INTO "
                                                          "employers "
                                                          "("
                                                          "name, "
                                                          "description"
                                                          ") "
                                                          "VALUES (?, ?);";

const std::string SetupWizardRepository::getByEmployerId = "SELECT "
                                                           "employer_id, "
                                                           "name, "
                                                           "description, "
                                                           "date_created, "
                                                           "date_modified, "
                                                           "is_active "
                                                           "FROM employers "
                                                           "WHERE employer_id = ?";

const std::string SetupWizardRepository::updateEmployer = "UPDATE employers "
                                                          "SET "
                                                          "name = ?, "
                                                          "date_modified = ? "
                                                          "WHERE employer_id = ?";

const std::string SetupWizardRepository::createClient = "INSERT INTO "
                                                        "clients "
                                                        "("
                                                        "name, "
                                                        "description, "
                                                        "employer_id"
                                                        ") "
                                                        "VALUES (?, ?, ?)";

const std::string SetupWizardRepository::getByClientId = "SELECT "
                                                         "clients.client_id, "
                                                         "clients.name, "
                                                         "clients.description, "
                                                         "clients.date_created, "
                                                         "clients.date_modified, "
                                                         "clients.is_active, "
                                                         "clients.employer_id "
                                                         "FROM clients "
                                                         "WHERE clients.client_id = ?";

const std::string SetupWizardRepository::updateClient = "UPDATE clients "
                                                        "SET "
                                                        "name = ?, "
                                                        "date_modified = ?, "
                                                        "employer_id = ? "
                                                        "WHERE client_id = ?";

const std::string SetupWizardRepository::createProject = "INSERT INTO "
                                                         "projects"
                                                         "("
                                                         "name, "
                                                         "display_name, "
                                                         "is_default, "
                                                         "employer_id, "
                                                         "client_id"
                                                         ") "
                                                         "VALUES(?, ?, ?, ?, ?)";

const std::string SetupWizardRepository::getByProjectId = "SELECT "
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

const std::string SetupWizardRepository::updateProject = "UPDATE projects "
                                                         "SET "
                                                         "name = ?,"
                                                         "display_name = ?,"
                                                         "is_default = ?,"
                                                         "date_modified = ?,"
                                                         "employer_id = ?,"
                                                         "client_id = ? "
                                                         "WHERE project_id = ?";

const std::string SetupWizardRepository::createCategory = "INSERT INTO "
                                                          "categories "
                                                          "("
                                                          "name, "
                                                          "color, "
                                                          "billable, "
                                                          "project_id "
                                                          ") "
                                                          "VALUES (?, ?, ?, ?)";

const std::string SetupWizardRepository::getCategoryById = "SELECT "
                                                           "category_id, "
                                                           "name, "
                                                           "color, "
                                                           "billable, "
                                                           "description, "
                                                           "date_created, "
                                                           "date_modified, "
                                                           "is_active, "
                                                           "project_id "
                                                           "FROM categories "
                                                           "WHERE category_id = ? "
                                                           "AND is_active = 1;";

const std::string SetupWizardRepository::updateCategory = "UPDATE categories "
                                                          "SET "
                                                          "name = ?, "
                                                          "color = ?, "
                                                          "billable = ?, "
                                                          "date_modified = ?, "
                                                          "project_id = ? "
                                                          "WHERE category_id = ?;";
;
} // namespace tks::repos
