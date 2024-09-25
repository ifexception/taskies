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
#include <string>
#include <vector>
#include <map>

#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../common/enums.h"

#include "../models/taskmodel.h"

namespace tks::DAO
{
class TaskDao final
{
public:
    TaskDao() = delete;
    TaskDao(const TaskDao&) = delete;
    TaskDao(const std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    ~TaskDao();

    TaskDao& operator=(const TaskDao&) = delete;

    int GetById(const std::int64_t taskId, /*out*/ Model::TaskModel& model);
    std::int64_t Create(Model::TaskModel& model);
    int Update(Model::TaskModel& task);
    int Delete(const std::int64_t taskId);
    int GetDescriptionById(const std::int64_t taskId, std::string& description);
    int IsDeleted(const std::int64_t taskId, bool& value);
    int GetTaskDurationsForDateRange(const std::string& startDate,
        const std::string& endDate,
        TaskDurationType type,
        /*out*/ std::vector<Model::TaskDurationModel>& models);
    int GetHoursForDateRangeGroupedByDate(const std::vector<std::string>& dates,
        /*out*/ std::map<std::string, std::vector<Model::TaskDurationModel>>& durationsGroupedByDate);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static const std::string getById;
    static const std::string create;
    static const std::string update;
    static const std::string isActive;
    static const std::string getDescriptionById;
    static const std::string isDeleted;
    static const std::string getAllHoursForDateRange;
    static const std::string getBillableHoursForDateRange;
    static const std::string getAllHoursForDate;
};
} // namespace tks::DAO
