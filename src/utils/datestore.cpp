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
    , TodayDateSeconds(0)
    , MondayDateSeconds(0)
    , SundayDateSeconds(0)
{
    Initialize();
}

void DateStore::Reset()
{
    SPDLOG_LOGGER_TRACE(pLogger, "Reset dates");
    Initialize();
}

bool DateStore::IsWeekDifferent(date::sys_days newDate)
{
    SPDLOG_LOGGER_TRACE(pLogger, "CurrentWeekMondayDate = {}", FormatDate(CurrentWeekMondayDate));
    SPDLOG_LOGGER_TRACE(pLogger, "newDate = {}", FormatDate(newDate));

    return GetStartOfWeek(CurrentWeekMondayDate) != GetStartOfWeek(newDate);
}

bool DateStore::IsMonthDifferent(date::sys_days newDate)
{
    // Convert time points to year_month_day components
    date::year_month_day currentDate{ TodayDate };
    date::year_month_day futureDate{ newDate };

    SPDLOG_LOGGER_TRACE(pLogger, "TodayDate = {}", FormatDate(currentDate));
    SPDLOG_LOGGER_TRACE(pLogger, "newDate = {}", FormatDate(futureDate));

    // Compare only the year and month components
    return date::year_month{ currentDate.year(), currentDate.month() } !=
           date::year_month{ futureDate.year(), futureDate.month() };
}

void DateStore::OnWeekChange(date::sys_days newDate)
{
    auto newMondayDate = GetStartOfWeek(newDate);

    MondayDate = newMondayDate;
    SPDLOG_LOGGER_TRACE(pLogger, "Monday date: {0}", FormatDate(MondayDate));

    SundayDate = MondayDate + date::days{ 6 };
    SPDLOG_LOGGER_TRACE(pLogger, "Sunday date: {0}", FormatDate(SundayDate));

    MondayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(MondayDate.time_since_epoch()).count();
    SundayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(SundayDate.time_since_epoch()).count();
}

void DateStore::OnMonthChange(date::sys_days newDate)
{
    auto todayYearMonthDayDate = date::year_month_day{ newDate };
    FirstDayOfMonth = todayYearMonthDayDate.year() / todayYearMonthDayDate.month() / 1;
    LastDayOfMonth = todayYearMonthDayDate.year() / todayYearMonthDayDate.month() / date::last;

    SPDLOG_LOGGER_TRACE(pLogger, "First day of the month: {0}", FormatDate(FirstDayOfMonth));
    SPDLOG_LOGGER_TRACE(pLogger, "Last day of the month: {0}", FormatDate(LastDayOfMonth));
}

std::string DateStore::FormatDate(date::sys_days dateToFormat) const
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

    date::year_month_day todayYearMonthDay = date::year_month_day{ TodayDate };
    FirstDayOfMonth = todayYearMonthDay.year() / todayYearMonthDay.month() / 1;
    LastDayOfMonth = todayYearMonthDay.year() / todayYearMonthDay.month() / date::last;

    SPDLOG_LOGGER_TRACE(pLogger, "First day of the month: {0}", FormatDate(FirstDayOfMonth));
    SPDLOG_LOGGER_TRACE(pLogger, "Last day of the month: {0}", FormatDate(LastDayOfMonth));

    TodayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(TodayDate.time_since_epoch()).count();
    MondayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(MondayDate.time_since_epoch()).count();
    SundayDateSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(SundayDate.time_since_epoch()).count();
}

date::sys_days DateStore::GetStartOfWeek(date::sys_days selectedDate)
{
    // weekday{0} is Sunday, weekday{1} is Monday
    date::weekday weekDay = date::weekday{ selectedDate };
    // Shift so that Monday is the start of the week
    auto daysFromMonday = (weekDay - date::Monday).count();
    if (daysFromMonday < 0) {
        daysFromMonday += 7;
    }

    return selectedDate - date::days{ daysFromMonday };
}
} // namespace tks
