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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../models/employermodel.h"
#include "../models/clientmodel.h"
#include "../models/projectmodel.h"
#include "../models/categorymodel.h"

namespace tks::repos
{
class SetupWizardRepository final
{
public:
    SetupWizardRepository() = delete;
    SetupWizardRepository(const SetupWizardRepository&) = delete;
    SetupWizardRepository(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    ~SetupWizardRepository();

    SetupWizardRepository& operator=(const SetupWizardRepository&) = delete;

    int BeginTransaction();
    int CommitTransaction();
    int RollbackTransaction();

    std::int64_t CreateEmployer(const Model::EmployerModel& employer);
    int GetByEmployerId(const std::int64_t employerId, /*out*/ Model::EmployerModel& employer);
    int UpdateEmployer(const Model::EmployerModel& employer);

    std::int64_t CreateClient(const Model::ClientModel& client);
    int GetByClientId(const std::int64_t clientId, /*out*/ Model::ClientModel& model);
    int UpdateClient(const Model::ClientModel& client);

    std::int64_t CreateProject(const Model::ProjectModel& project);
    int GetByProjectId(const std::int64_t projectId, /*out*/ Model::ProjectModel& model);
    int UpdateProject(const Model::ProjectModel& project);

    std::int64_t CreateCategory(const Model::CategoryModel& category);
    int GetByCategoryId(const std::int64_t categoryId, /*out*/ Model::CategoryModel& model);
    int UpdateCategory(const Model::CategoryModel& model);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    int mTransactionCounter;

    static const std::string beginTransaction;
    static const std::string commitTransaction;
    static const std::string rollbackTransaction;

    static const std::string createEmployer;
    static const std::string getByEmployerId;
    static const std::string updateEmployer;

    static const std::string createClient;
    static const std::string getByClientId;
    static const std::string updateClient;

    static const std::string createProject;
    static const std::string getByProjectId;
    static const std::string updateProject;

    static const std::string createCategory;
    static const std::string getCategoryById;
    static const std::string updateCategory;
};
} // namespace tks::repos
