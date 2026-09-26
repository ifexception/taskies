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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <date/date.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

namespace tks::UI::Panel
{
class OutlookMeetingsPanel : public wxPanel
{
public:
    OutlookMeetingsPanel() = delete;
    OutlookMeetingsPanel(const OutlookMeetingsPanel&) = delete;
    OutlookMeetingsPanel(wxWindow* parent,
        wxWindowID windowPanelId,
        std::shared_ptr<spdlog::logger> logger);
    virtual ~OutlookMeetingsPanel();

    OutlookMeetingsPanel& operator=(const OutlookMeetingsPanel&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    // void DataToControls();

    void OnRefresh(wxCommandEvent& event);
    void OnAccountChoice(wxCommandEvent& event);

    std::shared_ptr<spdlog::logger> pLogger;

    wxStaticBoxSizer* pMeetingStaticBoxSizer;
    wxBitmapButton* pRefreshButton;
    wxChoice* pAccountsChoiceCtrl;

    wxStaticText* pFeedbackLabel;

    wxScrolledWindow* pScrolledWindow;
    wxSizer* pScrolledWindowSizer;

    date::sys_days mTodaysDate;

    enum {
        tksIDC_OUTLOOKMEETINGSPANELBASE = wxID_HIGHEST + 1001,
        tksIDC_REFRESH_BUTTON,
        tksIDC_ACCOUNT_CHOICE_CTRL,
        tksIDC_FEEDBACKLABEL,
        tksIDC_ATTENDEDCHECKBOX_BASE,
    };
};
} // namespace tks::UI::Panel
