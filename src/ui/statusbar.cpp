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

void StatusBar::UpdateAllHours(const std::string& allHoursDay,
    const std::string& allHoursWeek,
    const std::string& allHoursMonth)
{
    auto text = fmt::format("Hours (D - {0}) (W - {1}) (M - {2})", allHoursDay, allHoursWeek, allHoursMonth);
    SetStatusText(text, Fields::AllHours);
}

void StatusBar::UpdateBillableHours(const std::string& billableHoursDay,
    const std::string& billableHoursWeek,
    const std::string& billableHoursMonth)
{
    auto text =
        fmt::format("Billable (D - {0}) (W - {1}) (M - {2})", billableHoursDay, billableHoursWeek, billableHoursMonth);
    SetStatusText(text, Fields::BillableHours);
}

void StatusBar::SetAllHoursDay(const std::string& allHoursDay, bool updateText)
{
    mAllHoursDay = allHoursDay;

    if (updateText) {
        auto text = fmt::format("Hours (D - {0}) (W - {1}) (M - {2})", mAllHoursDay, mAllHoursWeek, mAllHoursMonth);
        SetStatusText(text, Fields::AllHours);
    }
}

void StatusBar::SetAllHoursWeek(const std::string& allHoursWeek, bool updateText)
{
    mAllHoursWeek = allHoursWeek;

    if (updateText) {
        auto text = fmt::format("Hours (D - {0}) (W - {1}) (M - {2})", mAllHoursDay, mAllHoursWeek, mAllHoursMonth);
        SetStatusText(text, Fields::AllHours);
    }
}

void StatusBar::SetAllHoursMonth(const std::string& allHoursMonth, bool updateText)
{
    mAllHoursMonth = allHoursMonth;

    if (updateText) {
        auto text = fmt::format("Hours (D - {0}) (W - {1}) (M - {2})", mAllHoursDay, mAllHoursWeek, mAllHoursMonth);
        SetStatusText(text, Fields::AllHours);
    }
}

void StatusBar::SetBillableHoursDay(const std::string& billableHoursDay, bool updateText)
{
    mBillableHoursDay = billableHoursDay;

    if (updateText) {
        auto text = fmt::format(
            "Billable (D - {0}) (W - {1}) (M - {2})", mBillableHoursDay, mBillableHoursWeek, mBillableHoursMonth);
        SetStatusText(text, Fields::BillableHours);
    }
}

void StatusBar::SetBillableHoursWeek(const std::string& billableHoursWeek, bool updateText)
{
    mBillableHoursWeek = billableHoursWeek;

    if (updateText) {
        auto text = fmt::format(
            "Billable (D - {0}) (W - {1}) (M - {2})", mBillableHoursDay, mBillableHoursWeek, mBillableHoursMonth);
        SetStatusText(text, Fields::BillableHours);
    }
}

void StatusBar::SetBillableHoursMonth(const std::string& billableHoursMonth, bool updateText)
{
    mBillableHoursMonth = billableHoursMonth;

    if (updateText) {
        auto text = fmt::format(
            "Billable (D - {0}) (W - {1}) (M - {2})", mBillableHoursDay, mBillableHoursWeek, mBillableHoursMonth);
        SetStatusText(text, Fields::BillableHours);
    }
}
} // namespace tks::UI
