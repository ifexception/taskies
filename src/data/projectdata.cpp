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

#include "projectdata.h"

#include "../core/environment.h"
#include "../utils/utils.h"
#include "../models/projectmodel.h"

namespace tks::Data
{
ProjectData::ProjectData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("ProjectData - Failed to open database\n {0} - {1}", rc, err);
    }
}

ProjectData::~ProjectData() {}

std::int64_t ProjectData::Create(Model::ProjectModel& client)
{
    return std::int64_t();
}

int ProjectData::Filter(const std::string& searchTerm, std::vector<Model::ProjectModel>& clients)
{
    return 0;
}

int ProjectData::GetById(const std::int64_t clientId, Model::ProjectModel& model)
{
    return 0;
}

int ProjectData::Update(Model::ProjectModel& client)
{
    return 0;
}

int ProjectData::Delete(const std::int64_t clientId)
{
    return 0;
}

int ProjectData::UnmarkDefaultProjects()
{
    return 0;
}
} // namespace tks::Data
