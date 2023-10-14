#include "datestore.h"
// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

void DateStore::Initialize()
{
    auto todaysDate = date::floor<date::days>(std::chrono::system_clock::now());
    pLogger->info("DateStore::Initialize - Todays date: {0}", date::format("%F", todaysDate));

    MondayDate = todaysDate - (date::weekday{ todaysDate } - date::Monday);
    pLogger->info("DateStore::Initialize - Monday date: {0}", date::format("%F", MondayDate));

    SundayDate = MondayDate + (date::Sunday - date::Monday);
    pLogger->info("DateStore::Initialize - Sunday date: {0}", date::format("%F", SundayDate));

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
