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

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../services/taskduration/taskdurationservice.h"

namespace tks::UI
{
class StatusBar : public wxStatusBar
{
public:
    StatusBar() = delete;
    StatusBar(wxWindow* parent, std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    virtual ~StatusBar() = default;

    void UpdateDefaultHoursDay(const std::string& fromDate, const std::string& toDate);
    void UpdateDefaultHoursWeek(const std::string& fromDate, const std::string& toDate);
    void UpdateDefaultHoursMonth(const std::string& fromDate, const std::string& toDate);

    void UpdateDefaultHoursRange(const std::string& fromDate, const std::string& toDate);

    void UpdateBillableHoursDay(const std::string& fromDate, const std::string& toDate);
    void UpdateBillableHoursWeek(const std::string& fromDate, const std::string& toDate);
    void UpdateBillableHoursMonth(const std::string& fromDate, const std::string& toDate);

    void UpdateBillableHoursRange(const std::string& fromDate, const std::string& toDate);

    void QueueErrorNotificationEventToParentWindow();

    enum Fields {
        Default = 0,
        HoursText = 1,
        HoursDay,
        HoursWeekMonthOrRange,
        BillableText,
        BillableDay,
        BillableWeekMonthOrRange,
        Count
    };

private:
    void UpdateDefaultHoursWeekMonth();
    void UpdateBillableHoursWeekMonth();

    wxWindow* pParent;

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    Services::TaskDuration::TaskDurationService mTaskDurationService;

    std::string mDefaultHoursWeek;
    std::string mDefaultHoursMonth;
    std::string mBillableHoursWeek;
    std::string mBillableHoursMonth;

    static std::string HoursDayFormat;
    static std::string HoursWeekMonthFormat;
    static std::string HoursRangeFormat;

    static std::string BillableDayFormat;
    static std::string BillableWeekMonthFormat;
    static std::string BillableRangeFormat;
};
} // namespace tks::UI
