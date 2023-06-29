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

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../models/projectmodel.h"

namespace tks
{
namespace DAO
{
class ProjectDao final
{
public:
    ProjectDao() = delete;
    ProjectDao(const ProjectDao&) = delete;
    ProjectDao(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    ~ProjectDao();

    ProjectDao& operator=(const ProjectDao&) = delete;

    int Filter(const std::string& searchTerm, /*out*/ std::vector<Model::ProjectModel>& clients);
    int GetById(const std::int64_t projectId, /*out*/ Model::ProjectModel& model);
    std::int64_t Create(Model::ProjectModel& client);
    int Update(Model::ProjectModel& project);
    int Delete(const std::int64_t projectId);
    int UnmarkDefault();

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static const std::string filter;
    static const std::string getById;
    static const std::string create;
    static const std::string update;
    static const std::string isActive;
    static const std::string unmarkDefault;
};
} // namespace Data
} // namespace tks