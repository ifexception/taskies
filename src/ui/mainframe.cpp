// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

#include "events.h"
#include "notificationclientdata.h"

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
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(ID_EDIT_EMPLOYER, MainFrame::OnEditEmployer)
EVT_MENU(ID_EDIT_CLIENT, MainFrame::OnEditClient)
EVT_MENU(ID_EDIT_PROJECT, MainFrame::OnEditProject)
EVT_MENU(ID_EDIT_CATEGORY, MainFrame::OnEditCategory)
EVT_MENU(ID_VIEW_RESET, MainFrame::OnViewReset)
EVT_MENU(ID_VIEW_PREFERENCES, MainFrame::OnViewPreferences)
EVT_MENU(ID_HELP_ABOUT, MainFrame::OnAbout)
/* Popup Menu Event Handlers */
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS, MainFrame::OnContainerCopyTasksToClipboard)
EVT_MENU(ID_POP_CONTAINER_COPY_TASKS_WITH_HEADERS, MainFrame::OnContainerCopyTasksWithHeadersToClipboard)
EVT_MENU(wxID_COPY, MainFrame::OnCopyTaskToClipboard)
EVT_MENU(wxID_EDIT, MainFrame::OnEditTask)
EVT_MENU(wxID_DELETE, MainFrame::OnDeleteTask)
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
EVT_DATAVIEW_ITEM_CONTEXT_MENU(tksIDC_TASKDATAVIEW, MainFrame::OnContextMenu)
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
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath()
    , pInfoBar(nullptr)
    , pNotificationPopupWindow(nullptr)
    , pFromDateCtrl(nullptr)
    , pToDateCtrl(nullptr)
    , mBellBitmap(wxNullBitmap)
    , mBellNotificationBitmap(wxNullBitmap)
    , pDateStore(nullptr)
    , mFromDate()
    , mToDate()
    , pTaskDataViewCtrl(nullptr)
    , pTaskTreeModel(nullptr)
    , mFromCtrlDate()
    , mToCtrlDate()
    , mTaskIdToModify(-1)
    , mTaskDate()
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
    viewMenu->Append(ID_VIEW_RESET, "&Reset View\tCtrl-R", "Reset task view to current");
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

    auto panel = new wxPanel(this);
    panel->SetSizer(sizer);

    /* InfoBar */
    pInfoBar = new wxInfoBar(panel, wxID_ANY);
    sizer->Add(pInfoBar, wxSizerFlags().Expand());

    auto topSizer = new wxBoxSizer(wxHORIZONTAL);

    auto fromDateLabel = new wxStaticText(panel, wxID_ANY, "From: ");
    pFromDateCtrl = new wxDatePickerCtrl(panel, tksIDC_FROMDATE);

    auto toDateLabel = new wxStaticText(panel, wxID_ANY, "To: ");
    pToDateCtrl = new wxDatePickerCtrl(panel, tksIDC_TODATE);

    topSizer->Add(fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pFromDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    topSizer->Add(toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pToDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    topSizer->AddStretchSpacer();

    pNotificationButton = new wxBitmapButton(panel, tksIDC_NOTIFICATIONBUTTON, mBellBitmap);
    pNotificationButton->SetToolTip("View notifications");
    topSizer->Add(pNotificationButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    sizer->Add(topSizer, wxSizerFlags().Expand());

    /* Data View Ctrl */
    pTaskDataViewCtrl = new wxDataViewCtrl(panel,
        tksIDC_TASKDATAVIEW,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

    /* Data View Model */
    pTaskTreeModel = new TaskTreeModel(mFromDate, mToDate, pLogger);
    pTaskDataViewCtrl->AssociateModel(pTaskTreeModel.get());

    /* Data View Columns */
    auto projectNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto categoryNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto durationTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto descriptionTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    descriptionTextRenderer->EnableEllipsize(wxEllipsizeMode::wxELLIPSIZE_END);

    auto idRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);

    /* Project Column */
    auto projectColumn = new wxDataViewColumn(
        "Project", projectNameTextRenderer, TaskTreeModel::Col_Project, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    projectColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pTaskDataViewCtrl->AppendColumn(projectColumn);

    /* Category Column */
    auto categoryColumn = new wxDataViewColumn(
        "Category", categoryNameTextRenderer, TaskTreeModel::Col_Category, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    categoryColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pTaskDataViewCtrl->AppendColumn(categoryColumn);

    /* Duration Column */
    auto durationColumn =
        new wxDataViewColumn("Duration", durationTextRenderer, TaskTreeModel::Col_Duration, 80, wxALIGN_CENTER);
    durationColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    durationColumn->SetResizeable(false);
    pTaskDataViewCtrl->AppendColumn(durationColumn);

    /* Description Column */
    auto descriptionColumn = new wxDataViewColumn("Description",
        descriptionTextRenderer,
        TaskTreeModel::Col_Description,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    pTaskDataViewCtrl->AppendColumn(descriptionColumn);

    /* ID Column */
    auto idColumn =
        new wxDataViewColumn("ID", idRenderer, TaskTreeModel::Col_Id, 32, wxALIGN_CENTER, wxDATAVIEW_COL_HIDDEN);
    pTaskDataViewCtrl->AppendColumn(idColumn);

    sizer->Add(pTaskDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Accelerator Table */
    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_CTRL, (int) 'R', ID_VIEW_RESET);
    entries[1].Set(wxACCEL_CTRL, (int) 'N', ID_NEW_TASK);

    // ENHANCEMENT: Ctrl-G shortcut to go to particular date and view tasks
    // will open a simple dialog to ask for the date and then refresh view after date entry
    // (can only be done once "day task" feature is done)

    wxAcceleratorTable table(ARRAYSIZE(entries), entries);
    SetAcceleratorTable(table);
}

void MainFrame::FillControls()
{
    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();

    // This date was selected arbitrarily
    // wxDatePickerCtrl needs a from and to date for the range
    // So we pick 2015-01-01 as that date
    // Conceivably, a user shouldn't go that far back
    wxDateTime maxFromDate = wxDateTime::Now();
    maxFromDate.SetYear(2015);
    maxFromDate.SetMonth(wxDateTime::Jan);
    maxFromDate.SetDay(1);
    pFromDateCtrl->SetRange(maxFromDate, wxDateTime(pDateStore->SundayDateSeconds));
    pToDateCtrl->SetRange(wxDateTime(pDateStore->MondayDateSeconds), wxDateTime(pDateStore->SundayDateSeconds));
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

    pTaskDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(date::format("%F", pDateStore->TodayDate)));
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
    DoResetToCurrentWeek();
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
    }
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::AboutDialog aboutDlg(this);
    aboutDlg.ShowModal();
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
        }

        if (isActive) {
            repos::TaskRepositoryModel taskModel;
            repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

            int rc = taskRepo.GetById(mTaskIdToModify, taskModel);
            if (rc != 0) {
                QueueFetchTasksErrorNotificationEvent();
            } else {
                pTaskTreeModel->ChangeChild(mTaskDate, taskModel);
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

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        auto message = "Successfully deleted task";
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
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
    // FIXME: These date range calculations are repeated often, consider moving to DateStore
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

    // Check if date that the task was inserted for is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateAdded; });

    // If we are in range, refetch the data for our particular date
    if (iterator != dates.end() && taskInsertedId != 0 && eventTaskDateAdded.size() != 0) {
        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskInsertedId);
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
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

    // Check if date that the task was deleted is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateDeleted; });

    // If we are in range, remove the task data for our particular date
    if (iterator != dates.end() && taskDeletedId != 0 && eventTaskDateDeleted.size() != 0) {
        pLogger->info("MainFrame::OnTaskDeletedOnDate - Task deleted on a date within bounds!");

        pTaskTreeModel->DeleteChild(*iterator, taskDeletedId);
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
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateChanged; });

    // If we are in range, remove the item data for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedFrom - Task changed from a date within bounds!");

        pTaskTreeModel->DeleteChild(*iterator, taskChangedId);
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
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

    // Check if date that the task was changed to is in the selected range of our wxDateTimeCtrl's
    auto iterator =
        std::find_if(dates.begin(), dates.end(), [&](const std::string& date) { return date == eventTaskDateChanged; });

    // If we are in range, add the task for our particular date
    if (iterator != dates.end() && taskChangedId != 0 && eventTaskDateChanged.size() != 0) {
        pLogger->info("MainFrame::OnTaskDateChangedTo - Task date changed to date within bounds!");
        auto& foundDate = *iterator;
        RefetchTasksForDate(foundDate, taskChangedId);
    }
}

void MainFrame::OnFromDateSelection(wxDateEvent& event)
{
    pLogger->info("MainFrame::OnFromDateSelection - Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());

    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);
    auto eventDateUtcTicks = eventDateUtc.GetTicks();

    if (eventDateUtc > mToCtrlDate) {
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot exceed to date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pFromDateCtrl);
        return;
    }

    auto currentDate = wxDateTime::Now();
    auto sixMonthsPast = currentDate.Subtract(wxDateSpan::Months(6));

    bool isYearMoreThanSixMonths = eventDateUtc.GetYear() < sixMonthsPast.GetYear();
    bool isMonthMoreThanSixMonths = eventDateUtc.GetMonth() < sixMonthsPast.GetMonth();

    if (isYearMoreThanSixMonths || isMonthMoreThanSixMonths) {
        int ret = wxMessageBox(
            "Are you sure you want to load tasks that are older than six (6) months?", "Confirmation", wxYES_NO, this);
        if (ret == wxNO) {
            SetFromDateAndDatePicker();
            return;
        }
    }

    mFromCtrlDate = eventDateUtc;
    auto newFromDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    mFromDate = newFromDate;

    pLogger->info("MainFrame::OnFromDateSelection - Calculate list of dates from date: \"{0}\" to date: \"{1}\"",
        date::format("%F", mFromDate),
        date::format("%F", mToDate));

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
        }
        return;
    }

    // Calculate list of dates between from and to date
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

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
}

void MainFrame::OnToDateSelection(wxDateEvent& event)
{
    pLogger->info("MainFrame::OnToDateSelection - Received date (wxDateTime) event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());

    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);
    auto eventDateUtcTicks = eventDateUtc.GetTicks();

    pToDateCtrl->SetRange(mFromCtrlDate, wxDateTime(pDateStore->SundayDateSeconds));

    if (eventDateUtc < mFromCtrlDate) {
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
        }
        return;
    }

    // Calculate list of dates between from and to date
    std::vector<std::string> dates;
    auto dateIterator = mFromDate;
    int loopIdx = 0;

    do {
        dates.push_back(date::format("%F", dateIterator));

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != mToDate);

    dates.push_back(date::format("%F", dateIterator));

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

            wxMenu menu;
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

void MainFrame::DoResetToCurrentWeek()
{
    pDateStore->Reset();

    bool shouldReset = false;

    if (mFromDate != pDateStore->MondayDate) {
        shouldReset = true;
    }

    if (mToDate != pDateStore->SundayDate) {
        shouldReset = true;
    }

    if (shouldReset) {
        ResetDateRange();
        ResetDatePickerValues();
        RefetchTasksForDateRange();

        pTaskDataViewCtrl->Expand(pTaskTreeModel->TryExpandTodayDateNode(date::format("%F", pDateStore->TodayDate)));
    }
}

void MainFrame::ResetDateRange()
{
    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;
}

void MainFrame::ResetDatePickerValues()
{
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

void MainFrame::QueueFetchTasksErrorNotificationEvent()
{
    std::string message = "Failed to fetch tasks";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(this, addNotificationEvent);
}

void MainFrame::SetFromDateAndDatePicker()
{
    pFromDateCtrl->SetValue(pDateStore->MondayDateSeconds);

    mFromCtrlDate = pDateStore->MondayDateSeconds;
}

void MainFrame::SetToDateAndDatePicker()
{
    pToDateCtrl->SetValue(pDateStore->SundayDateSeconds);

    mToCtrlDate = pDateStore->SundayDateSeconds;
}

void MainFrame::ResetTaskContextMenuVariables()
{
    mTaskIdToModify = -1;
    mTaskDate = "";
}
} // namespace tks::UI
