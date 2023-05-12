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

#include "../models/categorymodel.h"

namespace tks
{
namespace Core
{
class Environment;
}
namespace Data
{
class CategoryData final
{
public:
    CategoryData() = delete;
    CategoryData(const CategoryData&) = delete;
    CategoryData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~CategoryData();

    CategoryData& operator=(const CategoryData&) = delete;

    std::int64_t Create(Model::CategoryModel& client);
    int Filter(const std::string& searchTerm, /*out*/ std::vector<Model::CategoryModel>& clients);
    int GetById(const std::int64_t clientId, /*out*/ Model::CategoryModel& model);
    int Update(Model::CategoryModel& client);
    int Delete(const std::int64_t clientId);

    std::int64_t GetLastInsertId() const;

private:
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static const std::string create;
    static const std::string filter;
    static const std::string getById;
    static const std::string update;
    static const std::string isActive;
};
} // namespace Data
} // namespace tks
