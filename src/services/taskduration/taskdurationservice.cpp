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

#include "taskdurationservice.h"

namespace tks::Services::TaskDuration
{
TaskDurationService::TaskDurationService(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , taskDao(pLogger, mDatabaseFilePath)
{
}

int TaskDurationService::CalculateAndFormatDuration(const std::string& fromDate,
    const std::string& toDate,
    TaskDurationType type,
    std::string& formatDuration)
{
    std::vector<Model::TaskDurationModel> taskDurations;
    int rc = taskDao.GetTaskDurationsForDateRange(fromDate, toDate, type, taskDurations);
    formatDuration = CalculateTaskDurationTime(taskDurations);

    return rc;
}

std::string TaskDurationService::CalculateTaskDurationTime(const std::vector<Model::TaskDurationModel>& taskDurations)
{
    int minutes = 0;
    int hours = 0;
    for (auto& duration : taskDurations) {
        hours += duration.Hours;
        minutes += duration.Minutes;
    }

    hours += (minutes / 60);
    minutes = minutes % 60;

    std::string formattedTotal = fmt::format("{0:02}:{1:02}", hours, minutes);
    return formattedTotal;
}
} // namespace tks::Services::TaskDuration
