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

#include "statusbar.h"

#include <fmt/format.h>

namespace tks::UI
{
StatusBar::StatusBar(wxWindow* parent)
    : wxStatusBar(parent, wxID_ANY, wxSTB_DEFAULT_STYLE, "statusbar")
{
    int widths[] = { -1, -1, -1 };
    SetFieldsCount(Fields::Count);
    SetStatusWidths(Fields::Count, widths);

    SetStatusText("Ready", Fields::Default);
}

void StatusBar::UpdateCurrentDayHours(const std::string& currentDayHours)
{
    auto text = fmt::format("Day: {0}", currentDayHours);
    SetStatusText(text, Fields::DayHours);
}

void StatusBar::UpdateCurrentWeekHours(const std::string& currentWeekHours)
{
    auto text = fmt::format("Week: {0}", currentWeekHours);
    SetStatusText(text, Fields::WeekHours);
}
} // namespace tks::UI
