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
#include <tuple>
#include <vector>

#include <spdlog/logger.h>
#include <sqlite3.h>

#include "../models/employermodel.h"

namespace tks
{
namespace Core
{
class Environment;
}
namespace Data
{
class EmployerData final
{
public:
    EmployerData(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~EmployerData();

    std::int64_t Create(const Model::EmployerModel& employer);
    int GetById(const std::int64_t employerId, /*out*/ Model::EmployerModel& employer);
    int Filter(const std::string& searchTerm, /*out*/ std::vector<Model::EmployerModel>& employers);
    int Update(Model::EmployerModel employer);
    int Delete(const std::int64_t employerId);

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
