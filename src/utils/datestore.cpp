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
DateStore::DateStore()
{
    Initialize();
}

void DateStore::Reset()
{
    Initialize();
}

void DateStore::Initialize()
{
    auto todaysDate = date::floor<date::days>(std::chrono::system_clock::now());

    MondayDate = todaysDate - (date::weekday{ todaysDate } - date::Monday);

    SundayDate = MondayDate + (date::Sunday - date::Monday);

    auto mondayTimestamp = MondayDate.time_since_epoch();
    MondayDateSeconds = std::chrono::duration_cast<std::chrono::seconds>(mondayTimestamp).count();

    auto sundayTimestamp = SundayDate.time_since_epoch();
    SundayDateSeconds = std::chrono::duration_cast<std::chrono::seconds>(sundayTimestamp).count();

    auto dateIterator = MondayDate;
    int loopIdx = 0;

    do {
        MondayToSundayDateRangeList.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != SundayDate);

    MondayToSundayDateRangeList.push_back(date::format("%F", dateIterator));
}
} // namespace tks
