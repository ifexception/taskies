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

std::int64_t SetupWizardService::CreateCategory(const Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);
    std::int64_t rowId = categoriesPersistence.Create(categoryModel);
    return rowId;
}

int SetupWizardService::GetByCategoryId(const std::int64_t categoryId,
    Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);
    int rc = categoriesPersistence.GetById(categoryId, categoryModel);
    return rc;
}

int SetupWizardService::UpdateCategory(const Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);
    int rc = categoriesPersistence.Update(categoryModel);
    return rc;
}

bool SetupWizardService::IsInTransaction() const
{
    return mTransactionCounter == 1;
}

std::string SetupWizardService::beginTransaction = "BEGIN TRANSACTION";

std::string SetupWizardService::commitTransaction = "COMMIT";

std::string SetupWizardService::rollbackTransaction = "ROLLBACK";
} // namespace tks::Services
