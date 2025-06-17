// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include "events.h"
#include "notificationclientdata.h"

#include "../services/taskduration/taskdurationviewmodel.h"

namespace tks::UI
{
std::string StatusBar::HoursDayFormat = "[D] {0}";
std::string StatusBar::HoursWeekMonthFormat = "[W] {0} | [M] {1}";
std::string StatusBar::HoursRangeFormat = "[R] {0}";

std::string StatusBar::BillableDayFormat = "[D] {0}";
std::string StatusBar::BillableWeekMonthFormat = "[W] {0} | [M] {1}";
std::string StatusBar::BillableRangeFormat = "[R] {0}";

StatusBar::StatusBar(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : wxStatusBar(parent, wxID_ANY, wxSTB_DEFAULT_STYLE, "tksstatusbar")
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , mTaskDurationService(pLogger, mDatabaseFilePath)
    , mDefaultHoursWeek()
    , mDefaultHoursMonth()
    , mBillableHoursWeek()
    , mBillableHoursMonth()
{
    int widths[] = {
        -1, FromDIP(56), FromDIP(64), FromDIP(136), FromDIP(56), FromDIP(64), FromDIP(136)
    };

    // clang-format off
    // TODO(SW): calling SetFieldsCount *without* the width parameters logs the following error(?) messages:
    // statusbar.cpp(448): 'SendMessage(SB_GETRECT)' failed with error 0x000000b7 (Cannot create a file when that file already exists.).
    // statusbar.cpp(448): 'SendMessage(SB_GETRECT)' failed with error 0x00000000 (The operation completed successfully.). (repeats * 10)
    // Though the statusbar gets painted correctly and continues to work fine
    // clang-format on
    SetFieldsCount(7, widths);
    // SetStatusWidths(7, widths);

    SetStatusText("Ready", Fields::Default);
    SetStatusText("Hours", Fields::HoursText);
    SetStatusText("[D] 00:00", Fields::HoursDay);
    SetStatusText("[W] 00:00 | [M] 00:00", Fields::HoursWeekMonthOrRange);
    SetStatusText("Billable", Fields::BillableText);
    SetStatusText("[D] 00:00", Fields::BillableDay);
    SetStatusText("[W] 00:00 | [M] 00:00", Fields::BillableWeekMonthOrRange);
}

void StatusBar::UpdateDefaultHoursDay(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        auto durationStatusBarFormat = fmt::format(StatusBar::HoursDayFormat, duration);
        SetStatusText(durationStatusBarFormat, Fields::HoursDay);
    }
}

void StatusBar::UpdateDefaultHoursWeek(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        mDefaultHoursWeek = duration;

        UpdateDefaultHoursWeekMonth();
    }
}

void StatusBar::UpdateDefaultHoursMonth(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        mDefaultHoursMonth = duration;

        UpdateDefaultHoursWeekMonth();
    }
}

void StatusBar::UpdateDefaultHoursRange(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        auto durationStatusBarFormat = fmt::format(StatusBar::HoursRangeFormat, duration);
        SetStatusText(durationStatusBarFormat, Fields::HoursWeekMonthOrRange);
    }
}

void StatusBar::UpdateBillableHoursDay(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        auto durationStatusBarFormat = fmt::format(StatusBar::BillableDayFormat, duration);
        SetStatusText(durationStatusBarFormat, Fields::BillableDay);
    }
}

void StatusBar::UpdateBillableHoursWeek(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        mBillableHoursWeek = duration;

        UpdateBillableHoursWeekMonth();
    }
}

void StatusBar::UpdateBillableHoursMonth(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        mBillableHoursMonth = duration;

        UpdateBillableHoursWeekMonth();
    }
}

void StatusBar::UpdateBillableHoursRange(const std::string& fromDate, const std::string& toDate)
{
    int rc = 0;
    std::string duration = "";

    rc = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (rc != 0) {
        QueueErrorNotificationEventToParentWindow();
    } else {
        auto durationStatusBarFormat = fmt::format(StatusBar::BillableRangeFormat, duration);
        SetStatusText(durationStatusBarFormat, Fields::BillableWeekMonthOrRange);
    }
}

void StatusBar::QueueErrorNotificationEventToParentWindow()
{
    std::string message = "Failed to get/calculate task durations";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}

void StatusBar::UpdateDefaultHoursWeekMonth()
{
    auto durationStatusBarFormat =
        fmt::format(StatusBar::HoursWeekMonthFormat, mDefaultHoursWeek, mDefaultHoursMonth);
    SetStatusText(durationStatusBarFormat, Fields::HoursWeekMonthOrRange);
}

void StatusBar::UpdateBillableHoursWeekMonth()
{
    auto durationStatusBarFormat =
        fmt::format(StatusBar::BillableWeekMonthFormat, mBillableHoursWeek, mBillableHoursMonth);
    SetStatusText(durationStatusBarFormat, Fields::BillableWeekMonthOrRange);
}
} // namespace tks::UI
