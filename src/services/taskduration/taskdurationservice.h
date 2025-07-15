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

#include <memory>
#include <string>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <sqlite3.h>

#include "../../common/enums.h"

#include "taskdurationviewmodel.h"

namespace tks::Services
{
struct TaskDurationService final {
    TaskDurationService() = delete;
    TaskDurationService(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    ~TaskDurationService();

    int GetTaskDurationsForDateRange(const std::string& startDate,
        const std::string& endDate,
        TaskDurationType type,
        /*out*/ std::vector<TaskDurationViewModel>& taskDurationViewModels) const;

    int CalculateAndFormatDuration(const std::string& fromDate,
        const std::string& toDate,
        TaskDurationType type,
        /*out*/ std::string& formatDuration);

    std::string CalculateTaskDurationTime(const std::vector<TaskDurationViewModel>& taskDurations);

    int GetTaskTimeByIdAndIncrementByValue(const std::int64_t taskId,
        const int value);
    int GetTaskTimeById(const std::int64_t taskId,
        /*out*/ TaskDurationViewModel& taskDurationViewModel) const;
    void IncrementTimeByValue(const int value, /*out*/ TaskDurationViewModel& taskDurationViewModel);
    int UpdateTaskTime(const std::int64_t taskId, TaskDurationViewModel& taskDurationViewModel) const;

    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static std::string getAllHoursForDateRange;
    static std::string getBillableHoursForDateRange;
    static std::string getTaskTimeById;
    static std::string updateTaskTime;
};
} // namespace tks::Services
