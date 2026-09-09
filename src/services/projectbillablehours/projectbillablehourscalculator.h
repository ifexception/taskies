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
#include <string>

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

namespace tks::Services
{
struct ProjectBillableHoursCalculatorService : public Persistence::PersistenceBase {
    ProjectBillableHoursCalculatorService() = delete;
    ProjectBillableHoursCalculatorService(const ProjectBillableHoursCalculatorService&) = delete;
    ProjectBillableHoursCalculatorService(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~ProjectBillableHoursCalculatorService();

    ProjectBillableHoursCalculatorService& operator=(
        const ProjectBillableHoursCalculatorService&) = delete;

    SqliteResult CalculateTotalBillableHoursByProjectId(const std::string& monthStart,
        const std::string& monthEnd,
        const std::int64_t projectId,
        /*out*/ double& total);

    static std::string getTotalBillableHoursByProjectId;
};
} // namespace tks::Services
