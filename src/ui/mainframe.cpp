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

#include "mainframe.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <vector>

#include <date/date.h>

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/taskbarbutton.h>

#include <sqlite3.h>

#include "../common/common.h"
#include "../common/constants.h"
#include "../common/enums.h"
#include "../common/version.h"

#include "../core/environment.h"
#include "../core/configuration.h"

#include "../dao/taskdao.h"

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
#include "../ui/dlg/preferencesdlg.h"
#include "../ui/dlg/taskdialog.h"
#include "../ui/dlg/daytaskviewdlg.h"
#include "../ui/dlg/exporttocsvdlg.h"

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
/* Menu Event Handlers */
EVT_MENU(ID_NEW_TASK, MainFrame::OnNewTask)
EVT_MENU(ID_NEW_EMPLOYER, MainFrame::OnNewEmployer)
EVT_MENU(ID_NEW_CLIENT, MainFrame::OnNewClient)
EVT_MENU(ID_NEW_PROJECT, MainFrame::OnNewProject)
EVT_MENU(ID_NEW_CATEGORY, MainFrame::OnNewCategory)
EVT_MENU(ID_TASKS_BACKUPDATABASE, MainFrame::OnTasksBackupDatabase)
EVT_MENU(ID_TASKS_EXPORTTOCSV, MainFrame::OnTasksExportToCsv)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_EDIT_EMPLOYER, MainFrame::OnEditEmployer)
EVT_MENU(ID_EDIT_CLIENT, MainFrame::OnEditClient)
EVT_MENU(ID_EDIT_PROJECT, MainFrame::OnEditProject)
EVT_MENU(ID_EDIT_CATEGORY, MainFrame::OnEditCategory)
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
/* Keyboard shortcuts */
EVT_MENU(ID_KYB_LEFT, MainFrame::OnKeyLeft)
EVT_MENU(ID_KYB_RIGHT, MainFrame::OnKeyRight)
/* Error Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ERROR, MainFrame::OnError)
/* Custom Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ADDNOTIFICATION, MainFrame::OnAddNotification)
EVT_COMMAND(wxID_ANY, tksEVT_TASKDATEADDED, MainFrame::OnTaskDateAdded)
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
/* Key Event Handlers */
//EVT_CHAR_HOOK(MainFrame::OnKeyDown)
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
    , pFromDateCtrl(nullptr)
    , pToDateCtrl(nullptr)
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
// clang-format on
{
    // Initialization setup
    SetMinSize(wxSize(FromDIP(320), FromDIP(320)));
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        pLogger->info("MainFrame::MainFrame - No persistent objects found. Set default size \"{0}\"x\"{1}\"", 800, 600);
        SetSize(FromDIP(wxSize(800, 600)));
    }

    // Initialize image handlers and images
    wxPNGHandler* pngHandler = new wxPNGHandler();
    wxImage::AddHandler(pngHandler);

    auto bellImagePath = pEnv->GetResourcesPath() / Common::Resources::Bell();
    auto bellNotificationImagePath = pEnv->GetResourcesPath() / Common::Resources::BellNotification();

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
        pLogger->info("MainFrame::MainFrame - TaskBarIcon \"ShowInTray\" is \"{0}\"", pCfg->ShowInTray());
        pTaskBarIcon->SetTaskBarIcon();
    }

    // Setup StatusBar
    pStatusBar = new StatusBar(this);

    // Setup DateStore
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    // Create controls
    Create();

    // Create the notification popup window
    pNotificationPopupWindow = new NotificationPopupWindow(this, pLogger);
}

MainFrame::~MainFrame()
{
    if (pTaskBarIcon) {
        pLogger->info("MainFrame - Removing task bar icon");
        pTaskBarIcon->RemoveIcon();
        pLogger->info("MainFrame - Delete task bar icon pointer");
        delete pTaskBarIcon;
    }

    if (pNotificationPopupWindow) {
        pLogger->info("MainFrame - Delete notification popup window pointer");
        delete pNotificationPopupWindow;
    }

    if (pStatusBar) {
        pLogger->info("MainFrame - Delete status bar pointer");
        delete pStatusBar;
    }

    pLogger->info("MainFrame - Destructor");
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
    fileMenu->AppendSubMenu(fileNewMenu, "New");
    fileMenu->AppendSeparator();

    auto fileTasksMenu = new wxMenu();
    auto fileTasksMenuItem =
        fileTasksMenu->Append(ID_TASKS_BACKUPDATABASE, "&Backup Database", "Backup a copy of the database");
    if (!pCfg->BackupDatabase()) {
        fileTasksMenuItem->Enable(false);
    }
    fileTasksMenu->Append(ID_TASKS_EXPORTTOCSV, "&Export to CSV", "Export selected data to CSV format");
    fileMenu->AppendSubMenu(fileTasksMenu, "Tasks");
    fileMenu->AppendSeparator();

    auto exitMenuItem = fileMenu->Append(wxID_EXIT, "Exit\tAlt-F4", "Exit the program");

    wxIconBundle exitIconBundle(Common::GetExitIconBundleName(), 0);
    exitMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(exitIconBundle));

    /* Edit */
    auto editMenu = new wxMenu();
    editMenu->Append(ID_EDIT_EMPLOYER, "Edit Employer", "Edit an employer");
    editMenu->Append(ID_EDIT_CLIENT, "Edit Client", "Edit a client");
    editMenu->Append(ID_EDIT_PROJECT, "Edit Project", "Edit a project");
    editMenu->Append(ID_EDIT_CATEGORY, "Edit Category", "Edit a category");

    /* View */
    auto viewMenu = new wxMenu();
    viewMenu->Append(ID_VIEW_RESET, "&Reset View\tCtrl-R", "Reset task view to current week");
    viewMenu->Append(ID_VIEW_EXPAND, "&Expand\tCtrl-E", "Expand date procedure");
    viewMenu->Append(ID_VIEW_DAY, "Day View", "See task view for the selected day");
    viewMenu->AppendSeparator();
    auto preferencesMenuItem = viewMenu->Append(ID_VIEW_PREFERENCES, "&Preferences", "View and adjust program options");

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
    menuBar->Append(editMenu, "&Edit");
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

    // Bind Ctrl+G plus menu bar option to a new dialog that pops up
    // Instead of hacking the mainframe continually
    auto topSizer = new wxBoxSizer(wxHORIZONTAL);

    auto fromDateLabel = new wxStaticText(framePanel, wxID_ANY, "From: ");
    pFromDateCtrl = new wxDatePickerCtrl(framePanel, tksIDC_FROMDATE);

    auto toDateLabel = new wxStaticText(framePanel, wxID_ANY, "To: ");
    pToDateCtrl = new wxDatePickerCtrl(framePanel, tksIDC_TODATE);

    topSizer->Add(fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pFromDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    topSizer->Add(toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pToDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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
    auto projectColumn = new wxDataViewColumn(
        "Project", projectNameTextRenderer, TaskTreeModel::Col_Project, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    projectColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(projectColumn);

    /* Category Column */
    auto categoryColumn = new wxDataViewColumn(
        "Category", categoryNameTextRenderer, TaskTreeModel::Col_Category, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    categoryColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(categoryColumn);

    /* Duration Column */
    auto durationColumn =
        new wxDataViewColumn("Duration", durationTextRenderer, TaskTreeModel::Col_Duration, 80, wxALIGN_CENTER);
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
    auto idColumn =
        new wxDataViewColumn("ID", idRenderer, TaskTreeModel::Col_Id, 32, wxALIGN_CENTER, wxDATAVIEW_COL_HIDDEN);
    pDataViewCtrl->AppendColumn(idColumn);

    sizer->Add(pDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    pDataViewCtrl->SetFocus();

    /* Accelerator Table */
    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, (int) 'R', ID_VIEW_RESET);
    entries[1].Set(wxACCEL_CTRL, (int) 'N', ID_NEW_TASK);
    entries[2].Set(wxACCEL_CTRL, (int) 'E', ID_VIEW_EXPAND);
    entries[3].Set(wxACCEL_CTRL, WXK_LEFT, ID_KYB_LEFT);
    entries[4].Set(wxACCEL_CTRL, WXK_RIGHT, ID_KYB_RIGHT);

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
        auto infoBarMessage = fmt::format("{0} {1} - v{2}.{3}.{4}",
            Common::GetProgramName(),
            BuildConfigurationToString(pEnv->GetBuildConfiguration()),
            TASKIES_MAJOR,
            TASKIES_MINOR,
            TASKIES_PATCH);
        pInfoBar->ShowMessage(infoBarMessage, wxICON_INFORMATION);
    }

    // Fetch tasks between mFromDate and mToDate
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDateRange(pDateStore->MondayToSundayDateRangeList, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertChildNodes(workdayDate, tasks);
        }
    }

    pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));

    // CalculateAndUpdateContainerLabels(pDateStore->MondayToSundayDateRangeList);

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
        // Call Hide() incase closing of program takes longer than expected and causes
        // a bad experience for the user
        Hide();

        pLogger->info("MainFrame::OnClose - Optimize database on program exit");
        pLogger->info("MainFrame::OnClose - Open database connection at \"{0}\"", mDatabaseFilePath);

        sqlite3* db = nullptr;
        int rc = sqlite3_open(mDatabaseFilePath.c_str(), &db);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(db);
            pLogger->error(LogMessage::OpenDatabaseTemplate, "MainFrame::OnClose", mDatabaseFilePath, rc, err);
            goto cleanup;
        }

        rc = sqlite3_exec(db, Utils::sqlite::pragmas::Optimize, nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(db);
            pLogger->error(
                LogMessage::ExecQueryTemplate, "MainFrame::OnClose", Utils::sqlite::pragmas::Optimize, rc, err);
            goto cleanup;
        }

        pLogger->info("MainFrame::OnClose - Optimizimation command successfully executed on database");

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
    UI::dlg::TaskDialog newTaskDialog(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
    newTaskDialog.ShowModal();
}

void MainFrame::OnNewEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pEnv, pLogger, mDatabaseFilePath);
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

void MainFrame::OnTasksBackupDatabase(wxCommandEvent& event)
{
    if (!pCfg->BackupDatabase()) {
        wxMessageBox("Backups are toggled off!\nToggle backups in \"File\">\"Tasks\">\"Backup Database\"",
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
        pLogger->error(
            LogMessage::OpenDatabaseTemplate, "MainFrame::OnTasksBackupDatabase", mDatabaseFilePath, rc, err);
        return;
    }

    auto backupFilePath = fmt::format("{0}/{1}", pCfg->GetBackupPath(), pEnv->GetDatabaseName());
    rc = sqlite3_open(backupFilePath.c_str(), &backupDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "MainFrame::OnTasksBackupDatabase", backupFilePath, rc, err);
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

    std::string message = "Backup successful!";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(this, addNotificationEvent);
}

void MainFrame::OnTasksExportToCsv(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::ExportToCsvDialog exportToCsv(this, pCfg, pLogger, mDatabaseFilePath);
    exportToCsv.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("MainFrame::OnExit - Menu/shortcut clicked to exit program");

    Close();
}

void MainFrame::OnEditEmployer(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editEmployer(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Employer);
    editEmployer.ShowModal();
}

void MainFrame::OnEditClient(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editClient(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Client);
    editClient.ShowModal();
}

void MainFrame::OnEditProject(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editProject(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Project);
    editProject.ShowModal();
}

void MainFrame::OnEditCategory(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::EditListDialog editCategory(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Category);
    editCategory.ShowModal();
}

void MainFrame::OnViewReset(wxCommandEvent& WXUNUSED(event))
{
    wxBusyCursor wait;

    DoResetToCurrentWeek();
}

void MainFrame::OnViewExpand(wxCommandEvent& event)
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

        if (todaysDate != date::Sunday) {
            auto tomorrowsDate = todaysDate + date::days{ 1 };
            dates.push_back(date::format("%F", tomorrowsDate));
        }

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
}

void MainFrame::OnViewDay(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::DayTaskViewDialog dayTaskView(
        this, pLogger, pEnv, mDatabaseFilePath, mTaskDate.empty() ? pDateStore->PrintTodayDate : mTaskDate);
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

    UI::dlg::TaskDialog popupNewTask(this, pEnv, pCfg, pLogger, mDatabaseFilePath, false, -1, mTaskDate);
    popupNewTask.ShowModal();

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

            pLogger->info("MainFrame::OnContainerCopyToClipboard - Successfully copied \"{0}\" tasks for date \"{1}\"",
                taskModels.size(),
                mTaskDate);
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnContainerCopyTasksWithHeadersToClipboard(wxCommandEvent& event)
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

            pLogger->info("MainFrame::OnContainerCopyToClipboard - Successfully copied \"{0}\" tasks for date \"{1}\"",
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
    DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);

    int rc = taskDao.GetDescriptionById(mTaskIdToModify, description);
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

    UI::dlg::TaskDialog editTaskDialog(this, pEnv, pCfg, pLogger, mDatabaseFilePath, true, mTaskIdToModify, mTaskDate);
    int ret = editTaskDialog.ShowModal();

    if (ret == wxID_OK) {
        bool isActive = false;
        DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);
        int rc = taskDao.IsDeleted(mTaskIdToModify, isActive);
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

                CalculateStatusBarTaskDurations();
            }
        }
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnDeleteTask(wxCommandEvent& WXUNUSED(event))
{
    assert(!mTaskDate.empty());
    assert(mTaskIdToModify != -1);

    DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);

    int rc = taskDao.Delete(mTaskIdToModify);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->DeleteChild(mTaskDate, mTaskIdToModify);

        CalculateStatusBarTaskDurations();

        auto message = "Successfully deleted task";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(this, addNotificationEvent);
    }

    ResetTaskContextMenuVariables();
}

void MainFrame::OnKeyLeft(wxCommandEvent& event)
{
    pLogger->info("MainFrame::OnKeyLeft - key left event received. Going back one week.");
    // get the current week's monday date
    auto currentMondaysDate = pDateStore->MondayDate;

    // calculate last week's monday date
    auto weekBackMondaysDate = currentMondaysDate + date::weeks{ -1 };
    pLogger->info(
        "MainFrame::OnKeyLeft - Mondays date one week in the past: \"{0}\"", date::format("%F", weekBackMondaysDate));

    // date store needs to recalculate the dates for the new range
    pDateStore->ReinitializeFromWeekChange(weekBackMondaysDate);

    // update the data view control for a week change event
    OnWeekChangedProcedure();
}

void MainFrame::OnKeyRight(wxCommandEvent& event)
{
    pLogger->info("MainFrame::OnKeyRight - key right event received. Going forward one week.");
    auto currentMondaysDate = pDateStore->MondayDate;

    auto weekForwardMondaysDate = currentMondaysDate + date::weeks{ 1 };
    pLogger->info("MainFrame::OnKeyRight - Mondays date one week in the future: \"{0}\"",
        date::format("%F", weekForwardMondaysDate));
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

    NotificationClientData* notificationClientData = reinterpret_cast<NotificationClientData*>(event.GetClientObject());

    pNotificationPopupWindow->AddNotification(notificationClientData->Message, notificationClientData->Type);

    if (notificationClientData) {
        delete notificationClientData;
    }
}

void MainFrame::OnTaskDateAdded(wxCommandEvent& event)
{
    // A task got inserted for a specific day
    auto eventTaskDateAdded = event.GetString().ToStdString();
    auto taskInsertedId = static_cast<std::int64_t>(event.GetExtraLong());
    pLogger->info("MainFrame::OnTaskDateAdded - Received task added event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateAdded,
        taskInsertedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was inserted for is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateAdded; });

    // If we are in range, refetch the data for our particular date
    if (iterator != dates.end() && taskInsertedId != 0 && eventTaskDateAdded.size() != 0) {
        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskInsertedId);

        std::vector<std::string> dates{ foundDate };
        // CalculateAndUpdateContainerLabels(dates);
        CalculateStatusBarTaskDurations();
    }
}

void MainFrame::OnTaskDeletedOnDate(wxCommandEvent& event)
{
    // A task got deleted on a specific day
    auto eventTaskDateDeleted = event.GetString().ToStdString();
    auto taskDeletedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info("MainFrame::OnTaskDeletedOnDate - Received task added event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateDeleted,
        taskDeletedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was deleted is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateDeleted; });

    // If we are in range, remove the task data for our particular date
    if (iterator != dates.end() && taskDeletedId != 0 && eventTaskDateDeleted.size() != 0) {
        pLogger->info("MainFrame::OnTaskDeletedOnDate - Task deleted on a date within bounds!");

        auto& foundDate = *iterator;
        pTaskTreeModel->DeleteChild(foundDate, taskDeletedId);

        std::vector<std::string> dates{ foundDate };
        // CalculateAndUpdateContainerLabels(dates);
        CalculateStatusBarTaskDurations();
    }
}

void MainFrame::OnTaskDateChangedFrom(wxCommandEvent& event)
{
    // A task got moved from one day to another day
    auto eventTaskDateChanged = event.GetString().ToStdString();
    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info(
        "MainFrame::OnTaskDateChangedFrom - Received task date changed event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateChanged,
        taskChangedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateChanged; });

    // If we are in range, remove the item data for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedFrom - Task changed from a date within bounds!");

        auto& foundDate = *iterator;
        pTaskTreeModel->DeleteChild(foundDate, taskChangedId);

        // CalculateAndUpdateContainerLabels(dates);
        CalculateStatusBarTaskDurations();
    }
}

void MainFrame::OnTaskDateChangedTo(wxCommandEvent& event)
{
    // A task got moved from one day to another day
    auto eventTaskDateChanged = event.GetString().ToStdString();
    auto taskChangedId = static_cast<std::int64_t>(event.GetExtraLong());

    pLogger->info("MainFrame::OnTaskDateChangedTo - Received task date changed event with date \"{0}\" and ID \"{1}\"",
        eventTaskDateChanged,
        taskChangedId);

    // Check if our current from and to dates encapsulate the date the task was inserted
    // by calculating _this_ date range
    std::vector<std::string> dates = pDateStore->CalculateDatesInRange(mFromDate, mToDate);

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateChanged; });

    // If we are in range, add the task for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedTo - Task date changed to date within bounds!");

        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskChangedId);

        std::vector<std::string> dates{ foundDate };
        // CalculateAndUpdateContainerLabels(dates);
        CalculateStatusBarTaskDurations();
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
        toolTip.ShowFor(pFromDateCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();

    // Check if the selected date goes beyond six months from the current date
    auto currentDate = date::year_month_day{ date::floor<date::days>(std::chrono::system_clock::now()) };
    auto sixMonthsPastDate = currentDate - date::months{ 6 };
    auto newFromDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));

    if (newFromDate < sixMonthsPastDate) {
        int ret = wxMessageBox(
            "Are you sure you want to load tasks that are older than six (6) months?", "Confirmation", wxYES_NO, this);
        if (ret == wxNO) {
            SetFromDateAndDatePicker();
            return;
        }
    }

    mFromCtrlDate = eventDateUtc;
    mFromDate = newFromDate;

    if (mFromDate == mToDate) {
        auto date = date::format("%F", mFromDate);

        std::vector<repos::TaskRepositoryModel> tasks;
        repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

        int rc = taskRepo.FilterByDate(date, tasks);
        if (rc != 0) {
            QueueFetchTasksErrorNotificationEvent();
        } else {
            pTaskTreeModel->ClearAll();
            pTaskTreeModel->InsertRootAndChildNodes(date, tasks);

            /*std::vector<std::string> dates{ date };
            CalculateAndUpdateContainerLabels(dates);*/
        }
        return;
    }

    pLogger->info("MainFrame::OnFromDateSelection - Calculate list of dates from date: \"{0}\" to date: \"{1}\"",
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

        // CalculateAndUpdateContainerLabels(dates);
    }
}

void MainFrame::OnToDateSelection(wxDateEvent& event)
{
    pLogger->info("MainFrame::OnToDateSelection - Received date (wxDateTime) event with value \"{0}\"",
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
        toolTip.ShowFor(pToDateCtrl);
        return;
    }

    mToCtrlDate = eventDateUtc;
    auto newToDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    mToDate = newToDate;

    pLogger->info("MainFrame::OnToDateSelection - Calculate list of dates from date: \"{0}\" to date: \"{1}\"",
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

            std::vector<std::string> dates{ date };
            // CalculateAndUpdateContainerLabels(dates);
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

        // CalculateAndUpdateContainerLabels(dates);
    }
}

void MainFrame::OnContextMenu(wxDataViewEvent& event)
{
    wxDataViewItem item = event.GetItem();

    if (item.IsOk()) {
        pLogger->info("MainFrame::OnContextMenu - Clicked on valid wxDateViewItem");
        auto model = (TaskTreeModelNode*) item.GetID();

        if (model->IsContainer()) {
            pLogger->info(
                "MainFrame::OnContextMenu - Clicked on container node with date \"{0}\"", model->GetProjectName());
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
            pLogger->info("MainFrame::OnContextMenu - Clicked on leaf node with task ID \"{0}\"", model->GetTaskId());
            mTaskIdToModify = model->GetTaskId();

            // This is confusing, but by calling `GetParent()` we are getting the container node pointer here
            // `GetProjectName()` then returns the date of the container node value
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

        if (pCfg->TodayAlwaysExpanded()) {
            pLogger->info("MainFrame::OnSelectionChanged - Expand today's item node");
            pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));
        }
    }
}

void MainFrame::OnKeyDown(wxKeyEvent& event)
{
    int direction = 0;
    auto mondaysDate = pDateStore->MondayDate;

    if (event.GetKeyCode() == WXK_RIGHT) {
        direction = 1;
        mondaysDate = mondaysDate + date::weeks{ direction };
    }
    if (event.GetKeyCode() == WXK_LEFT) {
        direction = -1;
        mondaysDate = mondaysDate + date::weeks{ direction };
    }

    pLogger->info("MainFrame::OnKeyDown - new date {0}", date::format("%F", mondaysDate));

    event.Skip();
}

void MainFrame::DoResetToCurrentWeek()
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

    int rc = taskRepo.FilterByDateRange(pDateStore->MondayToSundayDateRangeList, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->ClearAll();
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertRootAndChildNodes(workdayDate, tasks);
        }

        // CalculateAndUpdateContainerLabels(pDateStore->MondayToSundayDateRangeList);
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

        std::vector<std::string> dates{ date };
        // CalculateAndUpdateContainerLabels(dates);
    }
}

void MainFrame::CalculateStatusBarTaskDurations()
{
    // All hours
    CalculateAllTaskDurations();

    // Billable
    CalculateBillableTaskDurations();
}

void MainFrame::CalculateAllTaskDurations()
{
    // Fetch tasks to calculate hours for today
    std::vector<Model::TaskDurationModel> taskDurationsForToday;
    DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);

    std::string allHoursDayTime;
    int rc =
        taskDao.GetHoursForDateRange(pDateStore->PrintTodayDate, pDateStore->PrintTodayDate, taskDurationsForToday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        allHoursDayTime = CalculateTaskDurations(taskDurationsForToday);
    }

    // Fetch tasks for current week to calculate hours
    std::vector<Model::TaskDurationModel> taskDurationsForTheWeek;

    std::string allHoursWeekTime;
    rc =
        taskDao.GetHoursForDateRange(pDateStore->PrintMondayDate, pDateStore->PrintSundayDate, taskDurationsForTheWeek);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        allHoursWeekTime = CalculateTaskDurations(taskDurationsForTheWeek);
    }

    // Fetch tasks for current month to calculate hours
    std::vector<Model::TaskDurationModel> taskDurationsForTheMonth;

    std::string allHoursMonthTime;
    rc = taskDao.GetHoursForDateRange(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth, taskDurationsForTheMonth);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        allHoursMonthTime = CalculateTaskDurations(taskDurationsForTheMonth);
    }

    pStatusBar->UpdateAllHours(allHoursDayTime, allHoursWeekTime, allHoursMonthTime);
}

void MainFrame::CalculateBillableTaskDurations()
{
    // Fetch tasks to calculate hours for today
    std::vector<Model::TaskDurationModel> taskDurationsForToday;
    DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);

    std::string billableHoursDayTime;
    int rc = taskDao.GetBillableHoursForDateRange(
        pDateStore->PrintTodayDate, pDateStore->PrintTodayDate, true, taskDurationsForToday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        billableHoursDayTime = CalculateTaskDurations(taskDurationsForToday);
    }

    // Fetch tasks for current week to calculate hours
    std::vector<Model::TaskDurationModel> taskDurationsForTheWeek;

    std::string billableHoursWeekTime;
    rc = taskDao.GetBillableHoursForDateRange(
        pDateStore->PrintMondayDate, pDateStore->PrintSundayDate, true, taskDurationsForTheWeek);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        billableHoursWeekTime = CalculateTaskDurations(taskDurationsForTheWeek);
    }

    // Fetch tasks for current month to calculate hours
    std::vector<Model::TaskDurationModel> taskDurationsForTheMonth;

    std::string billableHoursMonthTime;
    rc = taskDao.GetBillableHoursForDateRange(
        pDateStore->PrintFirstDayOfMonth, pDateStore->PrintLastDayOfMonth, true, taskDurationsForTheMonth);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        billableHoursMonthTime = CalculateTaskDurations(taskDurationsForTheMonth);
    }

    pStatusBar->UpdateBillableHours(billableHoursDayTime, billableHoursWeekTime, billableHoursMonthTime);
}

std::string MainFrame::CalculateTaskDurations(const std::vector<Model::TaskDurationModel>& taskDurations)
{
    int minutes = 0;
    int hours = 0;
    for (auto& duration : taskDurations) {
        hours += duration.Hours;
        minutes += duration.Minutes;
    }

    hours += (minutes / 60);
    minutes = minutes % 60;

    std::string formattedTotal = fmt::format("{0:02}:{1:02}", hours, minutes);
    return formattedTotal;
}

// void MainFrame::CalculateAndUpdateContainerLabels(const std::vector<std::string>& dateRange)
//{
//     std::map<std::string, std::vector<Model::TaskDurationModel>> durationsGroupedByDate;
//     DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);
//
//     int rc = taskDao.GetHoursForDateRangeGroupedByDate(dateRange, durationsGroupedByDate);
//     if (rc != 0) {
//         QueueFetchTasksErrorNotificationEvent();
//     } else {
//         // update container label here
//         for (auto& [date, taskDurationModels] : durationsGroupedByDate) {
//             int minutes = 0;
//             int hours = 0;
//             for (auto& duration : taskDurationModels) {
//                 hours += duration.Hours;
//                 minutes += duration.Minutes;
//             }
//
//             hours += (minutes / 60);
//             minutes = minutes % 60;
//             std::string formattedTime = fmt::format("{0:02}:{1:02}", hours, minutes);
//             pTaskTreeModel->ChangeContainerLabelWithTime(date, formattedTime);
//         }
//     }
// }

void MainFrame::QueueFetchTasksErrorNotificationEvent()
{
    std::string message = "Failed to fetch tasks";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(this, addNotificationEvent);
}

void MainFrame::SetFromAndToDatePickerRanges()
{
    pFromDateCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateTime fromFromDate = wxDateTime::Now(), toFromDate = wxDateTime::Now();

    if (pFromDateCtrl->GetRange(&fromFromDate, &toFromDate)) {
        pLogger->info("MainFrame::SetFromAndToDatePickerRanges - pFromDateCtrl range is [{0} - {1}]",
            fromFromDate.FormatISODate().ToStdString(),
            toFromDate.FormatISODate().ToStdString());
    }

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDateCtrl->SetRange(wxDateTime(pDateStore->MondayDateSeconds), latestPossibleDatePlusOneDay);

    wxDateTime toFromDate2 = wxDateTime::Now(), toToDate = wxDateTime::Now();

    if (pToDateCtrl->GetRange(&toFromDate2, &toToDate)) {
        pLogger->info("MainFrame::SetFromAndToDatePickerRanges - pToDateCtrl range is [{0} - {1})",
            toFromDate2.FormatISODate().ToStdString(),
            toToDate.FormatISODate().ToStdString());
    }

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void MainFrame::SetFromDateAndDatePicker()
{
    pFromDateCtrl->SetValue(pDateStore->MondayDateSeconds);

    pLogger->info("MainFrame::SetFromDateAndDatePicker - Reset pFromDateCtrl to: {0}",
        pFromDateCtrl->GetValue().FormatISODate().ToStdString());

    mFromCtrlDate = pDateStore->MondayDateSeconds;

    pLogger->info("MainFrame::SetFromDateAndDatePicker - Reset mFromCtrlDate to: {0}",
        mFromCtrlDate.FormatISODate().ToStdString());
}

void MainFrame::SetToDateAndDatePicker()
{
    pToDateCtrl->SetValue(pDateStore->SundayDateSeconds);

    pLogger->info("MainFrame::SetToDateAndDatePicker - \npToDateCtrl date = {0}\nSundayDateSeconds = {1}",
        pToDateCtrl->GetValue().FormatISOCombined().ToStdString(),
        date::format("%Y-%m-%d %I:%M:%S %p", date::sys_seconds{ std::chrono::seconds(pDateStore->SundayDateSeconds) }));

    pLogger->info("MainFrame::SetToDateAndDatePicker - Reset pToDateCtrl to: {0}",
        pToDateCtrl->GetValue().FormatISODate().ToStdString());

    mToCtrlDate = pDateStore->SundayDateSeconds;

    pLogger->info(
        "MainFrame::SetToDateAndDatePicker - Reset mToCtrlDate to: {0}", mToCtrlDate.FormatISODate().ToStdString());
}

void MainFrame::ResetTaskContextMenuVariables()
{
    mTaskIdToModify = -1;
    mTaskDate = "";
}

void MainFrame::OnWeekChangedProcedure()
{
    // Fetch tasks between from date and to date
    std::map<std::string, std::vector<repos::TaskRepositoryModel>> tasksGroupedByWorkday;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDateRange(pDateStore->MondayToSundayDateRangeList, tasksGroupedByWorkday);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskTreeModel->ClearAll();
        for (auto& [workdayDate, tasks] : tasksGroupedByWorkday) {
            pTaskTreeModel->InsertRootAndChildNodes(workdayDate, tasks);
        }

        pDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(pDateStore->PrintTodayDate));

        // CalculateAndUpdateContainerLabels(pDateStore->MondayToSundayDateRangeList);

        CalculateStatusBarTaskDurations();

        SetFromAndToDatePickerRanges();
        SetFromDateAndDatePicker();
        SetToDateAndDatePicker();
    }
}
} // namespace tks::UI
