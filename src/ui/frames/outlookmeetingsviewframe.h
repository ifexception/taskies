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

#pragma once

#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/datectrl.h>
#include <wx/dateevt.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "../../models/attendedmeetingmodel.h"

namespace tks::Core
{
class Configuration;
class Environment;
} // namespace tks::Core

namespace tks::Services::Outlook
{
struct OutlookMeetingModel;
}

namespace tks::UI::frames
{
class OutlookMeetingsViewFrame final : public wxFrame
{
public:
    OutlookMeetingsViewFrame() = delete;
    OutlookMeetingsViewFrame(const OutlookMeetingsViewFrame&) = delete;
    OutlookMeetingsViewFrame(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isMainFrameMaximized,
        const wxString& name = "outlookmeetingsviewdlg");
    virtual ~OutlookMeetingsViewFrame();

    void OnParentFrameMove();
    void OnParentFrameResize();

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnDateSelection(wxDateEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnEmployerChoice(wxCommandEvent& event);
    void OnAccountChoice(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnProjectChoice(wxCommandEvent& event);
    void OnAttendedCheckBoxCheck(wxCommandEvent& event);

    void FetchOutlookMeetingsAndUpdateFeedbackLabel();
    std::vector<Model::AttendedMeetingModel> FetchAttendedMeetings();
    void AddMeetingsToPanel(const std::vector<Model::AttendedMeetingModel>& attendedMeetingModels);
    void SetDialogSizeFromParent();

    void RemoveActiveMeetingsPanel();
    void ResetFeedbackLabelOnNoData(const std::string& message = "");

    void AddMeetingControlsToPanel(wxBoxSizer* panelSizer,
        int* attendedCheckBoxControlId,
        int* projectChoiceControlId,
        const Services::Outlook::OutlookMeetingModel& meetingModel,
        bool meetingAttended);

    /*void ResetProjectsChoiceControl(bool disable = false);
    void ResetCategoriesChoiceControl(bool disable = true);*/

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    wxWindow* pParent;
    wxPanel* pThisPanel;

    wxBoxSizer* pMainSizer;

    wxDatePickerCtrl* pDatePickerCtrl;
    wxBitmapButton* pRefreshButton;

    wxChoice* pEmployerChoiceCtrl;
    wxChoice* pAccountsChoiceCtrl;

    wxStaticText* pFeedbackLabel;

    wxScrolledWindow* pScrolledWindow;
    wxSizer* pScrolledWindowSizer;
    wxPanel* pActiveMeetingsPanel;

    std::string mSelectedAccount;
    std::string mSelectedDate;
    std::vector<Services::Outlook::OutlookMeetingModel> mMeetingModels;
    bool bIsMainFrameMaximized;
    std::int64_t mEmployerId;

    enum {
        tksIDC_DATEPICKERCTRL = wxID_HIGHEST + 1001,
        tksIDC_EMPLOYERCHOICECTRL,
        tksIDC_REFRESH_BUTTON,
        tksIDC_ACCOUNT_CHOICE_CTRL,
        tksIDC_FEEDBACKLABEL,
        tksIDC_ATTENDEDCHECKBOX_BASE,
    };

    enum {
        tksIDC_PROJECTSCHOICECTRL_BASE = wxID_HIGHEST + 1256,
    };

    enum {
        tksIDC_CATEGORIESCHOICECTRL_BASE = wxID_HIGHEST + 1556,
    };
};
} // namespace tks::UI::frames
