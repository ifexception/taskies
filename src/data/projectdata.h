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
namespace Core
{
class Environment;
} // namespace Core
namespace Data
{
class ProjectData final
{
public:
    ProjectData() = delete;
    ProjectData(const ProjectData&) = delete;
    ProjectData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~ProjectData();

    ProjectData& operator=(const ProjectData&) = delete;

    std::int64_t Create(Model::ProjectData& client);
    int Filter(const std::string& searchTerm, /*out*/ std::vector<Model::ProjectData>& clients);
    int GetById(const std::int64_t clientId, /*out*/ Model::ProjectData& model);
    int Update(Model::ProjectData& client);
    int Delete(const std::int64_t clientId);
    int UnmarkDefaultProjects();

private:
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

     static const std::string createProject;
    static const std::string filterProjects;
    static const std::string getProject;
    static const std::string updateProject;
    static const std::string deleteProject;
    static const std::string unmarkDefaultProjects;
};
} // namespace Data
} // namespace tks
