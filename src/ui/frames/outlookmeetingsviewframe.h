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

namespace tks::Core
{
class Configuration;
}

namespace tks::Services::Integrations
{
struct OutlookMeetingModel;
}

namespace tks::UI::frames
{
class OutlookMeetingsViewFrame : public wxFrame
{
public:
    OutlookMeetingsViewFrame() = delete;
    OutlookMeetingsViewFrame(const OutlookMeetingsViewFrame&) = delete;
    OutlookMeetingsViewFrame(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isMainFrameMaximized,
        const wxString& name = "outlookmeetingsviewdlg");
    virtual ~OutlookMeetingsViewFrame() = default;

    void OnParentFrameMove();
    void OnParentFrameResize();

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnRefresh(wxCommandEvent& event);
    void OnAccountChoice(wxCommandEvent& event);

    void OnAttendedCheckBoxCheck(wxCommandEvent& event);

    void QueueErrorNotificationEvent(const std::string& message);

    void RemoveActiveMeetingsPanel();
    void ResetFeedbackLabelOnNoData(const std::string& message = "");

    void AddMeetingControlsToPanel(wxBoxSizer* panelSizer,
        int* attendedCheckBoxControlId,
        const Services::Integrations::OutlookMeetingModel& meetingModel,
        bool meetingAttended);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    wxWindow* pParent;
    wxPanel* pThisPanel;

    wxBoxSizer* pMainSizer;

    wxBitmapButton* pRefreshButton;
    wxChoice* pAccountsChoiceCtrl;

    wxStaticText* pFeedbackLabel;

    wxScrolledWindow* pScrolledWindow;
    wxSizer* pScrolledWindowSizer;
    wxPanel* pActiveMeetingsPanel;

    std::string mSelectedAccount;

    std::vector<Services::Integrations::OutlookMeetingModel> mMeetingModels;
    bool bIsMainFrameMaximized;

    enum {
        tksIDC_REFRESH_BUTTON = wxID_HIGHEST + 1001,
        tksIDC_ACCOUNT_CHOICE_CTRL,
        tksIDC_FEEDBACKLABEL,
        tksIDC_ATTENDEDCHECKBOX_BASE
    };
};
} // namespace tks::UI::frames
