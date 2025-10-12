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

#include "outlookmeetingsviewdlg.h"

#include "../../../common/common.h"

namespace tks::UI::dlg
{
OutlookMeetingsViewDialog::OutlookMeetingsViewDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Outlook Meetings View",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pMainSizer(nullptr)
    , pScrolledWindow(nullptr)
    , pRefreshButton(nullptr)
    , pActivityIndicator(nullptr)
    , pCancelButton(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void OutlookMeetingsViewDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
}

void OutlookMeetingsViewDialog::CreateControls()
{
    /* Main dialog sizer for controls */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Main Scrolled Window */
    pScrolledWindow = new wxScrolledWindow(this, wxID_ANY);
    pMainSizer->Add(pScrolledWindow, wxSizerFlags(1).Expand());

    auto scrolledSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(scrolledSizer);

    /* Today Date Label */
    pTodayDateLabel = new wxStaticText(pScrolledWindow, tksIDC_TODAYDATELABEL, wxGetEmptyString());
    auto todayDateLabelFont = pTodayDateLabel->GetFont();
    todayDateLabelFont.SetPointSize(14);
    pTodayDateLabel->SetFont(todayDateLabelFont);
    scrolledSizer->Add(pTodayDateLabel, wxSizerFlags().CenterHorizontal().Border(wxALL, 5).Top());

    /* Activity Indicator */
    pActivityIndicator = new wxActivityIndicator(pScrolledWindow, tksIDC_ACTIVITYINDICATOR);
    scrolledSizer->Add(pActivityIndicator, wxSizerFlags(1).Center());
}

void OutlookMeetingsViewDialog::ConfigureEventBindings() {}

void OutlookMeetingsViewDialog::FillControls() {}

void OutlookMeetingsViewDialog::OnRefresh(wxCommandEvent& event) {}

void OutlookMeetingsViewDialog::OnAttendedCheckboxCheck(wxCommandEvent& event) {}

void OutlookMeetingsViewDialog::FeedbackLabel(const wxString& message) {}

void OutlookMeetingsViewDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {}

void OutlookMeetingsViewDialog::QueueErrorNotificationEvent(const std::string& message) {}
} // namespace tks::UI::dlg
