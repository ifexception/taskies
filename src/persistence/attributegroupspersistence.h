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
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../models/attributegroupmodel.h"

namespace tks::Persistence
{
class AttributeGroupsPersistence final
{
public:
    AttributeGroupsPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    ~AttributeGroupsPersistence();

    int Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::AttributeGroupModel>& attributeGroupModels);
    int GetById(const std::int64_t employerId,
        /*out*/ Model::AttributeGroupModel& attributeGroupModel);
    std::int64_t Create(const Model::AttributeGroupModel& attributeGroupModel);
    int Update(Model::AttributeGroupModel attributeGroupModel);
    int Delete(const std::int64_t attributeGroupId);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static const std::string filter;
    static const std::string getById;
    static const std::string create;
    static const std::string update;
    static const std::string isActive;
};
} // namespace tks::Persistence
