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

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "../../models/taskmodel.h"

#include "../../dao/taskdao.h"

namespace tks::Services::TaskDuration
{
class TaskDurationService final
{
public:
    TaskDurationService() = delete;
    TaskDurationService(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    ~TaskDurationService() = default;

    int CalculateAndFormatDuration(const std::string& fromDate,
        const std::string& toDate,
        TaskDurationType type,
        /*out*/ std::string& formatDuration);

private:
    std::string CalculateTaskDurationTime(const std::vector<Model::TaskDurationModel>& taskDurations);

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;
    DAO::TaskDao taskDao;
};
} // namespace tks::Services::TaskDuration
