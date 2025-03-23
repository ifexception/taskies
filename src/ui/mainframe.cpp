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
#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/taskbarbutton.h>

#include "../common/common.h"
#include "../common/constants.h"
#include "../common/enums.h"
#include "../common/version.h"

#include "../core/environment.h"
#include "../core/configuration.h"

#include "../persistence/taskpersistence.h"

#include "../repository/taskrepository.h"
#include "../repository/taskrepositorymodel.h"

#include "../utils/utils.h"

#include "../ui/dlg/errordlg.h"
#include "../ui/dlg/employerdlg.h"
#include "../ui/dlg/editlistdlg.h"
#include "../ui/dlg/clientdlg.h"
#include "../ui/dlg/projectdlg.h"
#include "../ui/dlg/categoriesdlg.h"
#include "../ui/dlg/aboutdlg.h"
#include "../ui/dlg/preferences/preferencesdlg.h"
#include "../ui/dlg/taskdlglegacy.h"
#include "../ui/dlg/daytaskviewdlg.h"
#include "../ui/dlg/exports/exporttocsvdlg.h"
#include "../ui/dlg/exports/quickexporttocsvdlg.h"
#include "../ui/dlg/taskdlg.h"
#include "../ui/dlg/attributes/attributegroupdlg.h"
#include "../ui/dlg/attributes/attributedlg.h"

#include "events.h"
#include "notificationclientdata.h"

// This date was selected arbitrarily
// wxDatePickerCtrl needs a from and to date for the range
// So we pick 2015-01-01 as that date
// Conceivably, a user shouldn't go that far back
wxDateTime MakeMaximumFromDate()
{
    wxDateTime maxFromDate = wxDateTime::Now();
    maxFromDate.SetYear(2015);
    maxFromDate.SetMonth(wxDateTime::Jan);
    maxFromDate.SetDay(1);

    return maxFromDate;
}

namespace tks::UI
{
// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
/* General Event Handlers */
EVT_CLOSE(MainFrame::OnClose)
EVT_ICONIZE(MainFrame::OnIconize)
EVT_SIZE(MainFrame::OnResize)
EVT_TIMER(tksIDC_TASKREMINDERTIMER, MainFrame::OnTaskReminder)
/* Menu Event Handlers */
EVT_MENU(ID_NEW_TASK, MainFrame::OnNewTask)
EVT_MENU(ID_NEW_EMPLOYER, MainFrame::OnNewEmployer)
EVT_MENU(ID_NEW_CLIENT, MainFrame::OnNewClient)
EVT_MENU(ID_NEW_PROJECT, MainFrame::OnNewProject)
EVT_MENU(ID_NEW_CATEGORY, MainFrame::OnNewCategory)
EVT_MENU(ID_NEW_ATTRIBUTEGROUP, MainFrame::OnNewAttributeGroup)
EVT_MENU(ID_NEW_ATTRIBUTE, MainFrame::OnNewAttribute)
EVT_MENU(ID_TASKS_BACKUPDATABASE, MainFrame::OnTasksBackupDatabase)
EVT_MENU(ID_TASKS_EXPORTTOCSV, MainFrame::OnTasksExportToCsv)
EVT_MENU(ID_TASKS_QUICKEXPORTTOCSV, MainFrame::OnTasksQuickExportToCsv)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_EDIT_EMPLOYER, MainFrame::OnEditEmployer)
EVT_MENU(ID_EDIT_CLIENT, MainFrame::OnEditClient)
EVT_MENU(ID_EDIT_PROJECT, MainFrame::OnEditProject)
EVT_MENU(ID_EDIT_CATEGORY, MainFrame::OnEditCategory)
EVT_MENU(ID_EDIT_ATTRIBUTE_GROUP, MainFrame::OnEditAttributeGroup)
EVT_MENU(ID_EDIT_ATTRIBUTE, MainFrame::OnEditAttribute)
EVT_MENU(ID_VIEW_RESET, MainFrame::OnViewReset)
EVT_MENU(ID_VIEW_EXPAND, MainFrame::OnViewExpand)
EVT_MENU(ID_VIEW_DAY, MainFrame::OnViewDay)
EVT_MENU(ID_VIEW_PREFERENCES, MainFrame::OnViewPreferences)
EVT_MENU(ID_HELP_ABOUT, MainFrame::OnAbout)
/* Popup Menu Event Handlers */
EVT_MENU(ID_POP_NEW_TASK, MainFrame::OnPopupNewTask)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS, MainFrame::OnContainerCopyTasksToClipboard)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS, MainFrame::OnContainerCopyTasksWithHeadersToClipboard)
EVT_MENU(wxID_COPY, MainFrame::OnCopyTaskToClipboard)
EVT_MENU(wxID_EDIT, MainFrame::OnEditTask)
EVT_MENU(wxID_DELETE, MainFrame::OnDeleteTask)
/* Error Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ERROR, MainFrame::OnError)
/* Custom Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ADDNOTIFICATION, MainFrame::OnAddNotification)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATEADDED, MainFrame::OnTaskAddedOnDate)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATEDELETED, MainFrame::OnTaskDeletedOnDate)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATEDCHANGEDFROM, MainFrame::OnTaskDateChangedFrom)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATEDCHANGEDTO, MainFrame::OnTaskDateChangedTo)
/* Control Event Handlers */
EVT_BUTTON(tksIDC_NOTIFICATIONBUTTON, MainFrame::OnNotificationClick)
EVT_DATE_CHANGED(tksIDC_FROMDATE, MainFrame::OnFromDateSelection)
EVT_DATE_CHANGED(tksIDC_TODATE, MainFrame::OnToDateSelection)
/* DataViewCtrl Event Handlers */
EVT_DATAVIEW_ITEM_CONTEXT_MENU(tksIDC_TASKDATAVIEWCTRL, MainFrame::OnContextMenu)
EVT_DATAVIEW_SELECTION_CHANGED(tksIDC_TASKDATAVIEWCTRL, MainFrame::OnDataViewSelectionChanged)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxFrame(
        nullptr,
        wxID_ANY,
        Common::GetProgramName(),
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_FRAME_STYLE,
        name
    )
    , pLogger(logger)
    , pEnv(env)
    , pCfg(cfg)
    , mDatabaseFilePath()
    , pInfoBar(nullptr)
    , pTaskBarIcon(nullptr)
    , pStatusBar(nullptr)
    , pNotificationPopupWindow(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , pNotificationButton(nullptr)
    , mBellBitmap(wxNullBitmap)
    , mBellNotificationBitmap(wxNullBitmap)
    , pDateStore(nullptr)
    , mFromDate()
    , mToDate()
    , mToLatestPossibleDate()
    , pDataViewCtrl(nullptr)
    , pTaskTreeModel(nullptr)
    , mFromCtrlDate()
    , mToCtrlDate()
    , mTaskIdToModify(-1)
    , mTaskDate()
    , mExpandCounter(0)
    , bDateRangeChanged(false)
    , pTaskReminderTimer(std::make_unique<wxTimer>(this, tksIDC_TASKREMINDERTIMER))
    , pTaskReminderNotification()
// clang-format on
{
    SPDLOG_LOGGER_TRACE(pLogger, "Initialization of MainFrame");
    // Initialization setup
    SetMinSize(wxSize(FromDIP(320), FromDIP(320)));
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        pLogger->info(
            "MainFrame::MainFrame - No persistent objects found. Set default size \"{0}\"x\"{1}\"",
            800,
            600);
        SetSize(FromDIP(wxSize(800, 600)));
    }

    // Initialize image handlers and images
    wxPNGHandler* pngHandler = new wxPNGHandler();
    wxImage::AddHandler(pngHandler);

    auto bellImagePath = pEnv->GetResourcesPath() / Common::Resources::Bell();
    auto bellNotificationImagePath =
        pEnv->GetResourcesPath() / Common::Resources::BellNotification();

    mBellBitmap.LoadFile(bellImagePath.string(), wxBITMAP_TYPE_PNG);
    mBellNotificationBitmap.LoadFile(bellNotificationImagePath.string(), wxBITMAP_TYPE_PNG);

    // Set main icon in titlebar
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);

    // Load database path
    mDatabaseFilePath = pCfg->GetDatabasePath();
    pLogger->info("MainFrame::MainFrame - Database location \"{0}\"", mDatabaseFilePath);

    // Setup TaskBarIcon
    pTaskBarIcon = new TaskBarIcon(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
    if (pCfg->ShowInTray()) {
        pLogger->info(
            "MainFrame::MainFrame - TaskBarIcon \"ShowInTray\" is \"{0}\"", pCfg->ShowInTray());
        pTaskBarIcon->SetTaskBarIcon();
    }

    // Setup StatusBar
    pStatusBar = new StatusBar(this, pLogger, mDatabaseFilePath);

    // Setup DateStore
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    // Setup reminders (if enabled)
    if (pCfg->UseReminders()) {
        pTaskReminderTimer->Start(Utils::ConvertMinutesToMilliseconds(pCfg->ReminderInterval()));

        wxNotificationMessage::UseTaskBarIcon(pTaskBarIcon);
    }

    // Create controls
    Create();

    // Create the notification popup window
    pNotificationPopupWindow = new NotificationPopupWindow(this, pLogger);
}

MainFrame::~MainFrame()
{
    const std::string TAG = "MainFrame::~MainFrame";
    if (pTaskBarIcon) {
        pLogger->info("{0} - Removing task bar icon", TAG);
        pTaskBarIcon->RemoveIcon();
        pLogger->info("{0} - Delete task bar icon pointer", TAG);
        delete pTaskBarIcon;
    }

    if (pStatusBar) {
        pLogger->info("{0} - Delete status bar pointer", TAG);
        delete pStatusBar;
    }

    if (pCfg->UseReminders() && pTaskReminderTimer->IsRunning()) {
        pLogger->info("{0} - Reminders enabled and timer is running", TAG);
        pTaskReminderTimer->Stop();
        pLogger->info("{0} - Timer stopped", TAG);
    }

    if (pNotificationPopupWindow) {
        pLogger->info("{0} - Delete notification popup window pointer", TAG);
        delete pNotificationPopupWindow;
    }

    pLogger->info("{0} - Destructor complete", TAG);
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

    auto newTaskMenuBarTitle =
        pCfg->UseLegacyTaskDialog() ? "&New Task (legacy)\tCtrl-N" : "&New Task\tCtrl-N";
    auto newTaskMenuBarDescription =
        pCfg->UseLegacyTaskDialog() ? "Create new task (legacy)" : "Create new task";

    auto newTaskMenuItem =
        fileMenu->Append(ID_NEW_TASK, newTaskMenuBarTitle, newTaskMenuBarDescription);

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
        ID_TASKS_EXPORTTOCSV, "E&xport to CSV", "Export selected data to CSV format");
    fileTasksMenu->Append(ID_TASKS_QUICKEXPORTTOCSV,
        "Q&uick Export to CSV",
        "Export selected data to CSV format using existing presets");
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

    /* View */
    auto viewMenu = new wxMenu();
    viewMenu->Append(ID_VIEW_RESET, "&Reset View\tCtrl-R", "Reset task view to current week");
    viewMenu->Append(ID_VIEW_EXPAND, "&Expand\tCtrl-E", "Expand date procedure");
    viewMenu->Append(ID_VIEW_DAY, "Day View", "See task view for the selected day");
    viewMenu->AppendSeparator();
    auto preferencesMenuItem =
        viewMenu->Append(ID_VIEW_PREFERENCES, "&Preferences", "View and adjust program options");

    wxIconBundle preferencesIconBundle(Common::GetPreferencesIconBundleName(), 0);
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

    auto fromDateLabel = new wxStaticText(framePanel, wxID_ANY, "From: ");
    pFromDatePickerCtrl = new wxDatePickerCtrl(framePanel, tksIDC_FROMDATE);

    auto toDateLabel = new wxStaticText(framePanel, wxID_ANY, "To: ");
    pToDatePickerCtrl = new wxDatePickerCtrl(framePanel, tksIDC_TODATE);

    topSizer->Add(fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pFromDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    topSizer->Add(toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pToDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    topSizer->AddStretchSpacer();

    pNotificationButton = new wxBitmapButton(framePanel, tksIDC_NOTIFICATIONBUTTON, mBellBitmap);
    pNotificationButton->SetToolTip("View notifications");
    topSizer->Add(pNotificationButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    sizer->Add(topSizer, wxSizerFlags().Expand());

    /* Data View Ctrl */
    /* Data View Columns Renderers */
    auto projectNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto categoryNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto durationTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto descriptionTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    descriptionTextRenderer->EnableEllipsize(wxEllipsizeMode::wxELLIPSIZE_END);

    auto idRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);

    /* Week Data View Ctrl */
    pDataViewCtrl = new wxDataViewCtrl(framePanel,
        tksIDC_TASKDATAVIEWCTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

    /* Week Data View Model */
    pTaskTreeModel = new TaskTreeModel(pDateStore->MondayToSundayDateRangeList, pLogger);
    pDataViewCtrl->AssociateModel(pTaskTreeModel.get());

    /* Project Column */
    auto projectColumn = new wxDataViewColumn("Project",
        projectNameTextRenderer,
        TaskTreeModel::Col_Project,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    projectColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(projectColumn);

    /* Category Column */
    auto categoryColumn = new wxDataViewColumn("Category",
        categoryNameTextRenderer,
        TaskTreeModel::Col_Category,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    categoryColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(categoryColumn);

    /* Duration Column */
    auto durationColumn = new wxDataViewColumn(
        "Duration", durationTextRenderer, TaskTreeModel::Col_Duration, 80, wxALIGN_CENTER);
    durationColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    durationColumn->SetResizeable(false);
    pDataViewCtrl->AppendColumn(durationColumn);

    /* Description Column */
    auto descriptionColumn = new wxDataViewColumn("Description",
        descriptionTextRenderer,
        TaskTreeModel::Col_Description,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    pDataViewCtrl->AppendColumn(descriptionColumn);

    /* ID Column */
    auto idColumn = new wxDataViewColumn(
        "ID", idRenderer, TaskTreeModel::Col_Id, 32, wxALIGN_CENTER, wxDATAVIEW_COL_HIDDEN);
    pDataViewCtrl->AppendColumn(idColumn);

    sizer->Add(pDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    pDataViewCtrl->SetFocus();

    /* Accelerator Table */
    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, (int) 'R', ID_VIEW_RESET);
    entries[1].Set(wxACCEL_CTRL, (int) 'N', ID_NEW_TASK);
    entries[2].Set(wxACCEL_CTRL, (int) 'E', ID_VIEW_EXPAND);

    wxAcceleratorTable table(ARRAYSIZE(entries), entries);
    SetAcceleratorTable(table);

    /* Status Bar */
    SetStatusBar(pStatusBar);
}

void MainFrame::FillControls()
{
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();
}

void MainFrame::DataToControls()
{
    // Set InfoBar
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
        auto infoBarMessage = fmt::format("{0} {1} - v{2}.{3}.{4}-{5}",
            Common::GetProgramName(),
            BuildConfigurationToString(pEnv->GetBuildConfiguration()),
            TASKIES_MAJOR,
            TASKIES_MINOR,
            TASKIES_PATCH,
            TASKIES_SRLC);
        pInfoBar->ShowMessage(infoBarMessage, wxICON_INFORMATION);
    }

    // Fetch tasks between mFromDate and mToDate
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc =
        taskRepo.FilterByDateRange(pDateStore->MondayToSundayDateRangeList, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertChildNodes(workdayDate, tasks);
        }
    }

    pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));

    // Status Bar durations
    CalculateStatusBarTaskDurations();
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    pLogger->info("MainFrame::OnClose - Closing program");
    if (pCfg->CloseToTray() && pCfg->ShowInTray() && event.CanVeto()) {
        pLogger->info("MainFrame::OnClose - Closing program to tray area");
        Hide();
        MSWGetTaskBarButton()->Hide();
    } else {
        // Call Hide() in case closing of program takes longer than expected and causes
        // a bad experience for the user
        Hide();

        pLogger->info("MainFrame::OnClose - Optimize database on program exit");
        pLogger->info(
            "MainFrame::OnClose - Open database connection at \"{0}\"", mDatabaseFilePath);

        sqlite3* db = nullptr;
        int rc = sqlite3_open(mDatabaseFilePath.c_str(), &db);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(db);
            pLogger->error(
                LogMessage::OpenDatabaseTemplate, "MainFrame::OnClose", mDatabaseFilePath, rc, err);
            goto cleanup;
        }

        rc = sqlite3_exec(db, Utils::sqlite::pragmas::Optimize, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(db);
            pLogger->error(LogMessage::ExecQueryTemplate,
                "MainFrame::OnClose",
                Utils::sqlite::pragmas::Optimize,
                rc,
                err);
            goto cleanup;
        }

        pLogger->info(
            "MainFrame::OnClose - Optimizimation command successfully executed on database");

    cleanup:
        sqlite3_close(db);
        event.Skip();
    }
}

void MainFrame::OnIconize(wxIconizeEvent& event)
{
    pLogger->info("MainFrame::OnIconize - Iconize program");
    if (event.IsIconized() && pCfg->ShowInTray() && pCfg->MinimizeToTray()) {
        pLogger->info("MainFrame::OnIconize - Iconize program to tray area");
        MSWGetTaskBarButton()->Hide();
    }
}

void MainFrame::OnResize(wxSizeEvent& event)
{
    if (pNotificationPopupWindow) {
        pNotificationPopupWindow->OnResize();
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

void MainFrame::OnNotificationClick(wxCommandEvent& event)
{
    pNotificationButton->SetBitmap(mBellBitmap);

    wxWindow* btn = (wxWindow*) event.GetEventObject();

    auto y = GetClientSize().GetWidth();
    auto xPositionOffset = (GetClientSize().GetWidth() + 4) * 0.25;
    if (GetClientSize().GetWidth() < 800) {
        xPositionOffset = 200; // cap notification window width at 200
    }

    wxPoint pos = btn->ClientToScreen(wxPoint(xPositionOffset * -1, 0));
    wxSize size = btn->GetSize();
    pNotificationPopupWindow->Position(pos, size);
    pNotificationPopupWindow->Popup();
}

void MainFrame::OnNewTask(wxCommandEvent& WXUNUSED(event))
{
    if (pCfg->UseLegacyTaskDialog()) {
        UI::dlg::TaskDialogLegacy newTaskDialogLegacy(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
        newTaskDialogLegacy.ShowModal();
    } else {
        UI::dlg::TaskDialog newTaskDialog(this, pCfg, pLogger, mDatabaseFilePath);
        newTaskDialog.ShowModal();
    }
}

void MainFrame::OnNewEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pLogger, mDatabaseFilePath);
    newEmployerDialog.ShowModal();
}

void MainFrame::OnNewClient(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ClientDialog newClientDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newClientDialog.ShowModal();
}

void MainFrame::OnNewProject(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ProjectDialog newProjectDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newProjectDialog.ShowModal();
}

void MainFrame::OnNewCategory(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::CategoriesDialog addCategories(this, pEnv, pLogger, mDatabaseFilePath);
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

void MainFrame::OnTasksBackupDatabase(wxCommandEvent& event)
{
    if (!pCfg->BackupDatabase()) {
        wxMessageBox(
            "Backups are toggled off!\nToggle backups in \"File\">\"Tasks\">\"Backup Database\"",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_WARNING);
        return;
    }

    int rc = 0;
    sqlite3* db = nullptr;
    sqlite3* backupDb = nullptr;
    sqlite3_backup* backup = nullptr;

    rc = sqlite3_open(mDatabaseFilePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "MainFrame::OnTasksBackupDatabase",
            mDatabaseFilePath,
            rc,
            err);
        return;
    }

    auto backupFilePath = fmt::format("{0}/{1}", pCfg->GetBackupPath(), pEnv->GetDatabaseName());
    rc = sqlite3_open(backupFilePath.c_str(), &backupDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "MainFrame::OnTasksBackupDatabase",
            backupFilePath,
            rc,
            err);
        return;
    }

    backup = sqlite3_backup_init(/*destination*/ backupDb, "main", /*source*/ db, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }

    rc = sqlite3_errcode(backupDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(backupDb);
        pLogger->error("{0} - Failed to backup database to {1}. Error {2}: \"{3}\"",
            "MainFrame::OnTasksBackupDatabase",
            backupFilePath,
            rc,
            err);
        return;
    }

    sqlite3_close(db);
    sqlite3_close(backupDb);

    std::string message = "Backup successful";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Information, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(this, addNotificationEvent);
}

void MainFrame::OnTasksExportToCsv(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ExportToCsvDialog exportToCsv(this, pCfg, pLogger, mDatabaseFilePath);
    exportToCsv.ShowModal();
}

void MainFrame::OnTasksQuickExportToCsv(wxCommandEvent& event)
{
    UI::dlg::QuickExportToCsvDialog quickExportToCsv(this, pCfg, pLogger, mDatabaseFilePath);
    quickExportToCsv.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("MainFrame::OnExit - Menu/shortcut clicked to exit program");

    Close(true);
}

void MainFrame::OnEditEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editEmployer(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Employer);
    editEmployer.ShowModal();
}

void MainFrame::OnEditClient(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editClient(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Client);
    editClient.ShowModal();
}

void MainFrame::OnEditProject(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editProject(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Project);
    editProject.ShowModal();
}

void MainFrame::OnEditCategory(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editCategory(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Category);
    editCategory.ShowModal();
}

void MainFrame::OnEditAttributeGroup(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editAttributeGroup(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::AttributeGroup);
    editAttributeGroup.ShowModal();
}

void MainFrame::OnEditAttribute(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editAttribute(
        this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Attribute);
    editAttribute.ShowModal();
}

void MainFrame::OnViewReset(wxCommandEvent& WXUNUSED(event))
{
    wxBusyCursor wait;

    DoResetToCurrentWeekAndOrToday();
}

void MainFrame::OnViewExpand(wxCommandEvent& WXUNUSED(event))
{
    for (auto& item : pTaskTreeModel->TryCollapseDateNodes()) {
        pDataViewCtrl->Collapse(item);
    }

    if (mExpandCounter == 0) {
        std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);
        for (auto& item : pTaskTreeModel->TryExpandAllDateNodes(dates)) {
            pDataViewCtrl->Expand(item);
        }
    }
    if (mExpandCounter == 1) {
        std::vector<std::string> dates;
        auto todaysDate = pDateStore->TodayDate;
        dates.push_back(date::format("%F", todaysDate));

        auto yesterdaysDate = todaysDate - date::days{ 1 };
        dates.push_back(date::format("%F", yesterdaysDate));

        for (auto dataViewItem : pTaskTreeModel->TryExpandAllDateNodes(dates)) {
            pDataViewCtrl->Expand(dataViewItem);
        }
    }
    if (mExpandCounter == 2) {
        pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));
    }

    mExpandCounter++;
    if (mExpandCounter >= MAX_EXPAND_COUNT) {
        mExpandCounter = 0;
    }

    TryUpdateSelectedDateAndAllTaskDurations(pDateStore->PrintTodayDate);
}

void MainFrame::OnViewDay(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::DayTaskViewDialog dayTaskView(this,
        pLogger,
        pEnv,
        mDatabaseFilePath,
        mTaskDate.empty() ? pDateStore->PrintTodayDate : mTaskDate);
    dayTaskView.ShowModal();
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

        SetNewTaskMenubarTitle();

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
    UI::dlg::AboutDialog aboutDlg(this);
    aboutDlg.ShowModal();
}

void MainFrame::OnPopupNewTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    if (pCfg->UseLegacyTaskDialog()) {
        UI::dlg::TaskDialogLegacy popupNewTaskLegacy(
            this, pEnv, pCfg, pLogger, mDatabaseFilePath, false, -1, mTaskDate);
        popupNewTaskLegacy.ShowModal();
    } else {
        UI::dlg::TaskDialog popupNewTask(
            this, pCfg, pLogger, mDatabaseFilePath, false, -1, mTaskDate);
        popupNewTask.ShowModal();
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    pLogger->info("MainFrame::OnContainerCopyToClipboard - Copy all tasks for date {0}", mTaskDate);

    std::vector<repos::TaskRepositoryModel> taskModels;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDate(mTaskDate, taskModels);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        std::stringstream formattedClipboardData;
        for (const auto& taskModel : taskModels) {
            if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
                formattedClipboardData << taskModel.TaskId << "\t";
            }

            formattedClipboardData << taskModel.ProjectName << "\t";
            formattedClipboardData << taskModel.CategoryName << "\t";
            formattedClipboardData << taskModel.GetDuration() << "\t";
            formattedClipboardData << taskModel.Description << "\t";
            formattedClipboardData << "\n";
        }

        std::string clipboardData = formattedClipboardData.str();
        auto canOpen = wxTheClipboard->Open();
        if (canOpen) {
            auto textData = new wxTextDataObject(clipboardData);
            wxTheClipboard->SetData(textData);
            wxTheClipboard->Close();

            pLogger->info("MainFrame::OnContainerCopyToClipboard - Successfully copied \"{0}\" "
                          "tasks for date \"{1}\"",
                taskModels.size(),
                mTaskDate);
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());

    pLogger->info("MainFrame::OnContainerCopyToClipboard - Copy all tasks for date {0}", mTaskDate);

    std::vector<repos::TaskRepositoryModel> taskModels;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDate(mTaskDate, taskModels);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        std::stringstream formattedClipboardData;
        if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
            formattedClipboardData << "Task Id"
                                   << "\t";
        }
        formattedClipboardData << "Project"
                               << "\t";
        formattedClipboardData << "Category"
                               << "\t";
        formattedClipboardData << "Duration"
                               << "\t";
        formattedClipboardData << "Description"
                               << "\t";
        formattedClipboardData << "\n";

        for (const auto& taskModel : taskModels) {
            if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
                formattedClipboardData << taskModel.TaskId << "\t";
            }

            formattedClipboardData << taskModel.ProjectName << "\t";
            formattedClipboardData << taskModel.CategoryName << "\t";
            formattedClipboardData << taskModel.GetDuration() << "\t";
            formattedClipboardData << taskModel.Description << "\t";
            formattedClipboardData << "\n";
        }

        std::string clipboardData = formattedClipboardData.str();
        auto canOpen = wxTheClipboard->Open();
        if (canOpen) {
            auto textData = new wxTextDataObject(clipboardData);
            wxTheClipboard->SetData(textData);
            wxTheClipboard->Close();

            pLogger->info("MainFrame::OnContainerCopyToClipboard - Successfully copied \"{0}\" "
                          "tasks for date \"{1}\"",
                taskModels.size(),
                mTaskDate);
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnCopyTaskToClipboard(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToModify != -1);

    std::string description;
    Persistence::TaskPersistence taskPersistence(pLogger, mDatabaseFilePath);

    int rc = taskPersistence.GetDescriptionById(mTaskIdToModify, description);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
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

void MainFrame::OnEditTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToModify != -1);

    int ret = -1;

    if (pCfg->UseLegacyTaskDialog()) {
        UI::dlg::TaskDialogLegacy editTaskDialogLegacy(
            this, pEnv, pCfg, pLogger, mDatabaseFilePath, true, mTaskIdToModify, mTaskDate);
        ret = editTaskDialogLegacy.ShowModal();
    } else {
        UI::dlg::TaskDialog editTaskDialog(
            this, pCfg, pLogger, mDatabaseFilePath, true, mTaskIdToModify, mTaskDate);
        ret = editTaskDialog.ShowModal();
    }

    if (ret == wxID_OK) {
        bool isActive = false;
        Persistence::TaskPersistence taskPersistence(pLogger, mDatabaseFilePath);
        int rc = taskPersistence.IsDeleted(mTaskIdToModify, isActive);
        if (rc != 0) {
            QueueFetchTasksErrorNotificationEvent();
            return;
        }

        if (isActive) {
            repos::TaskRepositoryModel taskModel;
            repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

            int rc = taskRepo.GetById(mTaskIdToModify, taskModel);
            if (rc != 0) {
                QueueFetchTasksErrorNotificationEvent();
            } else {
                pTaskTreeModel->ChangeChild(mTaskDate, taskModel);

                TryUpdateSelectedDateAndAllTaskDurations(mTaskDate);
            }
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnDeleteTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToModify != -1);

    Persistence::TaskPersistence taskPersistence(pLogger, mDatabaseFilePath);

    int rc = taskPersistence.Delete(mTaskIdToModify);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->DeleteChild(mTaskDate, mTaskIdToModify);

        TryUpdateSelectedDateAndAllTaskDurations(mTaskDate);

        auto message = "Successfully deleted task";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(this, addNotificationEvent);
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnError(wxCommandEvent& event)
{
    UI::dlg::ErrorDialog errDialog(this, pEnv, pLogger, event.GetString().ToStdString());
    errDialog.ShowModal();
}

void MainFrame::OnAddNotification(wxCommandEvent& event)
{
    pLogger->info("MainFrame::OnAddNotification - Received notification event");

    pNotificationButton->SetBitmap(mBellNotificationBitmap);

    NotificationClientData* notificationClientData =
        reinterpret_cast<NotificationClientData*>(event.GetClientObject());

    pNotificationPopupWindow->AddNotification(
        notificationClientData->Message, notificationClientData->Type);

    if (notificationClientData) {
        delete notificationClientData;
    }
}

void MainFrame::OnTaskAddedOnDate(wxCommandEvent& event)
{
    // A task got inserted for a specific day
    auto eventTaskDateAdded = event.GetString().ToStdString();
    auto taskInsertedId = static_cast<std::int64_t>(event.GetExtraLong());
    pLogger->info(
        "MainFrame::OnTaskAddedOnDate - Received task added event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateAdded,
        taskInsertedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was inserted for is in the selected range of our wxDateTimeCtrl's
    auto iterator = std::find_if(dates.begin(), dates.end(), [&](const std::string& date) {
        return date == eventTaskDateAdded;
    });

    // If we are in range, refetch the data for our particular date
    if (iterator != dates.end() && taskInsertedId != 0 && eventTaskDateAdded.size() != 0) {
        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskInsertedId);

        TryUpdateSelectedDateAndAllTaskDurations(foundDate);
    }
}

void MainFrame::OnTaskDeletedOnDate(wxCommandEvent& event)
{
    // A task got deleted on a specific day
    auto eventTaskDateDeleted = event.GetString().ToStdString();
    auto taskDeletedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info("MainFrame::OnTaskDeletedOnDate - Received task added event with date \"{0}\" "
                  "and ID \"{1}\"",
        eventTaskDateDeleted,
        taskDeletedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was deleted is in the selected range of our wxDateTimeCtrl's
    auto iterator = std::find_if(dates.begin(), dates.end(), [&](const std::string& date) {
        return date == eventTaskDateDeleted;
    });

    // If we are in range, remove the task data for our particular date
    if (iterator != dates.end() && taskDeletedId != 0 && eventTaskDateDeleted.size() != 0) {
        pLogger->info("MainFrame::OnTaskDeletedOnDate - Task deleted on a date within bounds!");

        auto& foundDate = *iterator;
        pTaskTreeModel->DeleteChild(foundDate, taskDeletedId);

        TryUpdateSelectedDateAndAllTaskDurations(foundDate);
    }
}

void MainFrame::OnTaskDateChangedFrom(wxCommandEvent& event)
{
    // A task got moved from one day to another day
    auto eventTaskDateChanged = event.GetString().ToStdString();
    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info("MainFrame::OnTaskDateChangedFrom - Received task date changed event with date "
                  "\"{0}\" and ID \"{1}\"",
        eventTaskDateChanged,
        taskChangedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator = std::find_if(dates.begin(), dates.end(), [&](const std::string& date) {
        return date == eventTaskDateChanged;
    });

    // If we are in range, remove the item data for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedFrom - Task changed from a date within bounds");

        auto& foundDate = *iterator;
        pTaskTreeModel->DeleteChild(foundDate, taskChangedId);

        TryUpdateSelectedDateAndAllTaskDurations(foundDate);
    }
}

void MainFrame::OnTaskDateChangedTo(wxCommandEvent& event)
{
    // A task got moved from one day to another day
    auto eventTaskDateChanged = event.GetString().ToStdString();
    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info("MainFrame::OnTaskDateChangedTo - Received task date changed event with date "
                  "\"{0}\" and ID \"{1}\"",
        eventTaskDateChanged,
        taskChangedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator = std::find_if(dates.begin(), dates.end(), [&](const std::string& date) {
        return date == eventTaskDateChanged;
    });

    // If we are in range, add the task for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedTo - Task date changed to date within bounds!");

        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskChangedId);

        TryUpdateSelectedDateAndAllTaskDurations(foundDate);
    }
}

void MainFrame::OnFromDateSelection(wxDateEvent& event)
{
    pLogger->info("MainFrame::OnFromDateSelection - Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());
    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > mToCtrlDate) {
        SetFromDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot exceed to date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pFromDatePickerCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();

    // Check if the selected date goes beyond six months from the current date
    auto currentDate =
        date::year_month_day{ date::floor<date::days>(std::chrono::system_clock::now()) };
    auto sixMonthsPastDate = currentDate - date::months{ 6 };
    auto newFromDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));

    if (newFromDate < sixMonthsPastDate) {
        int ret =
            wxMessageBox("Are you sure you want to load tasks that are older than six (6) months?",
                "Confirmation",
                wxYES_NO,
                this);
        if (ret == wxNO) {
            SetFromDateAndDatePicker();
            return;
        }
    }

    mFromCtrlDate = eventDateUtc;
    mFromDate = newFromDate;

    if (mFromDate == mToDate) {
        auto fromDateString = date::format("%F", mFromDate);

        std::vector<repos::TaskRepositoryModel> tasks;
        repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

        int rc = taskRepo.FilterByDate(fromDateString, tasks);
        if (rc != 0) {
            QueueFetchTasksErrorNotificationEvent();
        } else {
            pTaskTreeModel->ClearAll();
            pTaskTreeModel->InsertRootAndChildNodes(fromDateString, tasks);
        }
        return;
    }

    pLogger->info("MainFrame::OnFromDateSelection - Calculate list of dates from date: \"{0}\" to "
                  "date: \"{1}\"",
        date::format("%F", mFromDate),
        date::format("%F", mToDate));

    // Calculate list of dates between from and to date
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Fetch all the tasks for said date range
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDateRange(dates, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->ClearAll();
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertRootAndChildNodes(workdayDate, tasks);
        }
    }

    // Update status bar hours
    // Check if the week dates have changed
    if (mFromDate != pDateStore->MondayDate) {
        // If so, we cannot display [Week] and [Month] as there is no guarantee where we have gone
        // Thus, switch to a [Range] format as a catch all for whatever the date selection is
        pStatusBar->UpdateDefaultHoursRange(
            date::format("%F", mFromDate), date::format("%F", mToDate));
        pStatusBar->UpdateBillableHoursRange(
            date::format("%F", mFromDate), date::format("%F", mToDate));

        bDateRangeChanged = true;
    } else {
        // Otherwise we are back in our week range and reset to the default
        UpdateDefaultWeekMonthTaskDurations();
        UpdateBillableWeekMonthTaskDurations();
    }
}

void MainFrame::OnToDateSelection(wxDateEvent& event)
{
    pLogger->info(
        "MainFrame::OnToDateSelection - Received date (wxDateTime) event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());

    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);
    auto eventDateUtcTicks = eventDateUtc.GetTicks();

    if (eventDateUtc > mToLatestPossibleDate) {
        SetToDateAndDatePicker();
        return;
    }

    if (eventDateUtc < mFromCtrlDate) {
        SetFromDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot go past from date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pToDatePickerCtrl);
        return;
    }

    mToCtrlDate = eventDateUtc;
    auto newToDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    mToDate = newToDate;

    pLogger->info("MainFrame::OnToDateSelection - Calculate list of dates from date: \"{0}\" to "
                  "date: \"{1}\"",
        date::format("%F", mFromDate),
        date::format("%F", mToDate));

    if (mFromDate == mToDate) {
        auto date = date::format("%F", mToDate);

        std::vector<repos::TaskRepositoryModel> tasks;
        repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

        int rc = taskRepo.FilterByDate(date, tasks);
        if (rc != 0) {
            QueueFetchTasksErrorNotificationEvent();
        } else {
            pTaskTreeModel->ClearAll();
            pTaskTreeModel->InsertRootAndChildNodes(date, tasks);
        }
        return;
    }

    // Calculate list of dates between from and to date
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Fetch all the tasks for said date range
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDateRange(dates, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->ClearAll();
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertRootAndChildNodes(workdayDate, tasks);
        }
    }

    // Update status bar hours
    // Check if the week dates have changed
    if (mToDate != pDateStore->SundayDate) {
        // If so, we cannot display [Week] and [Month] as there is no guarantee where we have gone
        // Thus, switch to a [Range] format as a catch all for whatever the date selection is
        pStatusBar->UpdateDefaultHoursRange(
            date::format("%F", mFromDate), date::format("%F", mToDate));
        pStatusBar->UpdateBillableHoursRange(
            date::format("%F", mFromDate), date::format("%F", mToDate));

        bDateRangeChanged = true;
    } else {
        // Otherwise we are back in our week range and reset to the default
        UpdateDefaultWeekMonthTaskDurations();
        UpdateBillableWeekMonthTaskDurations();
    }
}

void MainFrame::OnContextMenu(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();

    if (item.IsOk()) {
        pLogger->info("MainFrame::OnContextMenu - Clicked on valid wxDateViewItem");
        auto model = (TaskTreeModelNode*) item.GetID();

        if (model->IsContainer()) {
            pLogger->info("MainFrame::OnContextMenu - Clicked on container node with date \"{0}\"",
                model->GetProjectName());
            mTaskDate = model->GetProjectName();

            std::istringstream ssTaskDate{ mTaskDate };
            std::chrono::time_point<std::chrono::system_clock, date::days> dateTaskDate;
            ssTaskDate >> date::parse("%F", dateTaskDate);

            wxMenu menu;
            auto newTaskMenuItem = menu.Append(ID_POP_NEW_TASK, "New Task");
            if (dateTaskDate > pDateStore->TodayDate) {
                newTaskMenuItem->Enable(false);
            }
            menu.AppendSeparator();
            menu.Append(ID_POP_CONTAINER_COPY_TASKS, wxT("&Copy"));
            menu.Append(ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS, wxT("Copy with Headers"));
            PopupMenu(&menu);
        } else {
            pLogger->info("MainFrame::OnContextMenu - Clicked on leaf node with task ID \"{0}\"",
                model->GetTaskId());
            mTaskIdToModify = model->GetTaskId();

            // This is confusing, but by calling `GetParent()` we are getting the container node
            // pointer here `GetProjectName()` then returns the date of the container node value
            mTaskDate = model->GetParent()->GetProjectName();

            wxMenu menu;
            menu.Append(wxID_COPY, wxT("&Copy"));
            menu.Append(wxID_EDIT, wxT("&Edit"));
            menu.Append(wxID_DELETE, wxT("&Delete"));

            PopupMenu(&menu);
        }
    }
}

void MainFrame::OnDataViewSelectionChanged(wxDataViewEvent& event)
{
    auto item = event.GetItem();
    if (!item.IsOk()) {
        return;
    }

    auto isContainer = pTaskTreeModel->IsContainer(item);
    pLogger->info("MainFrame::OnSelectionChanged - IsContainer = {0}", isContainer);

    if (isContainer) {
        pLogger->info("MainFrame::OnSelectionChanged - Collapse all nodes");
        for (auto& item : pTaskTreeModel->TryCollapseDateNodes()) {
            pDataViewCtrl->Collapse(item);
        }

        pLogger->info("MainFrame::OnSelectionChanged - Expand selected item node");
        pDataViewCtrl->Expand(item);

        auto model = (TaskTreeModelNode*) item.GetID();
        auto selectedDate = model->GetProjectName();

        TryUpdateSelectedDateAndAllTaskDurations(selectedDate);

        if (pCfg->TodayAlwaysExpanded()) {
            pLogger->info("MainFrame::OnSelectionChanged - Expand today's item node");
            pDataViewCtrl->Expand(
                pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));
        }
    }
}

void MainFrame::OnReminderNotificationClicked(wxCommandEvent& WXUNUSED(event))
{
    if (pCfg->UseLegacyTaskDialog()) {
        UI::dlg::TaskDialogLegacy newTaskDialogLegacy(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
        newTaskDialogLegacy.ShowModal();
    } else {
        UI::dlg::TaskDialog newTaskDialog(this, pCfg, pLogger, mDatabaseFilePath);
        newTaskDialog.ShowModal();
    }
}

void MainFrame::SetNewTaskMenubarTitle()
{
    auto newTaskMenubarTitle =
        pCfg->UseLegacyTaskDialog() ? "&New Task (legacy)\tCtrl-N" : "&New Task\tCtrl-N";
    auto newTaskMenubarHelp =
        pCfg->UseLegacyTaskDialog() ? "Create new task (legacy)" : "Create new task";

    auto fileMenu = GetMenuBar()->GetMenu(0);
    auto newTaskMenu = fileMenu->FindItemByPosition(0);
    newTaskMenu->SetItemLabel(newTaskMenubarTitle);
    newTaskMenu->SetHelp(newTaskMenubarHelp);
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

        ResetDateRange();
        ResetDatePickerValues();
        RefetchTasksForDateRange();
    }

    for (auto& item : pTaskTreeModel->TryCollapseDateNodes()) {
        pDataViewCtrl->Collapse(item);
    }

    bDateRangeChanged = false;

    CalculateStatusBarTaskDurations();
    pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));
}

void MainFrame::ResetDateRange()
{
    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;
}

void MainFrame::ResetDatePickerValues()
{
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();

    SetToDateAndDatePicker();
}

void MainFrame::RefetchTasksForDateRange()
{
    pLogger->info("MainFrame::RefetchTasksForDateRange - Dates: \"{0}\" - \"{1}\"",
        date::format("%F", mFromDate),
        date::format("%F", mToDate));

    // Fetch tasks between mFromDate and mToDate
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc =
        taskRepo.FilterByDateRange(pDateStore->MondayToSundayDateRangeList, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->ClearAll();
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertRootAndChildNodes(workdayDate, tasks);
        }
    }
}

void MainFrame::RefetchTasksForDate(const std::string& date, const std::int64_t taskId)
{
    repos::TaskRepositoryModel taskModel;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.GetById(taskId, taskModel);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->InsertChildNode(date, taskModel);
    }
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
    const auto& fromDate = date::format("%F", mFromDate);
    const auto& toDate = date::format("%F", mToDate);

    pStatusBar->UpdateDefaultHoursRange(fromDate, toDate);
}

void MainFrame::UpdateBillableRangeTaskDurations()
{
    const auto& fromDate = date::format("%F", mFromDate);
    const auto& toDate = date::format("%F", mToDate);

    pStatusBar->UpdateBillableHoursRange(fromDate, toDate);
}

void MainFrame::TryUpdateSelectedDateAndAllTaskDurations(const std::string& date)
{
    pStatusBar->UpdateDefaultHoursDay(date, date);
    pStatusBar->UpdateBillableHoursDay(date, date);
    if (bDateRangeChanged) {
        UpdateDefaultRangeTaskDurations();
        UpdateBillableRangeTaskDurations();
    } else {
        UpdateDefaultWeekMonthTaskDurations();
        UpdateBillableWeekMonthTaskDurations();
    }
}

void MainFrame::UpdateSelectedDayStatusBarTaskDurations(const std::string& date)
{
    pStatusBar->UpdateDefaultHoursDay(date, date);
    pStatusBar->UpdateBillableHoursDay(date, date);
}

void MainFrame::QueueFetchTasksErrorNotificationEvent()
{
    std::string message = "Failed to fetch tasks";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(this, addNotificationEvent);
}

void MainFrame::SetFromAndToDatePickerRanges()
{
    pFromDatePickerCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateTime fromFromDate = wxDateTime::Now(), toFromDate = wxDateTime::Now();

    if (pFromDatePickerCtrl->GetRange(&fromFromDate, &toFromDate)) {
        pLogger->info(
            "MainFrame::SetFromAndToDatePickerRanges - pFromDatePickerCtrl range is [{0} - {1}]",
            fromFromDate.FormatISODate().ToStdString(),
            toFromDate.FormatISODate().ToStdString());
    }

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDatePickerCtrl->SetRange(
        wxDateTime(pDateStore->MondayDateSeconds), latestPossibleDatePlusOneDay);

    wxDateTime toFromDate2 = wxDateTime::Now(), toToDate = wxDateTime::Now();

    if (pToDatePickerCtrl->GetRange(&toFromDate2, &toToDate)) {
        pLogger->info(
            "MainFrame::SetFromAndToDatePickerRanges - pToDatePickerCtrl range is [{0} - {1})",
            toFromDate2.FormatISODate().ToStdString(),
            toToDate.FormatISODate().ToStdString());
    }

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void MainFrame::SetFromDateAndDatePicker()
{
    pFromDatePickerCtrl->SetValue(pDateStore->MondayDateSeconds);

    pLogger->info("MainFrame::SetFromDateAndDatePicker - Reset pFromDatePickerCtrl to: {0}",
        pFromDatePickerCtrl->GetValue().FormatISODate().ToStdString());

    mFromCtrlDate = pDateStore->MondayDateSeconds;

    pLogger->info("MainFrame::SetFromDateAndDatePicker - Reset mFromCtrlDate to: {0}",
        mFromCtrlDate.FormatISODate().ToStdString());
}

void MainFrame::SetToDateAndDatePicker()
{
    pToDatePickerCtrl->SetValue(pDateStore->SundayDateSeconds);

    pLogger->info(
        "MainFrame::SetToDateAndDatePicker - \npToDateCtrl date = {0}\nSundayDateSeconds = {1}",
        pToDatePickerCtrl->GetValue().FormatISOCombined().ToStdString(),
        date::format("%Y-%m-%d %I:%M:%S %p",
            date::sys_seconds{ std::chrono::seconds(pDateStore->SundayDateSeconds) }));

    pLogger->info("MainFrame::SetToDateAndDatePicker - Reset pToDatePickerCtrl to: {0}",
        pToDatePickerCtrl->GetValue().FormatISODate().ToStdString());

    mToCtrlDate = pDateStore->SundayDateSeconds;

    pLogger->info("MainFrame::SetToDateAndDatePicker - Reset mToCtrlDate to: {0}",
        mToCtrlDate.FormatISODate().ToStdString());
}

void MainFrame::ResetTaskContextMenuVariables()
{
    mTaskIdToModify = -1;
    mTaskDate = "";
}
} // namespace tks::UI
