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
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/listctrl.h>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

namespace tks
{
namespace Core
{
class Configuration;
struct TaskViewColumn;
} // namespace Core
namespace UI
{
namespace dlg
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
    void Save();
    void Reset();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnAvailableColumnItemCheck(wxListEvent& event);
    void OnAvailableColumnItemUncheck(wxListEvent& event);

    void OnAddAvailableColumnToDisplayColumnList(wxCommandEvent& event);
    void OnRemoveDisplayColumnToAvailableColumnList(wxCommandEvent& event);

    void OnDisplayColumnItemCheck(wxListEvent& event);
    void OnDisplayColumnItemUncheck(wxListEvent& event);

    void OnDisplayColumnItemRightClick(wxListEvent& event);

    void OnPopupMenuSortAscending(wxCommandEvent& event);
    void OnPopupMenuSortDescending(wxCommandEvent& event);

    void UpdateDisplayColumns();
    void UpdateDisplayColumnsOrderOnRemove();

    void SortDisplaysColumnsAsc();

    std::string GetDisplayColumnNameFromListItem(int itemIndex);
    std::string GetAvailableColumnNameFromListItem(int itemIndex);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxCheckBox* pTodayAlwaysExpanded;

    wxChoice* pDefaultColumnChoiceCtrl;

    wxListView* pAvailableColumnsListView;

    wxButton* pRightChevronButton;
    wxButton* pLeftChevronButton;

    wxListView* pDisplayColumnsListView;

    std::vector<long> mSelectedAvailableItemIndexes;
    std::vector<long> mSelectedDisplayItemIndexes;
    std::vector<Core::TaskViewColumn> mTaskViewColumns;

    long mItemIndexToSort;

    enum {
        tksIDC_TODAYALWAYSEXPANDED = wxID_HIGHEST + 1001,
        tksIDC_DEFAULTCOLUMNCHOICECTRL,
        tksIDC_AVAILABLECOLUMNSLISTVIEW,
        tksIDC_RIGHTCHEVRONBUTTON,
        tksIDC_LEFTCHEVRONBUTTON,
        tksIDC_DISPLAYCOLUMNSLISTVIEW,
        tksIDC_POP_SORTASC,
        tksIDC_POP_SORTDESC
    };
};
} // namespace dlg
} // namespace UI
} // namespace tks
