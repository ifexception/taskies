// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include <wx/richmsgdlg.h>

#include "../../common/common.h"
#include "../../common/logmessages.h"

#include "../../common/results/sqliteresult.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/clientspersistence.h"
#include "../../persistence/projectspersistence.h"
#include "../../persistence/categoriespersistence.h"

#include "../../utils/utils.h"

namespace tks::Services
{
SetupWizardService::SetupWizardService(const std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : Persistence::PersistenceBase(logger, databaseFilePath)
    , mDatabaseFilePath(databaseFilePath)
    , mTransactionCounter(0)
{
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

SqliteResult SetupWizardService::CreateEmployer(/*out*/ std::int64_t& employerId,
    const Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = employersPersistence.Create(employerId, employerModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::GetByEmployerId(const std::int64_t employerId,
    Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);
    auto sqliteResult = employersPersistence.GetById(employerId, employerModel);

    return sqliteResult;
}

SqliteResult SetupWizardService::UpdateEmployer(
    const Model::EmployerModel& employerModel) const
{
    Persistence::EmployersPersistence employersPersistence(pLogger, mDatabaseFilePath);
    auto sqliteResult = employersPersistence.Update(employerModel);

    return sqliteResult;
}

SqliteResult SetupWizardService::CreateClient(
    /*out*/ std::int64_t& clientId,
    const Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = clientsPersistence.Create(clientId, clientModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::GetByClientId(const std::int64_t clientId,
    Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = clientsPersistence.GetById(clientId, clientModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::UpdateClient(const Model::ClientModel& clientModel) const
{
    Persistence::ClientsPersistence clientsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = clientsPersistence.Update(clientModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::CreateProject(std::int64_t& projectId,
    const Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = projectsPersistence.Create(projectId, projectModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::GetByProjectId(const std::int64_t projectId,
    Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = projectsPersistence.GetById(projectId, projectModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::UpdateProject(
    const Model::ProjectModel& projectModel) const
{
    Persistence::ProjectsPersistence projectsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = projectsPersistence.Update(projectModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::CreateCategory(std::int64_t& categoryId,
    const Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = categoriesPersistence.Create(categoryId, categoryModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::GetByCategoryId(const std::int64_t categoryId,
    Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = categoriesPersistence.GetById(categoryId, categoryModel);
    return sqliteResult;
}

SqliteResult SetupWizardService::UpdateCategory(
    const Model::CategoryModel& categoryModel) const
{
    Persistence::CategoriesPersistence categoriesPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = categoriesPersistence.Update(categoryModel);
    return sqliteResult;
}

bool SetupWizardService::IsInTransaction() const
{
    return mTransactionCounter == 1;
}

std::string SetupWizardService::beginTransaction = "BEGIN TRANSACTION";

std::string SetupWizardService::commitTransaction = "COMMIT";

std::string SetupWizardService::rollbackTransaction = "ROLLBACK";
} // namespace tks::Services
