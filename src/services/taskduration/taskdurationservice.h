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

#include <memory>
#include <string>

#include "../../common/enums.h"

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

#include "taskdurationviewmodel.h"

namespace tks::Services
{
class TaskDurationService final : public Persistence::PersistenceBase {
public:
    TaskDurationService() = delete;
    TaskDurationService(const TaskDurationService&) = delete;
    TaskDurationService(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~TaskDurationService();

    TaskDurationService& operator=(const TaskDurationService&) = delete;

    SqliteResult GetTaskTimeByIdAndIncrementByValue(const std::int64_t taskId, const int value);

private:
    SqliteResult GetTaskTimeById(const std::int64_t taskId,
        /*out*/ TaskDurationViewModel& taskDurationViewModel) const;
    static void IncrementTimeByValue(const int value,
        /*out*/ TaskDurationViewModel& taskDurationViewModel);
    SqliteResult UpdateTaskTime(const std::int64_t taskId,
        TaskDurationViewModel& taskDurationViewModel) const;

    static std::string getTaskTimeById;
    static std::string updateTaskTime;
};
} // namespace tks::Services
