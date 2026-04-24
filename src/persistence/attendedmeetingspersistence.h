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
#include <memory>
#include <string>
#include <vector>

#include "base/persistencebase.h"

#include "../models/attendedmeetingmodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct AttendedMeetingsPersistence final : public PersistenceBase {
    AttendedMeetingsPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~AttendedMeetingsPersistence() = default;

    SqliteResult GetByEntryId(const std::string& entryId,
        /*out*/ Model::AttendedMeetingModel& attendedMeetingModel) const;
    SqliteResult GetByTodaysDate(const std::int32_t unixFromDateTime,
        const std::int32_t unixToDateTime,
        /*out*/ std::vector<Model::AttendedMeetingModel>& attendedMeetingModels) const;
    SqliteResult Create(std::int64_t& attendedMeetingId,
        const Model::AttendedMeetingModel& attendedMeetingModel) const;
    SqliteResult Delete(const std::int64_t attendedMeetingId) const;

    static std::string getByEntryId;
    static std::string getByTodaysDate;
    static std::string create;
    static std::string isActive;
};
} // namespace tks::Persistence
