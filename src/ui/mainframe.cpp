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

#include "mainframe.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <vector>

#include <date/date.h>

#include <sqlite3.h>

#include <spdlog/spdlog.h>

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/taskbarbutton.h>

#include "../common/common.h"
#include "../common/constants.h"
#include "../common/logmessages.h"
#include "../common/queryhelper.h"
#include "../common/enums.h"
#include "../common/version.h"
#include "../common/wxcommon.h"

#include "../common/messages/operationmessages.h"
#include "../common/messages/persistencemessages.h"
#include "../common/messages/sqlitemessages.h"

#include "../core/environment.h"
#include "../core/configuration.h"
#include "../core/database_backup.h"
#include "../core/database_optimizer.h"
#include "../core/zip_database_backup.h"

#include "../persistence/attendedmeetingspersistence.h"
#include "../persistence/taskattributevaluespersistence.h"
#include "../persistence/taskspersistence.h"

#include "../services/export/columnexportmodel.h"
#include "../services/export/csvexporterservice.h"
#include "../services/export/projectionbuilder.h"
#include "../services/tasks/taskviewmodel.h"
#include "../services/tasks/tasksservice.h"

#include "../utils/mswutils.h"
#include "../utils/utils.h"

#include "../ui/dlg/employerdlg.h"
#include "../ui/dlg/editlistdlg.h"
#include "../ui/dlg/clientdlg.h"
#include "../ui/dlg/projectdlg.h"
#include "../ui/dlg/categoriesdlg.h"
#include "../ui/dlg/aboutdlg.h"
#include "../ui/dlg/preferences/preferencesdlg.h"
#include "../ui/dlg/exports/exporttocsvdlg.h"
#include "../ui/dlg/exports/exporttoexceldlg.h"
#include "../ui/dlg/exports/quickexporttoformatdlg.h"
#include "../ui/dlg/taskdlg.h"
#include "../ui/dlg/attributes/attributegroupdlg.h"
#include "../ui/dlg/attributes/attributedlg.h"
#include "../ui/dlg/attributes/staticattributevaluesdlg.h"

#include "events.h"

namespace tks::UI
{
// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
/* General Event Handlers */
EVT_CLOSE(MainFrame::OnClose)
EVT_ICONIZE(MainFrame::OnIconize)
EVT_SIZE(MainFrame::OnResize)
EVT_TIMER(tksIDC_TASKREMINDERTIMER, MainFrame::OnTaskReminder)
EVT_MOVE(MainFrame::OnMove)
/* Taskbar Button (thumbbar) Event Handlers */
EVT_BUTTON(tksIDC_THUMBBAR_NEWTASK, MainFrame::OnThumbBarNewTask)
EVT_BUTTON(tksIDC_THUMBBAR_QUICKEXPORT, MainFrame::OnThumbBarQuickExport)
/* Menu Event Handlers */
EVT_MENU(ID_NEW_TASK, MainFrame::OnNewTask)
EVT_MENU(ID_NEW_EMPLOYER, MainFrame::OnNewEmployer)
EVT_MENU(ID_NEW_CLIENT, MainFrame::OnNewClient)
EVT_MENU(ID_NEW_PROJECT, MainFrame::OnNewProject)
EVT_MENU(ID_NEW_CATEGORY, MainFrame::OnNewCategory)
EVT_MENU(ID_NEW_ATTRIBUTEGROUP, MainFrame::OnNewAttributeGroup)
EVT_MENU(ID_NEW_ATTRIBUTE, MainFrame::OnNewAttribute)
EVT_MENU(ID_NEW_STATIC_ATTRIBUTES, MainFrame::OnNewStaticAttributes)
EVT_MENU(ID_TASKS_BACKUPDATABASE, MainFrame::OnTasksBackupDatabase)
EVT_MENU(ID_TASKS_EXPORTTOCSV, MainFrame::OnTasksExportToCsv)
EVT_MENU(ID_TASKS_EXPORTTOEXCEL, MainFrame::OnTasksExportToExcel)
EVT_MENU(ID_TASKS_QUICKEXPORTTOCSV, MainFrame::OnTasksQuickExportToFormat)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_EDIT_EMPLOYER, MainFrame::OnEditEmployer)
EVT_MENU(ID_EDIT_CLIENT, MainFrame::OnEditClient)
EVT_MENU(ID_EDIT_PROJECT, MainFrame::OnEditProject)
EVT_MENU(ID_EDIT_CATEGORY, MainFrame::OnEditCategory)
EVT_MENU(ID_EDIT_ATTRIBUTE_GROUP, MainFrame::OnEditAttributeGroup)
EVT_MENU(ID_EDIT_ATTRIBUTE, MainFrame::OnEditAttribute)
EVT_MENU(ID_EDIT_STATIC_ATTRIBUTE_VALUES, MainFrame::OnEditStaticAttributeValues)
EVT_MENU(ID_VIEW_RESET, MainFrame::OnViewReset)
EVT_MENU(ID_VIEW_OUTLOOK, MainFrame::OnViewOutlook)
EVT_MENU(ID_VIEW_PREFERENCES, MainFrame::OnViewPreferences)
EVT_MENU(ID_HELP_ABOUT, MainFrame::OnAbout)
/* Popup Menu Event Handlers */
EVT_MENU(ID_POP_NEW_TASK, MainFrame::OnPopupNewTask)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS, MainFrame::OnContainerCopyTasksToClipboard)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS, MainFrame::OnContainerCopyTasksWithHeadersToClipboard)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS_PRESET, MainFrame::OnContainerCopyTasksUsingPreset)
EVT_MENU(wxID_COPY, MainFrame::OnCopyTaskDescriptionToClipboard)
EVT_MENU(ID_POP_COPY_ROW_TASK_PRESET, MainFrame::OnCopyRowTaskToClipboardWithPreset)
EVT_MENU(wxID_EDIT, MainFrame::OnEditTask)
EVT_MENU(wxID_DELETE, MainFrame::OnDeleteTask)
EVT_MENU(ID_POP_CLONE_TASK, MainFrame::OnCloneTask)
EVT_MENU(wxID_ADD, MainFrame::OnAddMinutes)
/* Custom Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_TASKINSERTED, MainFrame::OnTaskInserted)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATECHANGED, MainFrame::OnTaskDateChanged)
EVT_COMMAND(wxID_ANY, tksEVT_TASKUPDATED, MainFrame::OnTaskUpdated)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDELETED, MainFrame::OnTaskDeleted)
EVT_COMMAND(wxID_ANY, tksEVT_OUTLOOKMEETINGSFRMCLOSED, MainFrame::OnOutlookMeetingViewClose)
/* Ctrl Event Handlers */
EVT_DATE_CHANGED(tksIDC_DATEPICKERCTRL, MainFrame::OnDateChanged)
/* List Ctrl Event Handlers */
EVT_LIST_ITEM_RIGHT_CLICK(tksIDC_LISTCTRL, MainFrame::OnItemRightClick)
EVT_LIST_COL_END_DRAG(tksIDC_LISTCTRL, MainFrame::OnColumnEndDrag)
EVT_LIST_COL_RIGHT_CLICK(tksIDC_LISTCTRL, MainFrame::OnColumnRightClick)
/* Power Event Handlers */
EVT_POWER_RESUME(MainFrame::OnPowerResume)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxFrame(nullptr,
        wxID_ANY,
        Common::GetProgramName(),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE,
        name)
    , pLogger(logger)
    , pEnv(env)
    , pCfg(cfg)
    , mDatabaseFilePath()
    , pInfoBar(nullptr)
    , pTaskBarIcon(nullptr)
    , pStatusBar(nullptr)
    , pDatePickerCtrl(nullptr)
    , pListCtrl(nullptr)
    , pDateStore(nullptr)
    , mFromDate()
    , mToDate()
    , mTaskIdToEdit(-1)
    , mTaskDate()
    //, bDateRangeChanged(false)
    , pTaskReminderTimer(std::make_unique<wxTimer>(this, tksIDC_TASKREMINDERTIMER))
    , pTaskReminderNotification()
    , pThumbBarNewTaskButton(nullptr)
    , pThumbBarQuickExportButton(nullptr)
    , mThumbBarDialogOpenCounter(0)
    , mOutlookMeetingViewFrameOpenCounter(0)
    , pMeetingsViewFrame(nullptr)
// clang-format on
{
    // Initialization setup
    SetMinSize(wxSize(FromDIP(320), FromDIP(320)));
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        pLogger->info(
            "No persistence information found for MainFrame. Use default size \"{0}\"x\"{1}\"",
            800,
            600);
        SetSize(FromDIP(wxSize(800, 600)));
    }

    // Set main icon in titlebar
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);

    // Load database path
    mDatabaseFilePath = pCfg->BuildFullDatabaseFilePath();
    SPDLOG_LOGGER_TRACE(pLogger, "Database location \"{0}\"", mDatabaseFilePath);

    // Setup TaskBarIcon
    pTaskBarIcon = new TaskBarIcon(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
    if (pCfg->ShowInTray()) {
        pTaskBarIcon->SetTaskBarIcon();
    }

    // Setup StatusBar
    pStatusBar = new StatusBar(this, pLogger, mDatabaseFilePath);

    // Setup DateStore
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;
    mTaskDate = pDateStore->PrintTodayDate;

    // Setup reminders (if enabled)
    if (pCfg->UseReminders()) {
        pTaskReminderTimer->Start(Utils::ConvertMinutesToMilliseconds(pCfg->ReminderInterval()));

        wxNotificationMessage::UseTaskBarIcon(pTaskBarIcon);
    }

    // ThumbBar button actions
    wxIconBundle addTaskIconBundle(Common::GetAddTaskIconBundleName(), 0);
    wxIcon newTaskIcon = addTaskIconBundle.GetIcon(wxSize(16, 16));
    pThumbBarNewTaskButton =
        new wxThumbBarButton(tksIDC_THUMBBAR_NEWTASK, newTaskIcon, "New Task ");

    wxIconBundle quickExportBundle(Common::GetQuickExportIconBundleName(), 0);
    wxIcon quickExportIcon = quickExportBundle.GetIcon(wxSize(16, 16));
    pThumbBarQuickExportButton =
        new wxThumbBarButton(tksIDC_THUMBBAR_QUICKEXPORT, quickExportIcon, "Quick Export ");

    MSWGetTaskBarButton()->AppendThumbBarButton(pThumbBarNewTaskButton);
    MSWGetTaskBarButton()->AppendThumbBarButton(pThumbBarQuickExportButton);

    // Create, fill, and set data to controls
    Create();
}

MainFrame::~MainFrame()
{
    if (pTaskBarIcon) {
        pTaskBarIcon->RemoveIcon();
        delete pTaskBarIcon;
    }

    if (pStatusBar) {
        delete pStatusBar;
    }

    if (pCfg->UseReminders() && pTaskReminderTimer->IsRunning()) {
        pTaskReminderTimer->Stop();
    }
}

void MainFrame::Create()
{
    CreateControls();
    FillControls();
    DataToControls();
}

void MainFrame::CreateControls()
{
    /* Menu Controls */
    /* Menubar */
    /* File */
    auto fileMenu = new wxMenu();

    auto newTaskMenuItem = fileMenu->Append(ID_NEW_TASK, "&New Task\tCtrl-N", "Create new task");

    wxIconBundle addTaskIconBundle(Common::GetAddTaskIconBundleName(), 0);
    newTaskMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(addTaskIconBundle));

    fileMenu->AppendSeparator();
    auto fileNewMenu = new wxMenu();
    fileNewMenu->Append(ID_NEW_EMPLOYER, "New Employer", "Create new employer");
    fileNewMenu->Append(ID_NEW_CLIENT, "New Client", "Create new client");
    fileNewMenu->Append(ID_NEW_PROJECT, "New Project", "Create new project");
    fileNewMenu->Append(ID_NEW_CATEGORY, "New Category", "Create new category");
    fileNewMenu->AppendSeparator();
    fileNewMenu->Append(ID_NEW_ATTRIBUTEGROUP, "New Attribute Group", "Create new attribute group");
    fileNewMenu->Append(ID_NEW_ATTRIBUTE, "New Attribute", "Create new attribute");
    fileNewMenu->Append(
        ID_NEW_STATIC_ATTRIBUTES, "New Static Attributes", "Create new static attribute values");
    fileMenu->AppendSubMenu(fileNewMenu, "New");
    fileMenu->AppendSeparator();

    auto fileTasksMenu = new wxMenu();
    auto fileTasksMenuItem = fileTasksMenu->Append(
        ID_TASKS_BACKUPDATABASE, "&Backup Database", "Backup a copy of the database");
    if (!pCfg->BackupDatabase()) {
        fileTasksMenuItem->Enable(false);
    }
    fileTasksMenu->AppendSeparator();
    fileTasksMenu->Append(
        ID_TASKS_EXPORTTOCSV, "E&xport to CSV", "Export tasks data to CSV file/clipboard");

    MswUtils::ExcelInstanceCheck isExcelInstalled;
    if (isExcelInstalled()) {
        fileTasksMenu->Append(
            ID_TASKS_EXPORTTOEXCEL, "Ex&port to Excel", "Export tasks data to Excel");
    }

    auto quickExportMenuItem = fileTasksMenu->Append(
        ID_TASKS_QUICKEXPORTTOCSV, "&Quick Export", "Export tasks data to CSV or Excel");

    wxIconBundle quickExportBundle(Common::GetQuickExportIconBundleName(), 0);
    quickExportMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(quickExportBundle));

    fileMenu->AppendSubMenu(fileTasksMenu, "Tasks");
    fileMenu->AppendSeparator();

    auto exitMenuItem = fileMenu->Append(wxID_EXIT, "Ex&it\tAlt-F4", "Exit the program");

    wxIconBundle exitIconBundle(Common::GetExitIconBundleName(), 0);
    exitMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(exitIconBundle));

    /* Edit */
    auto editMenu = new wxMenu();
    editMenu->Append(ID_EDIT_EMPLOYER, "Edit Employer", "Edit an employer");
    editMenu->Append(ID_EDIT_CLIENT, "Edit Client", "Edit a client");
    editMenu->Append(ID_EDIT_PROJECT, "Edit Project", "Edit a project");
    editMenu->Append(ID_EDIT_CATEGORY, "Edit Category", "Edit a category");
    editMenu->AppendSeparator();
    editMenu->Append(ID_EDIT_ATTRIBUTE_GROUP, "Edit Attribute Group", "Edit an attribute group");
    editMenu->Append(ID_EDIT_ATTRIBUTE, "Edit Attribute", "Edit an attribute");
    editMenu->Append(
        ID_EDIT_STATIC_ATTRIBUTE_VALUES, "Edit Static Attributes", "Edit static attribute values");

    /* View */
    auto viewMenu = new wxMenu();
    viewMenu->Append(ID_VIEW_RESET, "&Reset View\tCtrl-R", "Reset task view to today");
    viewMenu->AppendSeparator();

    MswUtils::OutlookInstanceCheck isOutlookInstalled;
    if (isOutlookInstalled()) {
        auto outlookViewMenuItem =
            viewMenu->Append(ID_VIEW_OUTLOOK, "&Outlook\tAlt-O", "View Outlook meetings");
        if (!MswUtils::IsOutlookRunning()) {
            outlookViewMenuItem->Enable(false);
        }
    }

    viewMenu->AppendSeparator();
    auto preferencesMenuItem = viewMenu->Append(
        ID_VIEW_PREFERENCES, "&Preferences\tCtrl-,", "View, set, and adjust program options");

    wxIconBundle preferencesIconBundle(Common::GetSettingsIconBundleName(), 0);
    preferencesMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(preferencesIconBundle));

    /* Help */
    auto helpMenu = new wxMenu();
    auto aboutMenuItem = helpMenu->Append(ID_HELP_ABOUT, "&About\tF1", "Information about Taskies");

    wxIconBundle aboutIconBundle(Common::GetAboutIconBundleName(), 0);
    aboutMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(aboutIconBundle));

    /* Menu bar */
    auto menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(editMenu, "E&dit");
    menuBar->Append(viewMenu, "&View");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);

    /* Main Controls */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto framePanel = new wxPanel(this);
    framePanel->SetSizer(sizer);

    /* InfoBar */
    pInfoBar = new wxInfoBar(framePanel, wxID_ANY);
    sizer->Add(pInfoBar, wxSizerFlags().Expand());

    auto topSizer = new wxBoxSizer(wxHORIZONTAL);

    auto fromDateLabel = new wxStaticText(framePanel, wxID_ANY, "<");
    pDatePickerCtrl = new wxDatePickerCtrl(framePanel, tksIDC_DATEPICKERCTRL);

    auto toDateLabel = new wxStaticText(framePanel, wxID_ANY, ">");

    topSizer->Add(fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    topSizer->Add(toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());

    sizer->Add(topSizer, wxSizerFlags().Expand());

    /* List Ctrl */
    pListCtrl = new wxListCtrl(framePanel,
        tksIDC_LISTCTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
    pListCtrl->SetFocus();
    sizer->Add(pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    int columnId = 0;

    wxListItem dateColumn;
    dateColumn.SetAlign(wxLIST_FORMAT_CENTER);
    dateColumn.SetText("Date");
    dateColumn.SetId(columnId);
    dateColumn.SetWidth(wxLIST_AUTOSIZE);
    pListCtrl->InsertColumn(columnId++, dateColumn);

    wxListItem projectColumn;
    projectColumn.SetId(columnId);
    projectColumn.SetText("Project");
    projectColumn.SetWidth(wxLIST_AUTOSIZE);
    pListCtrl->InsertColumn(columnId++, projectColumn);

    wxListItem categoryColumn;
    categoryColumn.SetId(columnId);
    categoryColumn.SetText("Category");
    categoryColumn.SetWidth(wxLIST_AUTOSIZE);
    pListCtrl->InsertColumn(columnId++, categoryColumn);

    wxListItem durationColumn;
    durationColumn.SetAlign(wxLIST_FORMAT_CENTER);
    durationColumn.SetId(columnId);
    durationColumn.SetText("Duration");
    durationColumn.SetWidth(wxLIST_AUTOSIZE);
    pListCtrl->InsertColumn(columnId++, durationColumn);

    wxListItem descriptionColumn;
    descriptionColumn.SetId(columnId);
    descriptionColumn.SetText("Description");
    descriptionColumn.SetWidth(wxLIST_AUTOSIZE);
    pListCtrl->InsertColumn(columnId++, descriptionColumn);

    /* Accelerator Table */
    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, (int) 'R', ID_VIEW_RESET);
    entries[1].Set(wxACCEL_CTRL, (int) 'N', ID_NEW_TASK);
    entries[2].Set(wxACCEL_CTRL, (int) ',', ID_VIEW_PREFERENCES);
    if (isOutlookInstalled() && !MswUtils::IsOutlookRunning()) {
        entries[3].Set(wxACCEL_ALT, (int) 'O', ID_VIEW_OUTLOOK);
    }

    wxAcceleratorTable table(ARRAYSIZE(entries), entries);
    SetAcceleratorTable(table);

    /* Status Bar */
    SetStatusBar(pStatusBar);
}

void MainFrame::FillControls() {}

void MainFrame::DataToControls()
{
    // Set InfoBar
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
        auto infoBarMessage = fmt::format("{0} {1} - v{2}.{3}.{4}",
            Common::GetProgramName(),
            BuildConfigurationToString(pEnv->GetBuildConfiguration()),
            TASKIES_MAJOR,
            TASKIES_MINOR,
            TASKIES_PATCH);
        pInfoBar->ShowMessage(infoBarMessage, wxICON_INFORMATION);
    }

    std::vector<Services::TaskViewModel> taskViewModels;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.FilterByDate(pDateStore->PrintTodayDate, taskViewModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateRangeTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        int columnIndex = 0;
        for (const auto& taskViewModel : taskViewModels) {
            int listIndex = pListCtrl->InsertItem(columnIndex++, taskViewModel.WorkdayDate);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.ProjectName);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.CategoryName);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.GetDuration());
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.Description);

            pListCtrl->SetItemBackgroundColour(listIndex, wxColor(taskViewModel.CategoryColor));
            if (Common::IsDarkColour(taskViewModel.CategoryColor)) {
                pListCtrl->SetItemTextColour(listIndex, *wxWHITE);
            }

            pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(taskViewModel.TaskId));
            columnIndex = 0;
        }

        int fixedWidth = 0;
        for (int i = 0; i < pListCtrl->GetColumnCount() - 1; i++) {
            // pListCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE);
            fixedWidth += pListCtrl->GetColumnWidth(i);
        }
        // fixedWidth += pListCtrl->GetColumnWidth(3);

        int totalWidth = pListCtrl->GetClientSize().GetWidth();

        // Ensure the expanding column fills the remaining space
        int expandWidth = totalWidth - fixedWidth - 4; // -4 for borders
        if (expandWidth < 80)
            expandWidth = 80; // Minimum width
        pListCtrl->SetColumnWidth(4, expandWidth);

        // Status Bar durations
        CalculateStatusBarTaskDurations();
    }
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    if (pCfg->CloseToTray() && pCfg->ShowInTray() && event.CanVeto()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Closing program to tray area");
        Hide();
        MSWGetTaskBarButton()->Hide();

        if (pMeetingsViewFrame) {
            pMeetingsViewFrame->Hide();
        }

        return;
    }
    // Call Hide() in case closing of program takes longer than expected and causes
    // a bad experience for the user
    Hide();

    if (pCfg->BackupDatabase() && pCfg->BackupOnProgramClose()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Backup database on program exit");

        Core::DatabaseBackup databaseBackup(pLogger);
        databaseBackup.SetSourceDatabaseFilePath(pCfg->BuildFullDatabaseFilePath());
        databaseBackup.SetDestinationDatabaseFilePath(pCfg->BuildFullBackupFilePath());

        auto result = databaseBackup.Backup();
        if (!result.Success) {
            pLogger->error("An error occured when performing backup on program close. Return "
                           "code ({0}) Message \"{1}\"",
                result.ReturnCode,
                result.ErrorMessage);
        }
    }

    if (pCfg->BackupDatabase() && pCfg->BackupOnProgramClose() && pCfg->ZipBackupFile()) {
        Core::ZipDatabaseBackup zipBackup(pLogger, pCfg->GetBackupPath());
        auto zipResult = zipBackup(pCfg->BuildFullBackupFilePath());
        if (!zipResult.Success) {
            wxRichMessageDialog dialog(this,
                Messages::ZipHeaderMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(zipResult.ErrorMessage);

            dialog.ShowModal();
        }
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Optimize database on program exit");

    Core::DatabaseOptimizer databaseOptimizer(pLogger, mDatabaseFilePath);
    auto result = databaseOptimizer.Optimize();
    if (!result.Success) {
        pLogger->error("An error occured when performing optimizations on program close. Return "
                       "code ({0}) Message \"{1}\"",
            result.ReturnCode,
            result.ErrorMessage);
    }

    event.Skip();
}

void MainFrame::OnIconize(wxIconizeEvent& event)
{
    if (event.IsIconized() && pCfg->ShowInTray() && pCfg->MinimizeToTray()) {
        MSWGetTaskBarButton()->Hide();

        if (pMeetingsViewFrame) {
            pMeetingsViewFrame->Hide();
        }
    }
}

void MainFrame::OnResize(wxSizeEvent& event)
{
    if (pMeetingsViewFrame) {
        pMeetingsViewFrame->OnParentFrameResize();
    }

    event.Skip();
}

void MainFrame::OnTaskReminder(wxTimerEvent& event)
{
    const std::string TAG = "MainFrame::OnTaskReminder";
    pLogger->info("{0} - Task reminder trigger notification", TAG);

    if (pCfg->UseNotificationBanners()) {
        pLogger->info("{0} - Display task reminder as notification", TAG);
        pTaskReminderNotification = std::make_shared<wxNotificationMessage>(
            "Reminder", "Reminder to capture tasks and their duration", this);
        pTaskReminderNotification->SetFlags(0);

        if (pCfg->OpenTaskDialogOnReminderClick()) {
            pLogger->info("{0} - Open task dialog on reminder is enabled", TAG);
            pTaskReminderNotification->Bind(
                wxEVT_NOTIFICATION_MESSAGE_CLICK, &MainFrame::OnReminderNotificationClicked, this);
        }

        pTaskReminderNotification->Show(wxNotificationMessage::Timeout_Auto);
    }
    if (pCfg->UseTaskbarFlashing()) {
        pLogger->info("{0} - Display task reminder as taskbar flashing", TAG);
        if (!IsActive() || IsIconized()) { // check if window is in background or minimized
            RequestUserAttention(wxUSER_ATTENTION_INFO);
        }
    }
    pLogger->info("{0} - Task reminder notification finished", TAG);
}

void MainFrame::OnMove(wxMoveEvent& event)
{
    if (pMeetingsViewFrame) {
        SPDLOG_LOGGER_TRACE(pLogger,
            "Main frame move event and Outlook frame is open!\nNew position => ({0},{1})",
            event.GetPosition().x,
            event.GetPosition().y);
        pMeetingsViewFrame->OnParentFrameMove();
    }

    event.Skip();
}

void MainFrame::OnThumbBarNewTask(wxCommandEvent& event)
{
    if (IsIconized()) {
        Restore();
    }
    Raise();
    Show();

    if (mThumbBarDialogOpenCounter == 0) {
        mThumbBarDialogOpenCounter++;

        dlg::TaskDialog newTaskDialog(this, pCfg, pLogger, mDatabaseFilePath);
        newTaskDialog.ShowModal();

        mThumbBarDialogOpenCounter--;
    }
}

void MainFrame::OnThumbBarQuickExport(wxCommandEvent& event)
{
    if (IsIconized()) {
        Restore();
    }
    Raise();
    Show();

    if (mThumbBarDialogOpenCounter == 0) {
        mThumbBarDialogOpenCounter++;

        dlg::QuickExportToFormatDialog quickExportToDialog(this, pCfg, pLogger, mDatabaseFilePath);
        quickExportToDialog.ShowModal();

        mThumbBarDialogOpenCounter--;
    }
}

void MainFrame::OnNewTask(wxCommandEvent& WXUNUSED(event))
{
    dlg::TaskDialog newTaskDialog(this, pCfg, pLogger, mDatabaseFilePath);
    newTaskDialog.ShowModal();
}

void MainFrame::OnNewEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pLogger, mDatabaseFilePath);
    newEmployerDialog.ShowModal();
}

void MainFrame::OnNewClient(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ClientDialog newClientDialog(this, pLogger, mDatabaseFilePath);
    newClientDialog.ShowModal();
}

void MainFrame::OnNewProject(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ProjectDialog newProjectDialog(this, pLogger, mDatabaseFilePath);
    newProjectDialog.ShowModal();
}

void MainFrame::OnNewCategory(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::CategoriesDialog addCategories(this, pLogger, mDatabaseFilePath);
    addCategories.ShowModal();
}

void MainFrame::OnNewAttributeGroup(wxCommandEvent& event)
{
    UI::dlg::AttributeGroupDialog newAttributeGroupDialog(this, pLogger, mDatabaseFilePath);
    newAttributeGroupDialog.ShowModal();
}

void MainFrame::OnNewAttribute(wxCommandEvent& event)
{
    dlg::AttributeDialog newAttributeDialog(this, pLogger, mDatabaseFilePath);
    newAttributeDialog.ShowModal();
}

void MainFrame::OnNewStaticAttributes(wxCommandEvent& event)
{
    dlg::StaticAttributeValuesDialog newStaticAttributesDialog(this, pLogger, mDatabaseFilePath);
    newStaticAttributesDialog.ShowModal();
}

void MainFrame::OnTasksBackupDatabase(wxCommandEvent& event)
{
    if (!pCfg->BackupDatabase()) {
        wxMessageBox(
            "Backups are not enabled", Common::GetProgramName(), wxOK_DEFAULT | wxICON_WARNING);
        return;
    }

    // put into a context to ensure the destructor of DatabaseBackup runs and
    // releases the locks on the backup file
    {
        Core::DatabaseBackup databaseBackup(pLogger);
        databaseBackup.SetSourceDatabaseFilePath(pCfg->BuildFullDatabaseFilePath());
        databaseBackup.SetDestinationDatabaseFilePath(pCfg->BuildFullBackupFilePath());

        auto result = databaseBackup.Backup();
        if (!result.Success) {
            wxRichMessageDialog dialog(this,
                Messages::BackupHeaderMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
        } else {
            wxMessageBox("Database backup completed successfully",
                Common::GetProgramName(),
                wxOK_DEFAULT | wxICON_INFORMATION,
                this);
        }
    }

    if (pCfg->ZipBackupFile()) {
        Core::ZipDatabaseBackup zipBackup(pLogger, pCfg->GetBackupPath());
        auto zipResult = zipBackup(pCfg->BuildFullBackupFilePath());
        if (!zipResult.Success) {
            wxRichMessageDialog dialog(this,
                Messages::ZipHeaderMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(zipResult.ErrorMessage);
            // dialog.ShowDetailedText(std::to_string(zipResult.ReturnCode));

            dialog.ShowModal();
        }
    }
}

void MainFrame::OnTasksExportToCsv(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ExportToCsvDialog exportToCsv(this, pCfg, pLogger, mDatabaseFilePath);
    exportToCsv.ShowModal();
}

void MainFrame::OnTasksExportToExcel(wxCommandEvent& WXUNUSED(event))
{
    dlg::ExportToExcelDialog exportToExcelDlg(this, pCfg, pLogger, mDatabaseFilePath);
    exportToExcelDlg.ShowModal();
}

void MainFrame::OnTasksQuickExportToFormat(wxCommandEvent& WXUNUSED(event))
{
    dlg::QuickExportToFormatDialog quickExportToDialog(this, pCfg, pLogger, mDatabaseFilePath);
    quickExportToDialog.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("MainFrame::OnExit - Menu/shortcut clicked to exit program");

    Close(true);
}

void MainFrame::OnEditEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editEmployer(
        this, pLogger, mDatabaseFilePath, EditListEntityType::Employers);
    editEmployer.ShowModal();
}

void MainFrame::OnEditClient(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editClient(
        this, pLogger, mDatabaseFilePath, EditListEntityType::Clients);
    editClient.ShowModal();
}

void MainFrame::OnEditProject(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editProject(
        this, pLogger, mDatabaseFilePath, EditListEntityType::Projects);
    editProject.ShowModal();
}

void MainFrame::OnEditCategory(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editCategory(
        this, pLogger, mDatabaseFilePath, EditListEntityType::Categories);
    editCategory.ShowModal();
}

void MainFrame::OnEditAttributeGroup(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editAttributeGroup(
        this, pLogger, mDatabaseFilePath, EditListEntityType::AttributeGroups);
    editAttributeGroup.ShowModal();
}

void MainFrame::OnEditAttribute(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editAttribute(
        this, pLogger, mDatabaseFilePath, EditListEntityType::Attributes);
    editAttribute.ShowModal();
}

void MainFrame::OnEditStaticAttributeValues(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editAttribute(
        this, pLogger, mDatabaseFilePath, EditListEntityType::StaticAttributeGroups);
    editAttribute.ShowModal();
}

void MainFrame::OnViewReset(wxCommandEvent& WXUNUSED(event))
{
    wxBusyCursor wait;

    DoResetToCurrentWeekAndOrToday();
}

void MainFrame::OnViewOutlook(wxCommandEvent& WXUNUSED(event))
{
    if (mOutlookMeetingViewFrameOpenCounter == 0) {
        mOutlookMeetingViewFrameOpenCounter++;
        pMeetingsViewFrame = new frames::OutlookMeetingsViewFrame(
            this, pCfg, pEnv, pLogger, mDatabaseFilePath, IsMaximized());
        pMeetingsViewFrame->Show();
    } else {
        SPDLOG_LOGGER_TRACE(pLogger, "Outlook meetings frame already open -> call Raise()");
        pMeetingsViewFrame->Raise();
    }
}

void MainFrame::OnViewPreferences(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::PreferencesDialog preferencesDlg(this, pEnv, pCfg, pLogger);
    int ret = preferencesDlg.ShowModal();

    if (ret == wxID_OK) {
        if (pCfg->ShowInTray() && !pTaskBarIcon->IsIconInstalled()) {
            pTaskBarIcon->SetTaskBarIcon();
        }
        if (!pCfg->ShowInTray() && pTaskBarIcon->IsIconInstalled()) {
            pTaskBarIcon->RemoveIcon();
        }
        if (pCfg->BackupDatabase()) {
            GetMenuBar()->Enable(ID_TASKS_BACKUPDATABASE, true);
        } else {
            GetMenuBar()->Enable(ID_TASKS_BACKUPDATABASE, false);
        }

        if (pCfg->UseReminders()) {
            if (!pTaskReminderTimer->IsRunning()) {
                pTaskReminderTimer->Start(
                    Utils::ConvertMinutesToMilliseconds(pCfg->ReminderInterval()));
            }
        } else {
            if (pTaskReminderTimer->IsRunning()) {
                pTaskReminderTimer->Stop();
            }
        }
    }
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    dlg::AboutDialog aboutDlg(this, pEnv);
    aboutDlg.ShowModal();
}

void MainFrame::OnPopupNewTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    dlg::TaskDialog popupNewTask(this, pCfg, pLogger, mDatabaseFilePath, false, -1, mTaskDate);
    popupNewTask.ShowModal();

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    SPDLOG_LOGGER_TRACE(pLogger, "Copy all tasks for date \"{0}\"", mTaskDate);

    std::vector<Services::TaskViewModel> taskModels;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.FilterByDate(mTaskDate, taskModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        ResetTaskContextMenuVariables();
        return;
    }

    std::stringstream formattedStringData;
    const auto& tasksViewColumns = pCfg->GetTasksViewColumns();

    for (const auto& taskModel : taskModels) {
        for (const auto& column : tasksViewColumns) {
            switch (column.ColumnModelIndex) {
            case TasksViewColumnModelIndex::ColumnModelIndexDate:
                formattedStringData << taskModel.WorkdayDate << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexEmployer:
                formattedStringData << taskModel.EmployerName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexClient:
                formattedStringData << taskModel.ClientName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexProject:
                formattedStringData << taskModel.ProjectName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexCategory:
                formattedStringData << taskModel.CategoryName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexDuration:
                formattedStringData << taskModel.GetDuration() << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexBillable:
                formattedStringData << taskModel.Billable << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexUniqueId:
                formattedStringData << taskModel.TryGetUniqueIdentifier() << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexDescription:
                formattedStringData << taskModel.Description << "\t";
                break;
            default:
                break;
            }
        }
        formattedStringData << "\n";
    }

    std::string clipboardData = formattedStringData.str();

    if (clipboardData.empty()) {
        ResetTaskContextMenuVariables();
        return;
    }

    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        wxTheClipboard->Clear();

        auto textData = new wxTextDataObject(clipboardData);
        wxTheClipboard->SetData(textData);
        wxTheClipboard->Close();

        SPDLOG_LOGGER_TRACE(pLogger,
            "Successfully copied \"{0}\" tasks for date \"{1}\"",
            taskModels.size(),
            mTaskDate);
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    SPDLOG_LOGGER_TRACE(pLogger, "Copy all tasks with headers for date \"{0}\"", mTaskDate);

    std::vector<Services::TaskViewModel> taskModels;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.FilterByDate(mTaskDate, taskModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        ResetTaskContextMenuVariables();
        return;
    }

    std::stringstream formattedStringData;
    const auto& tasksViewColumns = pCfg->GetTasksViewColumns();
    for (const auto& column : tasksViewColumns) {
        formattedStringData << column.Name << "\t";
    }
    formattedStringData << "\n";

    for (const auto& taskModel : taskModels) {
        for (const auto& column : tasksViewColumns) {
            switch (column.ColumnModelIndex) {
            case TasksViewColumnModelIndex::ColumnModelIndexDate:
                formattedStringData << taskModel.WorkdayDate << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexEmployer:
                formattedStringData << taskModel.EmployerName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexClient:
                formattedStringData << taskModel.ClientName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexProject:
                formattedStringData << taskModel.ProjectName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexCategory:
                formattedStringData << taskModel.CategoryName << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexDuration:
                formattedStringData << taskModel.GetDuration() << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexBillable:
                formattedStringData << taskModel.Billable << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexUniqueId:
                formattedStringData << taskModel.TryGetUniqueIdentifier() << "\t";
                break;
            case TasksViewColumnModelIndex::ColumnModelIndexDescription:
                formattedStringData << taskModel.Description << "\t";
                break;
            default:
                break;
            }
        }
        formattedStringData << "\n";
    }

    std::string clipboardData = formattedStringData.str();
    if (clipboardData.empty()) {
        ResetTaskContextMenuVariables();
        return;
    }

    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        wxTheClipboard->Clear();

        auto textData = new wxTextDataObject(clipboardData);
        wxTheClipboard->SetData(textData);
        wxTheClipboard->Close();

        SPDLOG_LOGGER_TRACE(pLogger,
            "Successfully copied \"{0}\" tasks with headers for date \"{1}\"",
            taskModels.size(),
            mTaskDate);
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksUsingPreset(wxCommandEvent& event)
{
    assert(!mTaskDate.empty());

    SPDLOG_LOGGER_TRACE(pLogger, "Copy all tasks using default preset for date \"{0}\"", mTaskDate);

    const auto& presets = pCfg->GetPresets();
    if (presets.size() == 0) {
        wxMessageBox("No presets defined to copy data with",
            Common::GetProgramName(),
            wxOK | wxOK_DEFAULT | wxICON_INFORMATION);
        return;
    }

    Services::Export::ExportOptions exportOptions;

    auto iter = std::find_if(presets.begin(),
        presets.end(),
        [](const Core::Configuration::PresetSetting& presetSetting) {
            return presetSetting.IsDefault == true;
        });

    Core::Configuration::PresetSetting presetSetting;
    if (iter != presets.end()) {
        presetSetting = *iter;
    } else {
        presetSetting = presets[0];
    }

    exportOptions.Delimiter = presetSetting.Delimiter;
    exportOptions.TextQualifier = presetSetting.TextQualifier;
    exportOptions.EmptyValuesHandler = presetSetting.EmptyValuesHandler;
    exportOptions.NewLinesHandler = presetSetting.NewLinesHandler;
    exportOptions.BooleanHandler = presetSetting.BooleanHandler;

    exportOptions.ExcludeHeaders = presetSetting.ExcludeHeaders;
    exportOptions.IncludeAttributes = presetSetting.IncludeAttributes;

    const auto& columnsToExport = presetSetting.Columns;
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("No columns to export in selected preset",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_WARNING);
        return;
    }

    auto columnExportModels = Services::Export::BuildFromPreset(columnsToExport);

    Services::Export::ProjectionBuilder projectionBuilder(pLogger);
    std::vector<Services::Export::Projection> projections =
        projectionBuilder.BuildProjections(columnExportModels);
    std::vector<Services::Export::ColumnJoinProjection> joinProjections =
        projectionBuilder.BuildJoinProjections(columnExportModels);

    Services::Export::CsvExporterService csvExporter(
        pLogger, exportOptions, mDatabaseFilePath, false);

    std::string exportedData = "";
    ExportResult result =
        csvExporter.ExportToCsv(projections, joinProjections, mTaskDate, mTaskDate, exportedData);

    if (!result.Success) {
        wxMessageBox(result.ErrorMessage, Common::GetProgramName(), wxICON_ERROR | wxOK_DEFAULT);

        return;
    }

    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        wxTheClipboard->Clear();

        auto textData = new wxTextDataObject(exportedData);
        wxTheClipboard->SetData(textData);

        wxTheClipboard->Close();
    } else {
        pLogger->error(
            "Failed to open the system clipboard to copy tasks for date \"{0}\"", mTaskDate);
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnCopyTaskDescriptionToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);

    std::string description;
    Persistence::TasksPersistence taskPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = taskPersistence.GetDescriptionById(mTaskIdToEdit, description);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::GetDescriptionByIdTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        auto canOpen = wxTheClipboard->Open();
        if (canOpen) {
            auto textData = new wxTextDataObject(description);
            wxTheClipboard->SetData(textData);
            wxTheClipboard->Close();
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnCopyRowTaskToClipboard(wxCommandEvent& event)
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);

    Services::TaskViewModel taskModel;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.GetById(mTaskIdToEdit, taskModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        return;
    }

    std::stringstream formattedStringData;
    const auto& tasksViewColumns = pCfg->GetTasksViewColumns();
    for (const auto& column : tasksViewColumns) {
        formattedStringData << column.Name << "\t";
    }
    formattedStringData << "\n";

    for (const auto& column : tasksViewColumns) {
        switch (column.ColumnModelIndex) {
        case TasksViewColumnModelIndex::ColumnModelIndexDate:
            formattedStringData << taskModel.WorkdayDate << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexEmployer:
            formattedStringData << taskModel.EmployerName << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexClient:
            formattedStringData << taskModel.ClientName << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexProject:
            formattedStringData << taskModel.ProjectName << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexCategory:
            formattedStringData << taskModel.CategoryName << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexDuration:
            formattedStringData << taskModel.GetDuration() << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexBillable:
            formattedStringData << taskModel.Billable << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexUniqueId:
            formattedStringData << taskModel.TryGetUniqueIdentifier() << "\t";
            break;
        case TasksViewColumnModelIndex::ColumnModelIndexDescription:
            formattedStringData << taskModel.Description << "\t";
            break;
        default:
            break;
        }
    }

    std::string copyData = formattedStringData.str();
    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        auto textData = new wxTextDataObject(copyData);
        wxTheClipboard->SetData(textData);
        wxTheClipboard->Close();
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnCopyRowTaskToClipboardWithPreset(wxCommandEvent& event)
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);

    const auto& presets = pCfg->GetPresets();
    if (presets.size() == 0) {
        wxMessageBox("No preset saved to use for copying data with",
            Common::GetProgramName(),
            wxOK | wxOK_DEFAULT | wxICON_INFORMATION);
        return;
    }

    Services::TaskViewModel taskModel;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.GetById(mTaskIdToEdit, taskModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        return;
    }

    auto iter = std::find_if(presets.begin(),
        presets.end(),
        [](const Core::Configuration::PresetSetting& presetSetting) {
            return presetSetting.IsDefault == true;
        });

    Core::Configuration::PresetSetting presetSetting;
    if (iter != presets.end()) {
        presetSetting = *iter;
    } else {
        presetSetting = presets[0];
    }

    Services::Export::ExportOptions exportOptions;

    exportOptions.Delimiter = presetSetting.Delimiter;
    exportOptions.TextQualifier = presetSetting.TextQualifier;
    exportOptions.EmptyValuesHandler = presetSetting.EmptyValuesHandler;
    exportOptions.NewLinesHandler = presetSetting.NewLinesHandler;
    exportOptions.BooleanHandler = presetSetting.BooleanHandler;

    exportOptions.ExcludeHeaders = presetSetting.ExcludeHeaders;
    exportOptions.IncludeAttributes = presetSetting.IncludeAttributes;

    const auto& columnsToExport = presetSetting.Columns;
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("No columns to export in selected preset",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_WARNING);
        return;
    }

    auto columnExportModels = Services::Export::BuildFromPreset(columnsToExport);

    Services::Export::ProjectionBuilder projectionBuilder(pLogger);
    std::vector<Services::Export::Projection> projections =
        projectionBuilder.BuildProjections(columnExportModels);
    std::vector<Services::Export::ColumnJoinProjection> joinProjections =
        projectionBuilder.BuildJoinProjections(columnExportModels);

    Services::Export::CsvExporterService csvExporter(
        pLogger, exportOptions, mDatabaseFilePath, mTaskIdToEdit);

    std::string exportedData = "";
    ExportResult result =
        csvExporter.ExportToCsv(projections, joinProjections, mTaskDate, mTaskDate, exportedData);

    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        wxTheClipboard->Clear();

        auto textData = new wxTextDataObject(exportedData);
        wxTheClipboard->SetData(textData);
        wxTheClipboard->Close();
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnEditTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);
    assert(mItemIndex >= 0);

    dlg::TaskDialog editTaskDialog(
        this, pCfg, pLogger, mDatabaseFilePath, true, mTaskIdToEdit, mTaskDate);
    editTaskDialog.ShowModal();

    mTaskIdToEdit = -1;
}

void MainFrame::OnDeleteTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);
    assert(mItemIndex > 0);

    int ret = wxMessageBox("Are you sure you want to delete this task?",
        Common::GetProgramName(),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
    if (ret == wxNO) {
        ResetTaskContextMenuVariables();
        return;
    }

    Persistence::TasksPersistence taskPersistence(pLogger, mDatabaseFilePath);
    Model::TaskModel taskModel;

    auto sqliteResult = taskPersistence.GetById(mTaskIdToEdit, taskModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::GetByIdTaskMessage,
            tks::Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();

        ResetTaskContextMenuVariables();
        return;
    }

    if (taskModel.AttendedMeetingId.has_value()) {
        Persistence::AttendedMeetingsPersistence attendedMeetingsPersistence(
            pLogger, mDatabaseFilePath);
        auto sqliteResult = attendedMeetingsPersistence.Delete(taskModel.AttendedMeetingId.value());

        if (!sqliteResult.Success) {
            wxRichMessageDialog dialog(this,
                Messages::DeleteAttendedMeetingMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
            dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

            dialog.ShowModal();

            ResetTaskContextMenuVariables();
            return;
        }
    }

    Persistence::TaskAttributeValuesPersistence taskAttributeValuesPersistence(
        pLogger, mDatabaseFilePath);
    sqliteResult = taskAttributeValuesPersistence.DeleteByTaskId(mTaskIdToEdit);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DeleteTaskAttributeValuesMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();

        ResetTaskContextMenuVariables();
        return;
    }

    sqliteResult = taskPersistence.Delete(mTaskIdToEdit);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DeleteTaskMessage,
            tks::Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();

        ResetTaskContextMenuVariables();
        return;
    } else {
        pListCtrl->DeleteItem(mItemIndex);

        TryUpdateSelectedDateAndAllTaskDurations(mTaskDate);
    }
}

void MainFrame::OnCloneTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);

    dlg::TaskDialog cloneTaskDialog(
        this, pCfg, pLogger, mDatabaseFilePath, true, mTaskIdToEdit, "", true);
    cloneTaskDialog.ShowModal();

    ResetTaskContextMenuVariables();
}

void MainFrame::OnAddMinutes(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToEdit != -1);
    assert(mItemIndex >= 0);

    Services::TaskDurationService taskDurationService(pLogger, mDatabaseFilePath);

    auto sqliteResult = taskDurationService.GetTaskTimeByIdAndIncrementByValue(
        mTaskIdToEdit, pCfg->GetMinutesIncrement());
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::DurationIncrementMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        ResetTaskContextMenuVariables();
        return;
    }

    TryUpdateSelectedDateAndAllTaskDurations(mTaskDate);
    RefreshListControlTaskItem(mTaskIdToEdit);

    ResetTaskContextMenuVariables();
}

void MainFrame::OnMenuHighlight(wxMenuEvent& event)
{
    wxMenuItem* item = nullptr;

    wxMenu* menu = event.GetMenu();
    if (menu) {
        item = menu->FindItem(event.GetId());
        if (item) {
            if (item && !item->GetHelp().empty()) {
                SetStatusText(item->GetHelp());
            } else {
                SetStatusText("");
            }
        }
    }
}

void MainFrame::OnTaskInserted(wxCommandEvent& event)
{
    // A task got inserted for a specific day
    auto eventTaskDateAdded = event.GetString().ToStdString();
    auto taskInsertedId = static_cast<std::int64_t>(event.GetExtraLong());
    SPDLOG_LOGGER_INFO(pLogger,
        "Received task inserted event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateAdded,
        taskInsertedId);

    std::istringstream ssTaskDateAdded{ eventTaskDateAdded };
    std::chrono::time_point<std::chrono::system_clock, date::days> dateTaskAdded;
    ssTaskDateAdded >> date::parse("%F", dateTaskAdded);

    TryUpdateSelectedDateAndAllTaskDurations(pDateStore->FormatDate(dateTaskAdded));

    if (dateTaskAdded != pDateStore->TodayDate) {
        return;
    }

    Services::TaskViewModel taskViewModel;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.GetById(taskInsertedId, taskViewModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::GetByIdTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        return;
    }

    int columnIndex = 0;
    int listIndex = pListCtrl->InsertItem(columnIndex++, taskViewModel.WorkdayDate);
    pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.ProjectName);
    pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.CategoryName);
    pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.GetDuration());
    pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.Description);

    pListCtrl->SetItemBackgroundColour(listIndex, wxColor(taskViewModel.CategoryColor));
    if (Common::IsDarkColour(taskViewModel.CategoryColor)) {
        pListCtrl->SetItemTextColour(listIndex, *wxWHITE);
    }

    pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(taskViewModel.TaskId));
}

void MainFrame::OnTaskDateChanged(wxCommandEvent& event)
{
    assert(mItemIndex >= 0);

    // A task got moved from one day to another day
    auto eventTaskDateChanged = event.GetString().ToStdString();
    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    SPDLOG_LOGGER_TRACE(pLogger,
        "Received task date changed event with new date"
        "\"{0}\" and ID \"{1}\"",
        eventTaskDateChanged,
        taskChangedId);

    pListCtrl->DeleteItem(mItemIndex);

    mItemIndex = -1;
}

void MainFrame::OnTaskUpdated(wxCommandEvent& event)
{
    assert(mItemIndex >= 0);

    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    SPDLOG_LOGGER_TRACE(pLogger, "Received task update event for ID: \"{0}\"", taskChangedId);

    RefreshListControlTaskItem(taskChangedId);

    mItemIndex = -1;
}

void MainFrame::OnTaskDeleted(wxCommandEvent& event)
{
    assert(mItemIndex >= 0);

    auto taskDeletedId = static_cast<std::int64_t>(event.GetExtraLong());

    SPDLOG_LOGGER_TRACE(pLogger, "Received task delete event with ID \"{0}\"", taskDeletedId);

    pListCtrl->DeleteItem(mItemIndex);

    mItemIndex = -1;
}

void MainFrame::OnReminderNotificationClicked(wxCommandEvent& WXUNUSED(event))
{
    dlg::TaskDialog newTaskDialog(this, pCfg, pLogger, mDatabaseFilePath);
    newTaskDialog.ShowModal();
}

void MainFrame::OnPowerResume(wxPowerEvent& WXUNUSED(event))
{
    SPDLOG_LOGGER_TRACE(pLogger, "System has resumed from being suspended");

    pDateStore->Reset();

    CalculateStatusBarTaskDurations();
}

void MainFrame::OnOutlookMeetingViewClose(wxCommandEvent& event)
{
    mOutlookMeetingViewFrameOpenCounter--;
    SPDLOG_LOGGER_TRACE(pLogger,
        "Outlook meetings frame closed, frame counter = {0}",
        mOutlookMeetingViewFrameOpenCounter);

    bool destroyStatus = pMeetingsViewFrame->Destroy();
    if (!destroyStatus) {
        pLogger->error("Unsuccessful destroy of meeting view frame");
    }
    pMeetingsViewFrame = nullptr;
}

void MainFrame::OnDateChanged(wxDateEvent& event)
{
    wxDateTime eventDate = wxDateTime(event.GetDate());
    wxDateTime eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);
    time_t eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newSelectedDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    std::string dateStringFormat = pDateStore->FormatDate(newSelectedDate);

    mTaskDate = dateStringFormat;

    RefreshListControlTaskItems();
}

void MainFrame::OnItemRightClick(wxListEvent& event)
{
    mItemIndex = event.GetIndex();
    mTaskIdToEdit = static_cast<std::int64_t>(event.GetData());

    wxMenu menu;
    auto copyMenuItem =
        menu.Append(wxID_COPY, "&Copy Description", "Copy description to the clipboard");
    wxIconBundle copyTaskIconBundle(Common::GetCopyPasteIconBundleName(), 0);
    copyMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(copyTaskIconBundle));

    auto copyRowMenuItem =
        menu.Append(ID_POP_COPY_ROW_TASK, "Copy &Row", "Copy row detail to the clipboard");
    wxIconBundle copyRowIconBundle(Common::GetCopyRowIconBundleName(), 0);
    copyRowMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(copyRowIconBundle));

    auto copyRowWithPresetMenuItem = menu.Append(ID_POP_COPY_ROW_TASK_PRESET,
        "Copy Row using &Preset",
        "Copy row detail using default preset to the clipboard");
    wxIconBundle copyRowWithPresetIconBundle(Common::GetCopyRowWithPresetIconBundleName(), 0);
    copyRowWithPresetMenuItem->SetBitmap(
        wxBitmapBundle::FromIconBundle(copyRowWithPresetIconBundle));

    menu.AppendSeparator();

    auto editTaskMenuItem = menu.Append(wxID_EDIT, "&Edit", "Edit the selected task");
    wxIconBundle editTaskIconBundle(Common::GetEditTaskIconBundleName(), 0);
    editTaskMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(editTaskIconBundle));

    auto deleteTaskMenuItem = menu.Append(wxID_DELETE, "&Delete", "Delete selected task");
    wxIconBundle deleteTaskIconBundle(Common::GetDeleteTaskIconBundleName(), 0);
    deleteTaskMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(deleteTaskIconBundle));

    menu.AppendSeparator();

    menu.Append(ID_POP_CLONE_TASK, "C&lone", "Clone the selected task");
    menu.AppendSeparator();

    std::string addMenuLabel = fmt::format("&Add {0} Minutes", pCfg->GetMinutesIncrement());
    menu.Append(wxID_ADD, addMenuLabel);

    menu.Bind(wxEVT_MENU_HIGHLIGHT, &MainFrame::OnMenuHighlight, this);

    PopupMenu(&menu);
}

void MainFrame::OnColumnEndDrag(wxListEvent& event)
{
    int totalWidth = pListCtrl->GetClientSize().GetWidth();
    int colCount = pListCtrl->GetColumnCount();

    // Subtract widths of all columns except last
    int usedWidth = 0;
    for (int i = 0; i < colCount - 1; i++) {
        usedWidth += pListCtrl->GetColumnWidth(i);
    }

    int lastColWidth = totalWidth - usedWidth - 4; // -4 for border/padding

    SPDLOG_LOGGER_TRACE(pLogger,
        "Resize columns. usedWith: {0} | lastColWidth: {1} | totalWidth: {2}",
        usedWidth,
        lastColWidth,
        totalWidth);

    if (lastColWidth < 80)
        lastColWidth = 80;

    pListCtrl->SetColumnWidth(colCount - 1, lastColWidth);
    event.Skip();
}

void MainFrame::OnColumnRightClick(wxListEvent& event)
{
    wxMenu menu;
    auto newTaskMenuItem = menu.Append(ID_POP_NEW_TASK, "&New Task", "Create new task");
    wxIconBundle addTaskIconBundle(Common::GetAddTaskIconBundleName(), 0);
    newTaskMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(addTaskIconBundle));

    menu.AppendSeparator();
    auto copyRowMenuItem = menu.Append(ID_POP_CONTAINER_COPY_TASKS,
        "&Copy",
        "Copy task values for selected date to the clipboard");
    wxIconBundle copyRowIconBundle(Common::GetCopyRowIconBundleName(), 0);
    copyRowMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(copyRowIconBundle));

    menu.Append(ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS,
        "Copy with &Headers",
        "Copy task values with headers for selected date to the clipboard");

    auto copyWithPresetMenuItem = menu.Append(ID_POP_CONTAINER_COPY_TASKS_PRESET,
        "Copy using &Preset",
        "Copy task values using default preset for selected date to the clipboard");
    wxIconBundle copyWithPresetIconBundle(Common::GetCopyWithPresetIconBundleName(), 0);
    copyWithPresetMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(copyWithPresetIconBundle));

    menu.Bind(wxEVT_MENU_HIGHLIGHT, &MainFrame::OnMenuHighlight, this);

    PopupMenu(&menu);
}

void MainFrame::DoResetToCurrentWeekAndOrToday()
{
    bool shouldReset = false;

    if (mFromDate != pDateStore->MondayDate) {
        shouldReset = true;
    }

    if (mToDate != pDateStore->SundayDate) {
        shouldReset = true;
    }

    if (pDateStore->MondayDate != pDateStore->CurrentWeekMondayDate) {
        shouldReset = true;
    }

    if (shouldReset) {
        pDateStore->Reset();
    }

    CalculateStatusBarTaskDurations();
}

void MainFrame::CalculateStatusBarTaskDurations()
{
    // Default hours
    CalculateDefaultTaskDurations();

    // Billable hours
    CalculateBillableTaskDurations();
}

void MainFrame::CalculateDefaultTaskDurations()
{
    pStatusBar->UpdateDefaultHoursDay(pDateStore->PrintTodayDate, pDateStore->PrintTodayDate);
    pStatusBar->UpdateDefaultHoursWeek(pDateStore->PrintMondayDate, pDateStore->PrintSundayDate);
    pStatusBar->UpdateDefaultHoursMonth(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth);
}

void MainFrame::CalculateBillableTaskDurations()
{
    pStatusBar->UpdateBillableHoursDay(pDateStore->PrintTodayDate, pDateStore->PrintTodayDate);
    pStatusBar->UpdateBillableHoursWeek(pDateStore->PrintMondayDate, pDateStore->PrintSundayDate);
    pStatusBar->UpdateBillableHoursMonth(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth);
}

void MainFrame::UpdateDefaultWeekMonthTaskDurations()
{
    pStatusBar->UpdateDefaultHoursWeek(pDateStore->PrintMondayDate, pDateStore->PrintSundayDate);
    pStatusBar->UpdateDefaultHoursMonth(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth);
}

void MainFrame::UpdateBillableWeekMonthTaskDurations()
{
    pStatusBar->UpdateBillableHoursWeek(pDateStore->PrintMondayDate, pDateStore->PrintSundayDate);
    pStatusBar->UpdateBillableHoursMonth(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth);
}

void MainFrame::UpdateDefaultRangeTaskDurations()
{
    const std::string fromDate = date::format("%F", mFromDate);
    const std::string toDate = date::format("%F", mToDate);

    pStatusBar->UpdateDefaultHoursRange(fromDate, toDate);
}

void MainFrame::UpdateBillableRangeTaskDurations()
{
    const std::string fromDate = date::format("%F", mFromDate);
    const std::string toDate = date::format("%F", mToDate);

    pStatusBar->UpdateBillableHoursRange(fromDate, toDate);
}

void MainFrame::TryUpdateSelectedDateAndAllTaskDurations(const std::string& date)
{
    pStatusBar->UpdateDefaultHoursDay(date, date);
    pStatusBar->UpdateBillableHoursDay(date, date);
    /*if (bDateRangeChanged) {
        UpdateDefaultRangeTaskDurations();
        UpdateBillableRangeTaskDurations();
    }*/
    UpdateDefaultWeekMonthTaskDurations();
    UpdateBillableWeekMonthTaskDurations();
}

void MainFrame::UpdateSelectedDayStatusBarTaskDurations(const std::string& date)
{
    pStatusBar->UpdateDefaultHoursDay(date, date);
    pStatusBar->UpdateBillableHoursDay(date, date);
}

void MainFrame::RefreshListControlTaskItem(const std::int64_t taskId)
{
    Services::TaskViewModel taskViewModel;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.GetById(taskId, taskViewModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::GetByIdTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        return;
    }

    int columnIndex = 0;
    pListCtrl->SetItem(mItemIndex, columnIndex++, taskViewModel.WorkdayDate);
    pListCtrl->SetItem(mItemIndex, columnIndex++, taskViewModel.ProjectName);
    pListCtrl->SetItem(mItemIndex, columnIndex++, taskViewModel.CategoryName);
    pListCtrl->SetItem(mItemIndex, columnIndex++, taskViewModel.GetDuration());
    pListCtrl->SetItem(mItemIndex, columnIndex++, taskViewModel.Description);

    pListCtrl->SetItemBackgroundColour(mItemIndex, wxColor(taskViewModel.CategoryColor));
    if (Common::IsDarkColour(taskViewModel.CategoryColor)) {
        pListCtrl->SetItemTextColour(mItemIndex, *wxWHITE);
    }

    pListCtrl->RefreshItem(mItemIndex);
}

void MainFrame::RefreshListControlTaskItems()
{
    assert(!mTaskDate.empty());

    pListCtrl->DeleteAllItems();

    std::vector<Services::TaskViewModel> taskViewModels;
    Services::TasksService tasksService(pLogger, mDatabaseFilePath);

    auto sqliteResult = tasksService.FilterByDate(mTaskDate, taskViewModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterByDateRangeTaskMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    } else {
        int columnIndex = 0;
        for (const auto& taskViewModel : taskViewModels) {
            int listIndex = pListCtrl->InsertItem(columnIndex++, taskViewModel.WorkdayDate);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.ProjectName);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.CategoryName);
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.GetDuration());
            pListCtrl->SetItem(listIndex, columnIndex++, taskViewModel.Description);

            pListCtrl->SetItemBackgroundColour(listIndex, wxColor(taskViewModel.CategoryColor));
            if (Common::IsDarkColour(taskViewModel.CategoryColor)) {
                pListCtrl->SetItemTextColour(listIndex, *wxWHITE);
            }

            pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(taskViewModel.TaskId));
            columnIndex = 0;
        }
    }

    // Status Bar durations
    CalculateStatusBarTaskDurations();
}

void MainFrame::ResetTaskContextMenuVariables()
{
    mTaskIdToEdit = -1;
    mItemIndex = -1;
}
} // namespace tks::UI
