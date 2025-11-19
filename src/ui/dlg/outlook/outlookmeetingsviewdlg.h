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

#pragma once

#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/activityindicator.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

namespace tks::UI::dlg
{
class OutlookMeetingsViewDialog : public wxDialog
{
public:
    OutlookMeetingsViewDialog() = delete;
    OutlookMeetingsViewDialog(const OutlookMeetingsViewDialog&) = delete;
    OutlookMeetingsViewDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        const wxString& name = "outlookmeetingsviewdlg");
    virtual ~OutlookMeetingsViewDialog() = default;

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnRefresh(wxCommandEvent& event);
    void OnAccountChoice(wxCommandEvent& event);

    void OnCancel(wxCommandEvent& event);

    void SetFeedbackLabelOnEvent(const std::string& message);

    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    wxWindow* pParent;

    wxBoxSizer* pMainSizer;

    wxBitmapButton* pRefreshButton;
    wxChoice* pAccountsChoiceCtrl;

    wxStaticText* pFeedbackLabel;

    wxScrolledWindow* pScrolledWindow;
    wxSizer* pScrolledWindowSizer;
    wxPanel* pActiveMeetingsPanel;

    wxButton* pCancelButton;

    std::string mSelectedAccount;

    enum {
        tksIDC_REFRESH_BUTTON = wxID_HIGHEST + 1001,
        tksIDC_ACCOUNT_CHOICE_CTRL,
        tksIDC_FEEDBACKLABEL
    };
};
} // namespace tks::UI::dlg
