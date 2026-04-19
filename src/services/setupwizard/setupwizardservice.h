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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"
#include "../../models/categorymodel.h"

namespace tks::Services
{
struct SetupWizardService final : public Persistence::PersistenceBase {
    SetupWizardService() = delete;
    SetupWizardService(const SetupWizardService&) = delete;
    SetupWizardService(const std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~SetupWizardService() = default;

    SetupWizardService& operator=(const SetupWizardService&) = delete;

    int BeginTransaction();
    int CommitTransaction();
    int RollbackTransaction();

    Common::SqliteResult CreateEmployer(/*out*/ std::int64_t& employerId,
        const Model::EmployerModel& employerModel) const;
    Common::SqliteResult GetByEmployerId(const std::int64_t employerId,
        /*out*/ Model::EmployerModel& employerModel) const;
    Common::SqliteResult UpdateEmployer(const Model::EmployerModel& employerModel) const;

    Common::SqliteResult CreateClient(
        /*out*/ std::int64_t& clientId,
        const Model::ClientModel& clientModel) const;
    Common::SqliteResult GetByClientId(const std::int64_t clientId,
        /*out*/ Model::ClientModel& clientModel) const;
    Common::SqliteResult UpdateClient(const Model::ClientModel& clientModel) const;

    Common::SqliteResult CreateProject(/*out*/ std::int64_t& projectId,
        const Model::ProjectModel& projectModel) const;
    Common::SqliteResult GetByProjectId(const std::int64_t projectId,
        /*out*/ Model::ProjectModel& projectModel) const;
    Common::SqliteResult UpdateProject(const Model::ProjectModel& projectModel) const;

    Common::SqliteResult CreateCategory(std::int64_t& categoryId,
        const Model::CategoryModel& categoryModel) const;
    Common::SqliteResult GetByCategoryId(const std::int64_t categoryId,
        /*out*/ Model::CategoryModel& categoryModel) const;
    Common::SqliteResult UpdateCategory(const Model::CategoryModel& categoryModel) const;

    bool IsInTransaction() const;

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;
    int mTransactionCounter;

    static std::string beginTransaction;
    static std::string commitTransaction;
    static std::string rollbackTransaction;
};
} // namespace tks::Services
