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
#include <vector>
#include <utility>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <spdlog/logger.h>

#include "../../../common/common.h"
#include "../../../common/enums.h"

namespace tks
{
namespace Core
{
namespace Settings
{
struct TasksViewColumnSetting;
} // namespace Settings
class Configuration;
} // namespace Core
namespace UI::dlg
{
class PreferencesTasksViewPage : public wxPanel
{
public:
    PreferencesTasksViewPage() = delete;
    PreferencesTasksViewPage(const PreferencesTasksViewPage&) = delete;
    PreferencesTasksViewPage(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger);
    virtual ~PreferencesTasksViewPage() = default;

    PreferencesTasksViewPage& operator=(const PreferencesTasksViewPage&) = delete;

    bool IsValid();
    void Save(bool* restartRequired);
    void Reset();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnAvailableColumnCheck(wxCommandEvent& event);
    void OnSelectedColumnCheck(wxCommandEvent& event);
    void OnRightChevronButtonClick(wxCommandEvent& event);
    void OnLeftChevronButtonClick(wxCommandEvent& event);
    void OnAscButtonClick(wxCommandEvent& event);
    void OnDescButtonClick(wxCommandEvent& event);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxCheckBox* pTodayAlwaysExpanded;
    wxCheckBox* pUseProjectDisplayName;

    wxCheckListBox* pAvailableTasksViewColumns;
    wxButton* pRightChevronButton;
    wxButton* pLeftChevronButton;
    wxCheckListBox* pSelectedTasksViewColumns;
    wxButton* pAscSortButton;
    wxButton* pDescSortButton;

    wxTextCtrl* pSelectedColumnNameReadonlyTextCtrl;
    wxSpinCtrl* pSelectedColumnWidthSpinCtrl;
    wxChoice* pSelectedColumnTextAlignmentChoiceCtrl;
    wxChoice* pSelectedColumnTextEllipsizeChoiceCtrl;
    wxButton* pApplyButton;

    std::vector<std::pair<int, TasksViewColumnIdentifier>> mCheckedAvailableColumns;
    std::vector<std::pair<int, Core::Settings::TasksViewColumnSetting>> mCheckedSelectedColumns;

    std::vector<Core::Settings::TasksViewColumnSetting> mAllTasksViewColumns;
    std::vector<Core::Settings::TasksViewColumnSetting> mCfgTasksViewColumns;

    enum {
        tksIDC_TODAYALWAYSEXPANDED = wxID_HIGHEST + 1001,
        tksIDC_USEPROJECTDISPLAYNAME,
        tksIDC_AVAILABLETASKSVIEWCOLUMNS,
        tksIDC_RIGHTCHEVRONBUTTON,
        tksIDC_LEFTCHEVRONBUTTON,
        tksIDC_SELECTEDTASKSVIEWCOLUMNS,
        tksIDC_ASCSORTBUTTON,
        tksIDC_DESCSORTBUTTON,
        tksIDC_SELECTEDCOLUMNNAMEREADONLYTEXTCTRL,
        tksIDC_SELECTEDCOLUMNWIDTHSPINCTRL,
        tksIDC_SELECTEDCOLUMNTEXTALIGNMENTCHOICE,
        tksIDC_SELECTEDCOLUMNTEXTELLIPSIZECHOICE,
        tksIDC_APPLYBUTTON
    };
};
} // namespace UI::dlg
} // namespace tks
