#include "datestore.h"
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

#include "datestore.h"

namespace tks
{
DateStore::DateStore(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
{
    pLogger->info("DateStore::DateStore - Constructor initialization");
    Initialize();
}

void DateStore::Reset()
{
    pLogger->info("DateStore::Reset - Reset dates");
    Initialize();
}

std::vector<std::string> DateStore::CalculateDatesInRange(
    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate,
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate)
{
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

    return dates;
}

void DateStore::Initialize()
{
    TodayDate = date::floor<date::days>(std::chrono::system_clock::now());
    PrintTodayDate = date::format("%F", TodayDate);
    pLogger->info("DateStore::Initialize - Todays date: {0}", PrintTodayDate);

    MondayDate = TodayDate - (date::weekday{ TodayDate } - date::Monday);
    PrintMondayDate = date::format("%F", MondayDate);
    pLogger->info("DateStore::Initialize - Monday date: {0}", PrintMondayDate);

    SundayDate = MondayDate + (date::Sunday - date::Monday);
    PrintSundayDate = date::format("%F", SundayDate);
    pLogger->info("DateStore::Initialize - Sunday date: {0}", PrintSundayDate);

    auto todayYearMonthDayDate = date::year_month_day{ TodayDate };
    auto firstDayOfCurrentMonth = todayYearMonthDayDate.year() / todayYearMonthDayDate.month() / 1;
    auto lastDayOfCurrentMonth = todayYearMonthDayDate.year() / todayYearMonthDayDate.month() / date::last;

    PrintFirstDayOfMonth = date::format("%F", firstDayOfCurrentMonth);
    pLogger->info("DateStore::Initialize - First day of the month: {0}", PrintFirstDayOfMonth);

    PrintLastDayOfMonth = date::format("%F", lastDayOfCurrentMonth);
    pLogger->info("DateStore::Initialize - Last day of the month: {0}", PrintLastDayOfMonth);

    auto mondayTimestamp = MondayDate.time_since_epoch();
    MondayDateSeconds = std::chrono::duration_cast<std::chrono::seconds>(mondayTimestamp).count();

    auto sundayTimestamp = SundayDate.time_since_epoch();
    SundayDateSeconds = std::chrono::duration_cast<std::chrono::seconds>(sundayTimestamp).count();

    auto dateIterator = MondayDate;
    int index = 0;

    do {
        MondayToSundayDateRangeList.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        index++;
    } while (dateIterator != SundayDate);

    MondayToSundayDateRangeList.push_back(date::format("%F", dateIterator));
}
} // namespace tks
