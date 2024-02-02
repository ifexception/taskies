// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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
#include <wx/dataview.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>

#include <spdlog/logger.h>

#include "../dataview/tasklistmodel.h"

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
} // namespace Core
namespace UI::dlg
{
class DayTaskViewDialog : public wxDialog
{
public:
    DayTaskViewDialog() = delete;
    DayTaskViewDialog(const DayTaskViewDialog&) = delete;
    DayTaskViewDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<Core::Environment> env,
        const std::string& databaseFilePath,
        const std::string& selectedDate,
        const wxString& name = "daytaskviewdlg");
    virtual ~DayTaskViewDialog() = default;

    DayTaskViewDialog& operator=(const DayTaskViewDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnDateChange(wxDateEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnCopy(wxCommandEvent& event);
    void OnCopyWithHeaders(wxCommandEvent& event);

    void QueueFetchTasksErrorNotificationEvent();

    void CopyTasksToClipboard(bool includeHeaders = false);

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;

    wxWindow* pParent;
    wxDatePickerCtrl* pDateCtrl;
    wxButton* pCopyButton;
    wxButton* pCopyWithHeadersButton;
    wxDataViewCtrl* pDataViewCtrl;
    wxObjectDataPtr<TaskListModel> pTaskListModel;

    std::string mDatabaseFilePath;
    std::string mSelectedDate;

    enum {
        tksIDC_DATEPICKERCTRL = wxID_HIGHEST + 100,
        tksIDC_COPYTASKS,
        tksIDC_COPYTASKSWITHEADERS,
        tksIDC_TASKDATAVIEWCTRL
    };
}; // namespace UI::dlg
} // namespace UI::dlg
} // namespace tks
