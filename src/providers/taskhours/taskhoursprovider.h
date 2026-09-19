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

#include <spdlog/logger.h>

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

namespace tks::Providers
{
struct TaskHoursProviderResult {
    SqliteResult status;
    std::string value;
};

class TaskHoursProvider : public Persistence::PersistenceBase
{
public:
    TaskHoursProvider() = delete;
    TaskHoursProvider(const TaskHoursProvider&) = delete;
    TaskHoursProvider(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    virtual ~TaskHoursProvider();

    TaskHoursProvider& operator=(const TaskHoursProvider&) = delete;

    TaskHoursProviderResult GetTaskHoursByDate(const std::string& date);
    TaskHoursProviderResult GetBillableTaskHoursByDate(const std::string& date);

    TaskHoursProviderResult GetTaskHoursByWeek(const std::string& fromDate,
        const std::string& toDate);
    TaskHoursProviderResult GetBillableTaskHoursByWeek(const std::string& fromDate,
        const std::string& toDate);

    TaskHoursProviderResult GetTaskHoursByMonth(const std::string& fromDate,
        const std::string& toDate);
    TaskHoursProviderResult GetBillableTaskHoursByMonth(const std::string& fromDate,
        const std::string& toDate);

private:
    TaskHoursProviderResult InternalTaskHoursQuery(const std::string& sql,
        const std::string& fromDate,
        const std::string& toDate);

    static const std::string sqlTaskHoursDateRange;
    static const std::string sqlBillableTaskHoursDateRange;
};
} // namespace tks::Providers
