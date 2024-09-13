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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

namespace tks::UI
{
class StatusBar : public wxStatusBar
{
public:
    StatusBar() = delete;
    StatusBar(wxWindow* parent);
    virtual ~StatusBar() = default;

    void UpdateAllHours(const std::string& allHoursDay,
        const std::string& allHoursWeek,
        const std::string& allHoursMonth);
    void UpdateBillableHours(const std::string& billableHoursDay,
        const std::string& billableHoursWeek,
        const std::string& billableHoursMonth);

    void SetAllHoursDay(const std::string& allHoursDay, bool updateText = false);
    void SetAllHoursWeek(const std::string& allHoursWeek, bool updateText = false);
    void SetAllHoursMonth(const std::string& allHoursMonth, bool updateText = false);
    void SetBillableHoursDay(const std::string& billableHoursDay, bool updateText = false);
    void SetBillableHoursWeek(const std::string& billableHoursWeek, bool updateText = false);
    void SetBillableHoursMonth(const std::string& billableHoursMonth, bool updateText = false);

    enum Fields { Default = 0, AllHours = 1, BillableHours = 2, Count };

private:
    std::string mAllHoursDay;
    std::string mAllHoursWeek;
    std::string mAllHoursMonth;
    std::string mBillableHoursDay;
    std::string mBillableHoursWeek;
    std::string mBillableHoursMonth;
};
} // namespace tks::UI
