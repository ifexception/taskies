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

#include "datestore.h"

namespace tks
{
DateStore::DateStore(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
{
    Initialize();
}

void DateStore::Reset()
{
    SPDLOG_LOGGER_TRACE(pLogger, "Reset dates");
    Initialize();
}

void DateStore::OnWeekChange(date::sys_days newMondayDate)
{
    MondayDate = newMondayDate;
    SPDLOG_LOGGER_TRACE(pLogger, "Monday date: {0}", FormatDate(MondayDate));

    SundayDate = MondayDate + date::days{ 6 };
    SPDLOG_LOGGER_TRACE(pLogger, "Sunday date: {0}", FormatDate(SundayDate));

    MondayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(MondayDate.time_since_epoch()).count();
    SundayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(SundayDate.time_since_epoch()).count();
}

std::string DateStore::FormatDate(date::sys_days dateToFormat)
{
    return date::format("%F", dateToFormat);
}

void DateStore::Initialize()
{
    TodayDate = date::floor<date::days>(std::chrono::system_clock::now());
    SPDLOG_LOGGER_TRACE(pLogger, "Todays date: {0}", FormatDate(TodayDate));

    MondayDate = TodayDate - (date::weekday{ TodayDate } - date::Monday);
    SPDLOG_LOGGER_TRACE(pLogger, "Monday date: {0}", FormatDate(MondayDate));

    CurrentWeekMondayDate = MondayDate;
    SundayDate = MondayDate + date::days{ 6 };
    SPDLOG_LOGGER_TRACE(pLogger, "Sunday date: {0}", FormatDate(SundayDate));

    auto todayYearMonthDay = date::year_month_day{ TodayDate };
    FirstOfMonth = todayYearMonthDay.year() / todayYearMonthDay.month() / 1;
    LastOfMonth = todayYearMonthDay.year() / todayYearMonthDay.month() / date::last;

    SPDLOG_LOGGER_TRACE(pLogger, "First day of the month: {0}", FormatDate(FirstOfMonth));
    SPDLOG_LOGGER_TRACE(pLogger, "Last day of the month: {0}", FormatDate(LastOfMonth));

    TodayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(TodayDate.time_since_epoch()).count();
    MondayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(MondayDate.time_since_epoch()).count();
    SundayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(SundayDate.time_since_epoch()).count();
}
} // namespace tks
