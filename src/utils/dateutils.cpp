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

#include "dateutils.h"

#include "utils.h"

namespace tks::Utils
{
std::int64_t UnixTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::int64_t UnixTimestampMidnight(date::sys_days date)
{
    auto duration = date.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::int64_t UnixTimestampNextDayMidnight(date::sys_days date)
{
    auto tomorrowMidnight = date + date::days{ 1 };
    auto duration = tomorrowMidnight.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::string ToISODateTime(std::int64_t unixTimestamp)
{
    date::sys_seconds tp{ std::chrono::seconds{ unixTimestamp } };
    std::string date = date::format("%F %T", tp);
    return date;
}

std::string Timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::string date = date::format("%F-%T", now);
    date = ReplaceAll(date, ":", "-");
    return date;
}
} // namespace tks::Utils
