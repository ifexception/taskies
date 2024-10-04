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

#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <date/date.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/infobar.h>
#include <wx/dataview.h>

#include <spdlog/spdlog.h>

#include "../common/enums.h"

#include "../ui/dataview/tasktreemodel.h"
#include "../ui/dataview/tasklistmodel.h"

#include "../models/taskmodel.h"

#include "../utils/datestore.h"

#include "notificationpopupwindow.h"
#include "taskbaricon.h"
#include "statusbar.h"

wxDateTime MakeMaximumFromDate();

namespace tks
{
enum class MenuIds : int {
    File_NewTask = wxID_HIGHEST + 102,
    File_NewEmployer,
    File_NewClient,
    File_NewProject,
    File_NewCategory,
    File_TasksDatabaseBackup,
    File_TasksExportToCsv,
    Edit_Employer,
    Edit_Client,
    Edit_Project,
    Edit_Category,
    View_Reset,
    View_Expand,
    View_Day,
    View_Preferences,
    Help_About,

    /* Popup Menu Ids */
    Pop_NewTask,
    Pop_ContainerCopyTasks,
    Pop_ContainerCopyTasksWithHeaders,

    /* Keyboard shortcuts */
    Kyb_LeftArrow,
    Kyb_RightArrow,
};

/* File */
static const int ID_NEW_TASK = static_cast<int>(MenuIds::File_NewTask);
static const int ID_NEW_EMPLOYER = static_cast<int>(MenuIds::File_NewEmployer);
static const int ID_NEW_CLIENT = static_cast<int>(MenuIds::File_NewClient);
static const int ID_NEW_PROJECT = static_cast<int>(MenuIds::File_NewProject);
static const int ID_NEW_CATEGORY = static_cast<int>(MenuIds::File_NewCategory);
static const int ID_TASKS_BACKUPDATABASE = static_cast<int>(MenuIds::File_TasksDatabaseBackup);
static const int ID_TASKS_EXPORTTOCSV = static_cast<int>(MenuIds::File_TasksExportToCsv);

/* Edit */
static const int ID_EDIT_EMPLOYER = static_cast<int>(MenuIds::Edit_Employer);
static const int ID_EDIT_CLIENT = static_cast<int>(MenuIds::Edit_Client);
static const int ID_EDIT_PROJECT = static_cast<int>(MenuIds::Edit_Project);
static const int ID_EDIT_CATEGORY = static_cast<int>(MenuIds::Edit_Category);

/* View */
static const int ID_VIEW_RESET = static_cast<int>(MenuIds::View_Reset);
static const int ID_VIEW_EXPAND = static_cast<int>(MenuIds::View_Expand);
static const int ID_VIEW_DAY = static_cast<int>(MenuIds::View_Day);
static const int ID_VIEW_PREFERENCES = static_cast<int>(MenuIds::View_Preferences);

/* Help */
static const int ID_HELP_ABOUT = static_cast<int>(MenuIds::Help_About);

/* Popup Menu Ids */
static const int ID_POP_NEW_TASK = static_cast<int>(MenuIds::Pop_NewTask);
static const int ID_POP_CONTAINER_COPY_TASKS = static_cast<int>(MenuIds::Pop_ContainerCopyTasks);
static const int ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS =
    static_cast<int>(MenuIds::Pop_ContainerCopyTasksWithHeaders);

static const int MAX_EXPAND_COUNT = 3;

namespace Core
{
class Environment;
class Configuration;
} // namespace Core

namespace UI
{
class MainFrame : public wxFrame
{
public:
    MainFrame() = delete;
    MainFrame(const MainFrame&) = delete;
    MainFrame(std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const wxString& name = "mainfrm");
    virtual ~MainFrame();

    MainFrame& operator=(const MainFrame&) = delete;

private:
    wxDECLARE_EVENT_TABLE();

    void Create();

    void CreateControls();
    void FillControls();
    void DataToControls();

    /* Event Table Handlers */
    /* General Event Handlers */
    void OnClose(wxCloseEvent& event);
    void OnIconize(wxIconizeEvent& event);
    void OnResize(wxSizeEvent& event);
    /* Menu Event Handlers */
    void OnNewTask(wxCommandEvent& event);
    void OnNewEmployer(wxCommandEvent& event);
    void OnNewClient(wxCommandEvent& event);
    void OnNewProject(wxCommandEvent& event);
    void OnNewCategory(wxCommandEvent& event);
    void OnTasksBackupDatabase(wxCommandEvent& event);
    void OnTasksExportToCsv(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnEditEmployer(wxCommandEvent& event);
    void OnEditClient(wxCommandEvent& event);
    void OnEditProject(wxCommandEvent& event);
    void OnEditCategory(wxCommandEvent& event);
    void OnViewReset(wxCommandEvent& event);
    void OnViewExpand(wxCommandEvent& event);
    void OnViewDay(wxCommandEvent& event);
    void OnViewPreferences(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    /* Popup Menu Event Handlers */
    void OnPopupNewTask(wxCommandEvent& event);
    void OnContainerCopyTasksToClipboard(wxCommandEvent& event);
    void OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& event);
    void OnCopyTaskToClipboard(wxCommandEvent& event);
    void OnEditTask(wxCommandEvent& event);
    void OnDeleteTask(wxCommandEvent& event);
    /* Error Event Handlers */ /*TODO(SW): Is this still relevant?*/
    void OnError(wxCommandEvent& event);
    /* Custom Event Handlers */
    void OnAddNotification(wxCommandEvent& event);
    void OnTaskAddedOnDate(wxCommandEvent& event);
    void OnTaskDeletedOnDate(wxCommandEvent& event);
    void OnTaskDateChangedFrom(wxCommandEvent& event);
    void OnTaskDateChangedTo(wxCommandEvent& event);
    /* Control Event Handlers */
    void OnNotificationClick(wxCommandEvent& event);
    void OnFromDateSelection(wxDateEvent& event);
    void OnToDateSelection(wxDateEvent& event);
    /* DataViewCtrl Event Handlers */
    void OnContextMenu(wxDataViewEvent& event);
    void OnDataViewSelectionChanged(wxDataViewEvent& event);

    void DoResetToCurrentWeekAndOrToday();
    void ResetDateRange();
    void ResetDatePickerValues();
    void RefetchTasksForDateRange();
    void RefetchTasksForDate(const std::string& date, const std::int64_t taskId);

    void CalculateStatusBarTaskDurations();
    void CalculateDefaultTaskDurations();
    void CalculateBillableTaskDurations();

    void UpdateDefaultWeekMonthTaskDurations();
    void UpdateBillableWeekMonthTaskDurations();

    void UpdateDefaultRangeTaskDurations();
    void UpdateBillableRangeTaskDurations();

    void TryUpdateSelectedDateAndAllTaskDurations(const std::string& date);
    void UpdateSelectedDayStatusBarTaskDurations(const std::string& date);

    void QueueFetchTasksErrorNotificationEvent();

    void SetFromAndToDatePickerRanges();
    void SetFromDateAndDatePicker();
    void SetToDateAndDatePicker();

    void ResetTaskContextMenuVariables();

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::string mDatabaseFilePath;

    wxInfoBar* pInfoBar;
    TaskBarIcon* pTaskBarIcon;
    StatusBar* pStatusBar;

    NotificationPopupWindow* pNotificationPopupWindow;
    wxDatePickerCtrl* pFromDateCtrl;
    wxDatePickerCtrl* pToDateCtrl;
    wxBitmapButton* pNotificationButton;
    wxBitmap mBellBitmap;
    wxBitmap mBellNotificationBitmap;
    std::unique_ptr<DateStore> pDateStore;
    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate;
    wxDataViewCtrl* pDataViewCtrl;
    wxObjectDataPtr<TaskTreeModel> pTaskTreeModel;
    wxDateTime mFromCtrlDate;
    wxDateTime mToCtrlDate;
    wxDateTime mToLatestPossibleDate;
    std::int64_t mTaskIdToModify;
    std::string mTaskDate;
    int mExpandCounter;
    bool bDateRangeChanged;

    enum {
        tksIDC_NOTIFICATIONBUTTON = wxID_HIGHEST + 1000,
        tksIDC_FROMDATE,
        tksIDC_TODATE,
        tksIDC_TASKDATAVIEWCTRL,
        tksIDC_DAY_TASKDATAVIEW
    };
};
} // namespace UI
} // namespace tks
