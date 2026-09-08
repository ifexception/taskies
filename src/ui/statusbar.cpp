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

#include "statusbar.h"

#include <wx/richmsgdlg.h>

#include <fmt/format.h>

#include "events.h"

#include "../common/common.h"

#include "../common/messages/persistencemessages.h"

#include "../services/taskduration/taskdurationviewmodel.h"

namespace tks::UI
{
std::string StatusBar::HoursDayFormat = "[D] {0}";
std::string StatusBar::HoursWeekFormat = "[W] {0}";
std::string StatusBar::HoursMonthFormat = "[M] {0}";

std::string StatusBar::BillableDayFormat = "[D] {0}";
std::string StatusBar::BillableWeekFormat = "[W] {0}";
std::string StatusBar::BillableMonthFormat = "[M] {0}";

StatusBar::StatusBar(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : wxStatusBar(parent, wxID_ANY, wxSTB_DEFAULT_STYLE, "tksStatusBar")
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , mTaskDurationService(pLogger, mDatabaseFilePath)
{
    int widths[] = { -1,
        FromDIP(56),
        FromDIP(64),
        FromDIP(64),
        FromDIP(64),
        FromDIP(56),
        FromDIP(64),
        FromDIP(64),
        FromDIP(64) };

    // clang-format off
    // TODO(SW): calling SetFieldsCount *without* the width parameters logs the following error(?) messages:
    // statusbar.cpp(448): 'SendMessage(SB_GETRECT)' failed with error 0x000000b7 (Cannot create a file when that file already exists.).
    // statusbar.cpp(448): 'SendMessage(SB_GETRECT)' failed with error 0x00000000 (The operation completed successfully.). (repeats * 10)
    // Though the statusbar gets painted correctly and continues to work fine
    // clang-format on
    SetFieldsCount(static_cast<int>(Fields::Count), widths);
    // SetStatusWidths(9, widths);

    SetStatusText("Ready", Fields::Default);
    SetStatusText("Hours", Fields::HoursText);
    SetStatusText("[D] 00:00", Fields::HoursDay);
    SetStatusText("[W] 00:00", Fields::HoursWeek);
    SetStatusText("[M] 00:00", Fields::HoursMonth);

    SetStatusText("Billable", Fields::BillableText);
    SetStatusText("[D] 00:00", Fields::BillableDay);
    SetStatusText("[W] 00:00", Fields::BillableWeek);
    SetStatusText("[M] 00:00", Fields::BillableMonth);
}

void StatusBar::UpdateDefaultHoursDay(const std::string& todayDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        todayDate, todayDate, TaskDurationType::Default, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::HoursDayFormat, duration);
        SetStatusText(formattedText, Fields::HoursDay);
    }
}

void StatusBar::UpdateDefaultHoursWeek(const std::string& fromDate, const std::string& toDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::HoursWeekFormat, duration);
        SetStatusText(formattedText, Fields::HoursWeek);
    }
}

void StatusBar::UpdateDefaultHoursMonth(const std::string& fromDate, const std::string& toDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Default, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::HoursMonthFormat, duration);
        SetStatusText(formattedText, Fields::HoursMonth);
    }
}

void StatusBar::UpdateBillableHoursDay(const std::string& todayDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        todayDate, todayDate, TaskDurationType::Billable, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::BillableDayFormat, duration);
        SetStatusText(formattedText, Fields::BillableDay);
    }
}

void StatusBar::UpdateBillableHoursWeek(const std::string& fromDate, const std::string& toDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::BillableWeekFormat, duration);
        SetStatusText(formattedText, Fields::BillableWeek);
    }
}

void StatusBar::UpdateBillableHoursMonth(const std::string& fromDate, const std::string& toDate)
{
    std::string duration = "";

    auto sqliteResult = mTaskDurationService.CalculateAndFormatDuration(
        fromDate, toDate, TaskDurationType::Billable, duration);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationCalculationMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto formattedText = fmt::format(StatusBar::BillableMonthFormat, duration);
        SetStatusText(formattedText, Fields::BillableMonth);
    }
}
} // namespace tks::UI
