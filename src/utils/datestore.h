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

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <date/date.h>

#include <spdlog/logger.h>

namespace tks
{
struct DateStore {
    DateStore(std::shared_ptr<spdlog::logger> logger);
    ~DateStore() = default;

    std::chrono::time_point<std::chrono::system_clock, date::days> TodayDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> MondayDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> SundayDate;
    long long MondayDateSeconds;
    long long SundayDateSeconds;
    std::string PrintTodayDate;
    std::string PrintMondayDate;
    std::string PrintSundayDate;

    std::vector<std::string> MondayToSundayDateRangeList;

    void Reset();
    std::vector<std::string> CalculateDatesInRange(
        std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate,
        std::chrono::time_point<std::chrono::system_clock, date::days> mToDate);

    // -private
    void Initialize();

    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace tks
