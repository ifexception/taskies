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

#include "setupwizardservice.h"

#include "../../common/logmessages.h"
#include "../../common/queryhelper.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/clientspersistence.h"
#include "../../persistence/projectspersistence.h"
#include "../../persistence/categoriespersistence.h"

#include "../../utils/utils.h"

namespace tks::Services
{
SetupWizardService::SetupWizardService(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
    , mDatabaseFilePath(databaseFilePath)
    , mTransactionCounter(0)
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

SetupWizardService::~SetupWizardService()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int SetupWizardService::BeginTransaction()
{
    mTransactionCounter++;

    /*
     * There must only be one transaction active during the setup wizard's lifetime
     * This assert ensures the logic of the transaction counter remains correct
     */
    assert(mTransactionCounter == 1);

    int rc =
        sqlite3_exec(pDb, SetupWizardService::beginTransaction.c_str(), nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecQueryTemplate, SetupWizardService::beginTransaction, rc, err);
    }
    return rc;
}

int SetupWizardService::CommitTransaction()
{
    mTransactionCounter--;

    assert(mTransactionCounter == 0);

    int rc =
        sqlite3_exec(pDb, SetupWizardService::commitTransaction.c_str(), nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate,
            "SetupWizardRepository",
            SetupWizardService::commitTransaction,
            rc,
            err);
    }
    return rc;
}

int SetupWizardService::RollbackTransaction()
{
    mTransactionCounter--;

    assert(mTransactionCounter == 0);

    int rc = sqlite3_exec(
        pDb, SetupWizardService::rollbackTransaction.c_str(), nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate,
            "SetupWizardRepository",
            SetupWizardService::rollbackTransaction,
            rc,
            err);
    }
    return rc;
}

std::int64_t SetupWizardService::CreateEmployer(const Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);
    std::int64_t rowId = employersPersistence.Create(employerModel);
    return rowId;
}

int SetupWizardService::GetByEmployerId(const std::int64_t employerId,
    Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);
    int rc = employersPersistence.GetById(employerId, employerModel);
    return rc;
}

int SetupWizardService::UpdateEmployer(const Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);
    int rc = employersPersistence.Update(employerModel);
    return rc;
}

std::int64_t SetupWizardService::CreateClient(const Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);
    std::int64_t rowId = clientsPersistence.Create(clientModel);
    return rowId;
}

int SetupWizardService::GetByClientId(const std::int64_t clientId,
    Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);
    int rc = clientsPersistence.GetById(clientId, clientModel);
    return rc;
}

int SetupWizardService::UpdateClient(const Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);
    int rc = clientsPersistence.Update(clientModel);
    return rc;
}

std::int64_t SetupWizardService::CreateProject(const Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);
    std::int64_t rowId = projectsPersistence.Create(projectModel);
    return rowId;
}

int SetupWizardService::GetByProjectId(const std::int64_t projectId,
    Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);
    int rc = projectsPersistence.GetById(projectId, projectModel);
    return rc;
}

int SetupWizardService::UpdateProject(const Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);
    int rc = projectsPersistence.Update(projectModel);
    return rc;
}

std::int64_t SetupWizardService::CreateCategory(const Model::CategoryModel& category)
{
    pLogger->info(
        LogMessage::InfoBeginCreateEntity, "SetupWizardRepository", "category", category.Name);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardService::createCategory.c_str(),
        static_cast<int>(SetupWizardService::createCategory.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardService::createCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    rc = sqlite3_bind_text(stmt,
        bindIndex++,
        category.Name.c_str(),
        static_cast<int>(category.Name.size()),
        SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, static_cast<int>(category.Color));
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int(stmt, bindIndex++, category.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "billable", 3, rc, err);
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
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 4, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate,
            "SetupWizardRepository",
            SetupWizardService::createCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    pLogger->info(LogMessage::InfoEndCreateEntity, "SetupWizardRepository", rowId);

    return rowId;
}

int SetupWizardService::GetByCategoryId(const std::int64_t categoryId, Model::CategoryModel& model)
{
    pLogger->info(
        LogMessage::InfoBeginGetByIdEntity, "SetupWizardRepository", "category", categoryId);

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardService::getCategoryById.c_str(),
        static_cast<int>(SetupWizardService::getCategoryById.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardService::getCategoryById,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_int64(stmt, 1, categoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "category_id", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate,
            "SetupWizardRepository",
            SetupWizardService::getCategoryById,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int columnIndex = 0;

    model.CategoryId = sqlite3_column_int64(stmt, columnIndex++);
    const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
    model.Name =
        std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));
    model.Color = sqlite3_column_int(stmt, columnIndex++);
    model.Billable = !!sqlite3_column_int(stmt, columnIndex++);
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
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        model.ProjectId = std::nullopt;
    } else {
        model.ProjectId = std::make_optional<std::int64_t>(sqlite3_column_int64(stmt, columnIndex));
    }
    columnIndex++;

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(
            LogMessage::ExecStepMoreResultsThanExpectedTemplate, "SetupWizardRepository", rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoEndGetByIdEntity, "SetupWizardRepository", categoryId);

    return 0;
}

int SetupWizardService::UpdateCategory(const Model::CategoryModel& model)
{
    pLogger->info(
        LogMessage::InfoBeginUpdateEntity, "SetupWizardRepository", "category", model.CategoryId);
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        SetupWizardService::updateCategory.c_str(),
        static_cast<int>(SetupWizardService::updateCategory.size()),
        &stmt,
        nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate,
            "SetupWizardRepository",
            SetupWizardService::updateCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;
    // name
    rc = sqlite3_bind_text(stmt,
        bindIndex++,
        model.Name.c_str(),
        static_cast<int>(model.Name.size()),
        SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "name", 1, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // color
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Color);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "color", 2, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // billable
    rc = sqlite3_bind_int(stmt, bindIndex++, model.Billable);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "billable", 3, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // date_modified
    rc = sqlite3_bind_int64(stmt, bindIndex++, Utils::UnixTimestamp());
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate,
            "SetupWizardRepository",
            "date_modified",
            4,
            rc,
            err);
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
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "project_id", 5, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    // category_id
    rc = sqlite3_bind_int64(stmt, bindIndex++, model.CategoryId);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::BindParameterTemplate, "SetupWizardRepository", "category_id", 6, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate,
            "SetupWizardRepository",
            SetupWizardService::updateCategory,
            rc,
            err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    pLogger->info(LogMessage::InfoBeginUpdateEntity, "Category", model.CategoryId);

    return 0;
}

bool SetupWizardService::IsInTransaction() const
{
    return mTransactionCounter == 1;
}

const std::string SetupWizardService::beginTransaction = "BEGIN TRANSACTION";

const std::string SetupWizardService::commitTransaction = "COMMIT";

const std::string SetupWizardService::rollbackTransaction = "ROLLBACK";

const std::string SetupWizardService::createEmployer = "INSERT INTO "
                                                       "employers "
                                                       "("
                                                       "name "
                                                       ") "
                                                       "VALUES (?);";

const std::string SetupWizardService::getByEmployerId = "SELECT "
                                                        "employer_id, "
                                                        "name, "
                                                        "description, "
                                                        "date_created, "
                                                        "date_modified, "
                                                        "is_active "
                                                        "FROM employers "
                                                        "WHERE employer_id = ?";

const std::string SetupWizardService::updateEmployer = "UPDATE employers "
                                                       "SET "
                                                       "name = ?, "
                                                       "date_modified = ? "
                                                       "WHERE employer_id = ?";

const std::string SetupWizardService::createClient = "INSERT INTO "
                                                     "clients "
                                                     "("
                                                     "name, "
                                                     "description, "
                                                     "employer_id"
                                                     ") "
                                                     "VALUES (?, ?, ?)";

const std::string SetupWizardService::getByClientId = "SELECT "
                                                      "clients.client_id, "
                                                      "clients.name, "
                                                      "clients.description, "
                                                      "clients.date_created, "
                                                      "clients.date_modified, "
                                                      "clients.is_active, "
                                                      "clients.employer_id "
                                                      "FROM clients "
                                                      "WHERE clients.client_id = ?";

const std::string SetupWizardService::updateClient = "UPDATE clients "
                                                     "SET "
                                                     "name = ?, "
                                                     "date_modified = ?, "
                                                     "employer_id = ? "
                                                     "WHERE client_id = ?";

const std::string SetupWizardService::createProject = "INSERT INTO "
                                                      "projects"
                                                      "("
                                                      "name, "
                                                      "display_name, "
                                                      "is_default, "
                                                      "employer_id, "
                                                      "client_id"
                                                      ") "
                                                      "VALUES(?, ?, ?, ?, ?)";

const std::string SetupWizardService::getByProjectId = "SELECT "
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

const std::string SetupWizardService::updateProject = "UPDATE projects "
                                                      "SET "
                                                      "name = ?,"
                                                      "display_name = ?,"
                                                      "is_default = ?,"
                                                      "date_modified = ?,"
                                                      "employer_id = ?,"
                                                      "client_id = ? "
                                                      "WHERE project_id = ?";

const std::string SetupWizardService::createCategory = "INSERT INTO "
                                                       "categories "
                                                       "("
                                                       "name, "
                                                       "color, "
                                                       "billable, "
                                                       "project_id "
                                                       ") "
                                                       "VALUES (?, ?, ?, ?)";

const std::string SetupWizardService::getCategoryById = "SELECT "
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

const std::string SetupWizardService::updateCategory = "UPDATE categories "
                                                       "SET "
                                                       "name = ?, "
                                                       "color = ?, "
                                                       "billable = ?, "
                                                       "date_modified = ?, "
                                                       "project_id = ? "
                                                       "WHERE category_id = ?;";
;
} // namespace tks::Services
