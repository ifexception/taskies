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
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "description", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

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

    if (category.Description.has_value()) {
        rc = sqlite3_bind_text(stmt,
            bindIndex,
            category.Description.value().c_str(),
            static_cast<int>(category.Description.value().size()),
            SQLITE_TRANSIENT);
    } else {
        rc = sqlite3_bind_null(stmt, bindIndex);
    }
    bindIndex++;

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "description", 4, rc, err);
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
        pLogger->error(LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 5, rc, err);
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

const std::string SetupWizardRepository::createClient = "INSERT INTO "
                                                        "clients "
                                                        "("
                                                        "name, "
                                                        "description, "
                                                        "employer_id"
                                                        ") "
                                                        "VALUES (?, ?, ?)";

const std::string SetupWizardRepository::createProject = "INSERT INTO "
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

const std::string SetupWizardRepository::createCategory = "";
} // namespace tks::repos
