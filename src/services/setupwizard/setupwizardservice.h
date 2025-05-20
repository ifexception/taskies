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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <sqlite3.h>

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"
#include "../../models/categorymodel.h"

namespace tks::Services
{
struct SetupWizardService final {
    SetupWizardService(const std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    ~SetupWizardService();

    int BeginTransaction();
    int CommitTransaction();
    int RollbackTransaction();

    std::int64_t CreateEmployer(const Model::EmployerModel& employerModel) const;
    int GetByEmployerId(const std::int64_t employerId,
        /*out*/ Model::EmployerModel& employerModel) const;
    int UpdateEmployer(const Model::EmployerModel& employerModel) const;

    std::int64_t CreateClient(const Model::ClientModel& clientModel) const;
    int GetByClientId(const std::int64_t clientId, /*out*/ Model::ClientModel& clientModel) const;
    int UpdateClient(const Model::ClientModel& clientModel) const;

    std::int64_t CreateProject(const Model::ProjectModel& projectModel) const;
    int GetByProjectId(const std::int64_t projectId, /*out*/ Model::ProjectModel& projectModel) const;
    int UpdateProject(const Model::ProjectModel& projectModel) const;

    std::int64_t CreateCategory(const Model::CategoryModel& category);
    int GetByCategoryId(const std::int64_t categoryId, /*out*/ Model::CategoryModel& model);
    int UpdateCategory(const Model::CategoryModel& model);

    bool IsInTransaction() const;

    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;
    std::string mDatabaseFilePath;
    int mTransactionCounter;

    static std::string beginTransaction;
    static std::string commitTransaction;
    static std::string rollbackTransaction;

    static std::string createEmployer;
    static std::string getByEmployerId;
    static std::string updateEmployer;

    static std::string createClient;
    static std::string getByClientId;
    static std::string updateClient;

    static std::string createProject;
    static std::string getByProjectId;
    static std::string updateProject;

    static std::string createCategory;
    static std::string getCategoryById;
    static std::string updateCategory;
};
} // namespace tks::Services
