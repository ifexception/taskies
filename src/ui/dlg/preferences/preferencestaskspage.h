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

#include <spdlog/logger.h>

namespace tks
{
namespace Core
{
class Configuration;
} // namespace Core
namespace UI
{
namespace dlg
{
class PreferencesTasksPage : public wxPanel
{
public:
    PreferencesTasksPage() = delete;
    PreferencesTasksPage(const PreferencesTasksPage&) = delete;
    PreferencesTasksPage(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger);
    virtual ~PreferencesTasksPage() = default;

    PreferencesTasksPage& operator=(const PreferencesTasksPage&) = delete;

    bool IsValid();
    void Save();
    void Reset();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnUseRemindersCheck(wxCommandEvent& event);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxChoice* pMinutesIncrementChoiceCtrl;
    wxCheckBox* pShowProjectAssociatedCategoriesCheckBoxCtrl;
    wxCheckBox* pUseLegacyTaskDialogCheckBoxCtrl;
    wxCheckBox* pUseRemindersCheckBoxCtrl;
    wxCheckBox* pUseNotificationBanners;
    wxCheckBox* pUseTaskbarFlashing;
    wxChoice* pReminderIntervalChoiceCtrl;
    wxCheckBox* pOpenTaskDialogOnReminderClickCheckBoxCtrl;

    enum {
        tksIDC_MINUTES_INCREMENT = wxID_HIGHEST + 100,
        tksIDC_ASSOCIATEDCATEGORIES,
        tksIDC_USELEGACYTASKDIALOGCHECKBOXCTRL,
        tksIDC_USEREMINDERSCHECKBOXCTRL,
        tksIDC_USENOTIFICATIONBANNERS,
        tksIDC_USETASKBARFLASHING,
        tksIDC_REMINDERINTERVALCHOICECTRL,
        tksIDC_OPENTASKDIALOGONREMINDERCLICKCHECKBOXCTRL
    };
};
} // namespace dlg
} // namespace UI
} // namespace tks
