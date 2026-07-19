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
#include <wx/listctrl.h>
#include <wx/infobar.h>
#include <wx/notifmsg.h>
#include <wx/taskbarbutton.h>
#include <wx/power.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "../common/enums.h"

#include "../ui/frames/outlookmeetingsviewframe.h"

#include "../models/taskmodel.h"

#include "../utils/datestore.h"

#include "taskbaricon.h"
#include "statusbar.h"

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
    File_TasksExportToExcel,
    File_TasksQuickExportToCsv,
    Edit_Employer,
    Edit_Client,
    Edit_Project,
    Edit_Category,
    Edit_AttributeGroup,
    Edit_Attribute,
    Edit_StaticAttributeValues,
    View_Reset,
    View_Outlook,
    // View_Day,
    View_Preferences,
    Help_About,

    /* Popup Menu Ids */
    Pop_CloneTask,
    Pop_NewTask,
    Pop_ContainerCopyTasks,
    Pop_ContainerCopyTasksWithHeaders,
    Pop_ContainerCopyTasksPreset,
    Pop_CopyRowTask,
    Pop_CopyRowTaskPreset
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
static const int ID_TASKS_EXPORTTOEXCEL = static_cast<int>(MenuIds::File_TasksExportToExcel);
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
static const int ID_VIEW_OUTLOOK = static_cast<int>(MenuIds::View_Outlook);
static const int ID_VIEW_PREFERENCES = static_cast<int>(MenuIds::View_Preferences);

/* Help */
static const int ID_HELP_ABOUT = static_cast<int>(MenuIds::Help_About);

/* Popup Menu Ids */
static const int ID_POP_CLONE_TASK = static_cast<int>(MenuIds::Pop_CloneTask);
static const int ID_POP_NEW_TASK = static_cast<int>(MenuIds::Pop_NewTask);

static const int ID_POP_CONTAINER_COPY_TASKS = static_cast<int>(MenuIds::Pop_ContainerCopyTasks);
static const int ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS =
    static_cast<int>(MenuIds::Pop_ContainerCopyTasksWithHeaders);
static const int ID_POP_CONTAINER_COPY_TASKS_PRESET =
    static_cast<int>(MenuIds::Pop_ContainerCopyTasksPreset);
static const int ID_POP_COPY_ROW_TASK = static_cast<int>(MenuIds::Pop_CopyRowTask);
static const int ID_POP_COPY_ROW_TASK_PRESET = static_cast<int>(MenuIds::Pop_CopyRowTaskPreset);

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
    void OnMove(wxMoveEvent& event);
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
    void OnTasksExportToExcel(wxCommandEvent& event);
    void OnTasksQuickExportToFormat(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnEditEmployer(wxCommandEvent& event);
    void OnEditClient(wxCommandEvent& event);
    void OnEditProject(wxCommandEvent& event);
    void OnEditCategory(wxCommandEvent& event);
    void OnEditAttributeGroup(wxCommandEvent& event);
    void OnEditAttribute(wxCommandEvent& event);
    void OnEditStaticAttributeValues(wxCommandEvent& event);
    void OnViewReset(wxCommandEvent& event);
    void OnViewOutlook(wxCommandEvent& event);
    void OnViewPreferences(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    /* Popup Menu Event Handlers */
    void OnPopupNewTask(wxCommandEvent& event);
    void OnContainerCopyTasksToClipboard(wxCommandEvent& event);
    void OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& event);
    void OnContainerCopyTasksUsingPreset(wxCommandEvent& event);
    void OnCopyTaskDescriptionToClipboard(wxCommandEvent& event);
    void OnCopyRowTaskToClipboard(wxCommandEvent& event);
    void OnCopyRowTaskToClipboardWithPreset(wxCommandEvent& event);
    void OnEditTask(wxCommandEvent& event);
    void OnDeleteTask(wxCommandEvent& event);
    void OnCloneTask(wxCommandEvent& event);
    void OnAddMinutes(wxCommandEvent& event);
    void OnMenuHighlight(wxMenuEvent& event);
    /* Custom Event Handlers */
    void OnTaskInserted(wxCommandEvent& event);
    void OnTaskDateChanged(wxCommandEvent& event);
    void OnTaskUpdated(wxCommandEvent& event);
    void OnTaskDeleted(wxCommandEvent& event);
    void OnOutlookMeetingViewClose(wxCommandEvent& event);
    /* Control Event Handlers */
    void OnDateChanged(wxDateEvent& event);
    /* ListCtrl Event Handlers */
    void OnItemRightClick(wxListEvent& event);
    void OnColumnEndDrag(wxListEvent& event);
    void OnColumnRightClick(wxListEvent& event);
    /* Notification Event Handlers */
    void OnReminderNotificationClicked(wxCommandEvent& event);
    /* Power Event Handlers */
    void OnPowerResume(wxPowerEvent& event);

    void DoResetToCurrentWeekAndOrToday();

    void CalculateStatusBarTaskDurations();
    void CalculateDefaultTaskDurations();
    void CalculateBillableTaskDurations();

    void UpdateDefaultWeekMonthTaskDurations();
    void UpdateBillableWeekMonthTaskDurations();

    void UpdateDefaultRangeTaskDurations();
    void UpdateBillableRangeTaskDurations();

    void TryUpdateSelectedDateAndAllTaskDurations(const std::string& date);
    void UpdateSelectedDayStatusBarTaskDurations(const std::string& date);

    void RefreshListControlTaskItem(const std::int64_t taskId);
    void RefreshListControlTaskItems();

    //void AdjustColumnWidths();
    void ResetTaskContextMenuVariables();

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::string mDatabaseFilePath;

    frames::OutlookMeetingsViewFrame* pMeetingsViewFrame;

    wxThumbBarButton* pThumbBarNewTaskButton;
    wxThumbBarButton* pThumbBarQuickExportButton;

    wxInfoBar* pInfoBar;
    TaskBarIcon* pTaskBarIcon;
    StatusBar* pStatusBar;

    wxDatePickerCtrl* pDatePickerCtrl;

    wxListCtrl* pListCtrl;

    std::unique_ptr<DateStore> pDateStore;

    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate;

    std::int64_t mTaskIdToEdit;
    std::string mTaskDate;
    long mItemIndex;

    /*
     * this variable ensures that only one dialog is opened at a time from the thumb bar actions
     * the thumb bar allows a user to open an as many dialogs as they want so we cap at 1
     */
    int mThumbBarDialogOpenCounter;

    int mOutlookMeetingViewFrameOpenCounter;

    std::unique_ptr<wxTimer> pTaskReminderTimer;
    std::shared_ptr<wxNotificationMessage> pTaskReminderNotification;

    enum {
        tksIDC_THUMBBAR_NEWTASK = wxID_HIGHEST + 1000,
        tksIDC_THUMBBAR_QUICKEXPORT,
        tksIDC_DATEPICKERCTRL,
        tksIDC_LISTCTRL,
        tksIDC_TASKREMINDERTIMER
    };
};
} // namespace UI
} // namespace tks
