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
#include <map>
#include <string>
#include <vector>

#include "base/persistencebase.h"

#include "../common/enums.h"

#include "../common/results/sqliteresult.h"

#include "../models/taskmodel.h"

namespace tks::Persistence
{
struct TasksPersistence final : public PersistenceBase {
    TasksPersistence(const std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~TasksPersistence() = default;

    SqliteResult GetById(const std::int64_t taskId,
        /*out*/ Model::TaskModel& taskModel) const;
    SqliteResult Create(std::int64_t& taskId, Model::TaskModel& taskModel) const;
    SqliteResult Update(Model::TaskModel& taskModel) const;
    SqliteResult Delete(const std::int64_t taskId);
    SqliteResult GetDescriptionById(const std::int64_t taskId,
        std::string& description) const;
    SqliteResult IsDeleted(const std::int64_t taskId, bool& value);

    static std::string getById;
    static std::string create;
    static std::string update;
    static std::string isActive;
    static std::string getDescriptionById;
    static std::string isDeleted;
};
} // namespace tks::Persistence
