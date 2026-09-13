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

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <date/date.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

namespace tks
{
class DateStore
{
public:
    explicit DateStore(std::shared_ptr<spdlog::logger> logger);
    ~DateStore() = default;

    date::sys_days TodayDate;
    date::sys_days CurrentWeekMondayDate;

    date::sys_days MondayDate;
    date::sys_days SundayDate;
    date::sys_days FirstOfMonth;
    date::sys_days LastOfMonth;

    std::int64_t TodayDateSeconds;
    std::int64_t MondayDateSeconds;
    std::int64_t SundayDateSeconds;

    void Reset();

    bool IsWeekDifferent(date::sys_days newDate);
    void OnWeekChange(date::sys_days newDate);

    std::string FormatDate(date::sys_days dateToFormat) const;

private:
    void Initialize();
    date::sys_days GetStartOfWeek(date::sys_days selectedDate);

    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace tks
