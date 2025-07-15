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
#include <wx/dataview.h>
#include <wx/infobar.h>
#include <wx/notifmsg.h>
#include <wx/taskbarbutton.h>

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
    File_NewAttributeGroup,
    File_NewAttribute,
    File_NewStaticAttributes,
    File_TasksDatabaseBackup,
    File_TasksExportToCsv,
    File_TasksQuickExportToCsv,
    Edit_Employer,
    Edit_Client,
    Edit_Project,
    Edit_Category,
    Edit_AttributeGroup,
    Edit_Attribute,
    Edit_StaticAttributeValues,
    View_Reset,
    View_Expand,
    // View_Day,
    View_Preferences,
    Help_About,

    /* Popup Menu Ids */
    Pop_NewTask,
    Pop_ContainerCopyTasks,
    Pop_ContainerCopyTasksWithHeaders,
};

/* File */
static const int ID_NEW_TASK = static_cast<int>(MenuIds::File_NewTask);
static const int ID_NEW_EMPLOYER = static_cast<int>(MenuIds::File_NewEmployer);
static const int ID_NEW_CLIENT = static_cast<int>(MenuIds::File_NewClient);
static const int ID_NEW_PROJECT = static_cast<int>(MenuIds::File_NewProject);
static const int ID_NEW_CATEGORY = static_cast<int>(MenuIds::File_NewCategory);
static const int ID_NEW_ATTRIBUTEGROUP = static_cast<int>(MenuIds::File_NewAttributeGroup);
static const int ID_NEW_ATTRIBUTE = static_cast<int>(MenuIds::File_NewAttribute);
static const int ID_NEW_STATIC_ATTRIBUTES = static_cast<int>(MenuIds::File_NewStaticAttributes);

static const int ID_TASKS_BACKUPDATABASE = static_cast<int>(MenuIds::File_TasksDatabaseBackup);
static const int ID_TASKS_EXPORTTOCSV = static_cast<int>(MenuIds::File_TasksExportToCsv);
static const int ID_TASKS_QUICKEXPORTTOCSV = static_cast<int>(MenuIds::File_TasksQuickExportToCsv);

/* Edit */
static const int ID_EDIT_EMPLOYER = static_cast<int>(MenuIds::Edit_Employer);
static const int ID_EDIT_CLIENT = static_cast<int>(MenuIds::Edit_Client);
static const int ID_EDIT_PROJECT = static_cast<int>(MenuIds::Edit_Project);
static const int ID_EDIT_CATEGORY = static_cast<int>(MenuIds::Edit_Category);
static const int ID_EDIT_ATTRIBUTE_GROUP = static_cast<int>(MenuIds::Edit_AttributeGroup);
static const int ID_EDIT_ATTRIBUTE = static_cast<int>(MenuIds::Edit_Attribute);
static const int ID_EDIT_STATIC_ATTRIBUTE_VALUES =
    static_cast<int>(MenuIds::Edit_StaticAttributeValues);

/* View */
static const int ID_VIEW_RESET = static_cast<int>(MenuIds::View_Reset);
static const int ID_VIEW_EXPAND = static_cast<int>(MenuIds::View_Expand);
// static const int ID_VIEW_DAY = static_cast<int>(MenuIds::View_Day);
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
    void OnTaskReminder(wxTimerEvent& event);
    /* Taskbar Button Event Handlers */
    void OnThumbBarNewTask(wxCommandEvent& event);
    void OnThumbBarQuickExport(wxCommandEvent& event);
    /* Menu Event Handlers */
    void OnNewTask(wxCommandEvent& event);
    void OnNewEmployer(wxCommandEvent& event);
    void OnNewClient(wxCommandEvent& event);
    void OnNewProject(wxCommandEvent& event);
    void OnNewCategory(wxCommandEvent& event);
    void OnNewAttributeGroup(wxCommandEvent& event);
    void OnNewAttribute(wxCommandEvent& event);
    void OnNewStaticAttributes(wxCommandEvent& event);
    void OnTasksBackupDatabase(wxCommandEvent& event);
    void OnTasksExportToCsv(wxCommandEvent& event);
    void OnTasksQuickExportToCsv(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnEditEmployer(wxCommandEvent& event);
    void OnEditClient(wxCommandEvent& event);
    void OnEditProject(wxCommandEvent& event);
    void OnEditCategory(wxCommandEvent& event);
    void OnEditAttributeGroup(wxCommandEvent& event);
    void OnEditAttribute(wxCommandEvent& event);
    void OnEditStaticAttributeValues(wxCommandEvent& event);
    void OnViewReset(wxCommandEvent& event);
    void OnViewExpand(wxCommandEvent& event);
    // void OnViewDay(wxCommandEvent& event);
    void OnViewPreferences(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    /* Popup Menu Event Handlers */
    void OnPopupNewTask(wxCommandEvent& event);
    void OnContainerCopyTasksToClipboard(wxCommandEvent& event);
    void OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& event);
    void OnCopyTaskToClipboard(wxCommandEvent& event);
    void OnEditTask(wxCommandEvent& event);
    void OnDeleteTask(wxCommandEvent& event);
    void OnAddMinutes(wxCommandEvent& event);
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
    /* Notification Event Handlers */
    void OnReminderNotificationClicked(wxCommandEvent& event);

    void SetNewTaskMenubarTitle();

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

    wxThumbBarButton* pThumbBarNewTaskButton;
    wxThumbBarButton* pThumbBarQuickExportButton;

    wxInfoBar* pInfoBar;
    TaskBarIcon* pTaskBarIcon;
    StatusBar* pStatusBar;

    NotificationPopupWindow* pNotificationPopupWindow;

    wxDatePickerCtrl* pFromDatePickerCtrl;
    wxDatePickerCtrl* pToDatePickerCtrl;

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

    std::unique_ptr<wxTimer> pTaskReminderTimer;
    std::shared_ptr<wxNotificationMessage> pTaskReminderNotification;

    enum {
        tksIDC_THUMBBAR_NEWTASK = wxID_HIGHEST + 1000,
        tksIDC_THUMBBAR_QUICKEXPORT,
        tksIDC_NOTIFICATIONBUTTON,
        tksIDC_FROMDATE,
        tksIDC_TODATE,
        tksIDC_TASKDATAVIEWCTRL,
        tksIDC_DAY_TASKDATAVIEW,
        tksIDC_TASKREMINDERTIMER
    };
};
} // namespace UI
} // namespace tks
