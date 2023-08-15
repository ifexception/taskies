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

#include <date/date.h>

#include <wx/artprov.h>
#include <wx/persist/toplevel.h>
#include <wx/taskbarbutton.h>

#include <sqlite3.h>

#include "../common/common.h"
#include "../common/constants.h"
#include "../common/enums.h"
#include "../common/version.h"

#include "../core/environment.h"
#include "../core/configuration.h"

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
/* Menu Handlers */
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
EVT_MENU(ID_VIEW_PREFERENCES, MainFrame::OnViewPreferences)
EVT_MENU(ID_HELP_ABOUT, MainFrame::OnAbout)
/* Error Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ERROR, MainFrame::OnError)
/* Custom Event Handlers */
EVT_COMMAND(wxID_ANY, tksEVT_ADDNOTIFICATION, MainFrame::OnAddNotification)
/* Control Event Handlers */
EVT_BUTTON(tksIDC_NOTIFICATIONBUTTON, MainFrame::OnNotificationClick)
EVT_DATE_CHANGED(tksIDC_FROMDATE, MainFrame::OnFromDateSelection)
EVT_DATE_CHANGED(tksIDC_TODATE, MainFrame::OnToDateSelection)
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
    , mFromDate()
    , mToDate()
    , pTaskDataViewCtrl(nullptr)
    , pTaskTreeModel(nullptr)
// clang-format on
{
    // Initialization setup
    SetMinSize(wxSize(FromDIP(320), FromDIP(320)));
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        pLogger->info("MainFrame - No persistent objects found. Set default size \"{0}\"x\"{1}\"", 800, 600);
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
    mDatabaseFilePath =
        pCfg->GetDatabasePath().empty() ? pEnv->GetDatabasePath().string() : pCfg->GetFullDatabasePath();
    pLogger->info("MainFrame - Database location \"{0}\"", mDatabaseFilePath);

    // Setup TaskBarIcon
    pTaskBarIcon = new TaskBarIcon(this, pEnv, pCfg, pLogger, mDatabaseFilePath);
    if (pCfg->ShowInTray()) {
        pLogger->info("MainFrame - TaskBarIcon \"ShowInTray\" is \"{0}\"", pCfg->ShowInTray());
        pTaskBarIcon->SetTaskBarIcon();
    }

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
    fileMenu->Append(ID_NEW_TASK, "New &Task", "Create new task");
    fileMenu->AppendSeparator();
    auto fileNewMenu = new wxMenu();
    fileNewMenu->Append(ID_NEW_EMPLOYER, "New E&mployer", "Create new employer");
    fileNewMenu->Append(ID_NEW_CLIENT, "New C&lient", "Create new client");
    fileNewMenu->Append(ID_NEW_PROJECT, "New &Project", "Create new project");
    fileNewMenu->Append(ID_NEW_CATEGORY, "New Cate&gory", "Create new category");
    fileMenu->AppendSubMenu(fileNewMenu, "&New");
    fileMenu->AppendSeparator();
    auto exitMenuItem = fileMenu->Append(wxID_EXIT, "E&xit", "Exit the program");

    wxIconBundle exitIconBundle(Common::GetExitIconBundleName(), 0);
    exitMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(exitIconBundle));

    /* Edit */
    auto editMenu = new wxMenu();
    editMenu->Append(ID_EDIT_EMPLOYER, "Edit E&mployer", "Edit an employer");
    editMenu->Append(ID_EDIT_CLIENT, "Edit C&lient", "Edit a client");
    editMenu->Append(ID_EDIT_PROJECT, "Edit &Project", "Edit a project");
    editMenu->Append(ID_EDIT_CATEGORY, "Edit Cate&gory", "Edit a category");

    /* View */
    auto viewMenu = new wxMenu();
    viewMenu->Append(ID_VIEW_PREFERENCES, "Pre&ferences", "View and adjust program options");

    /* Help */
    auto helpMenu = new wxMenu();
    helpMenu->Append(ID_HELP_ABOUT, "&About", "Information about Taskies");

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
    topSizer->Add(pNotificationButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    sizer->Add(topSizer, wxSizerFlags().Expand());

    /* Data View Ctrl */
    pTaskDataViewCtrl = new wxDataViewCtrl(panel,
        tksIDC_TASKDATAVIEW,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

    /* Data View Model */
    pTaskTreeModel = new TaskTreeModel(mFromDate, mToDate);
    pTaskDataViewCtrl->AssociateModel(pTaskTreeModel.get());

    /* Data View Columns */
    auto projectNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto categoryNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto durationTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto descriptionTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    descriptionTextRenderer->EnableEllipsize(wxEllipsizeMode::wxELLIPSIZE_END);

    auto idRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);

    /* Project Column */
    auto projectColumn = new wxDataViewColumn("Project",
        projectNameTextRenderer,
        TaskTreeModel::Col_Project,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    projectColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pTaskDataViewCtrl->AppendColumn(projectColumn);

    /* Category Column */
    auto categoryColumn = new wxDataViewColumn("Category",
        categoryNameTextRenderer,
        TaskTreeModel::Col_Category,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    categoryColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pTaskDataViewCtrl->AppendColumn(categoryColumn);

    /* Duration Column */
    auto durationColumn =
        new wxDataViewColumn("Duration", durationTextRenderer, TaskTreeModel::Col_Duration, 80, wxALIGN_LEFT);
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
    auto idColumn = new wxDataViewColumn(
        "ID", idRenderer, TaskTreeModel::Col_Id, 32, wxALIGN_CENTER, wxDATAVIEW_COL_HIDDEN);
    pTaskDataViewCtrl->AppendColumn(idColumn);

    sizer->Add(pTaskDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());
}

void MainFrame::FillControls()
{
    CalculateAndSetMondayAndSundayFromCurrentDate();
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
}

void MainFrame::OnClose(wxCloseEvent& event)
{
    pLogger->info("MainFrame - Closing program");
    if (pCfg->CloseToTray() && pCfg->ShowInTray() && event.CanVeto()) {
        pLogger->info("MainFrame - Closing program to tray area");
        Hide();
        MSWGetTaskBarButton()->Hide();
    } else {
        Hide();
        event.Skip();
    }
}

void MainFrame::OnIconize(wxIconizeEvent& event)
{
    pLogger->info("MainFrame - Iconize program");
    if (event.IsIconized() && pCfg->ShowInTray() && pCfg->MinimizeToTray()) {
        pLogger->info("MainFrame - Iconize program to tray area");
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
    UI::dlg::TaskDialog newTaskDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newTaskDialog.ShowModal();
}

void MainFrame::OnNewEmployer(wxCommandEvent& event)
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newEmployerDialog.ShowModal();
}

void MainFrame::OnNewClient(wxCommandEvent& event)
{
    UI::dlg::ClientDialog newClientDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newClientDialog.ShowModal();
}

void MainFrame::OnNewProject(wxCommandEvent& event)
{
    UI::dlg::ProjectDialog newProjectDialog(this, pEnv, pLogger, mDatabaseFilePath);
    newProjectDialog.ShowModal();
}

void MainFrame::OnNewCategory(wxCommandEvent& event)
{
    UI::dlg::CategoriesDialog addCategories(this, pEnv, pLogger, mDatabaseFilePath);
    addCategories.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    pLogger->info("MainFrame - Optimize database on program exit");
    pLogger->info("MainFrame - Open database connection at \"{0}\"", mDatabaseFilePath);

    sqlite3* db = nullptr;
    int rc = sqlite3_open(mDatabaseFilePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "MainFrame", mDatabaseFilePath, rc, err);
        goto cleanup;
    }

    rc = sqlite3_exec(db, Utils::sqlite::pragmas::Optimize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::ExecQueryTemplate, "MainFrame", Utils::sqlite::pragmas::Optimize, rc, err);
    }

cleanup:
    sqlite3_close(db);
    Close();
}

void MainFrame::OnEditEmployer(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editEmployer(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Employer);
    editEmployer.ShowModal();
}

void MainFrame::OnEditClient(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editClient(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Client);
    editClient.ShowModal();
}

void MainFrame::OnEditProject(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editProject(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Project);
    editProject.ShowModal();
}

void MainFrame::OnEditCategory(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editCategory(this, pEnv, pLogger, mDatabaseFilePath, EditListEntityType::Category);
    editCategory.ShowModal();
}

void MainFrame::OnViewPreferences(wxCommandEvent& event)
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

void MainFrame::OnAbout(wxCommandEvent& event)
{
    UI::dlg::AboutDialog aboutDlg(this);
    aboutDlg.ShowModal();
}

void MainFrame::OnError(wxCommandEvent& event)
{
    UI::dlg::ErrorDialog errDialog(this, pEnv, pLogger, event.GetString().ToStdString());
    errDialog.ShowModal();
}

void MainFrame::OnAddNotification(wxCommandEvent& event)
{
    pLogger->info("MainFrame - Received notification event");

    pNotificationButton->SetBitmap(mBellNotificationBitmap);

    NotificationClientData* notificationClientData = reinterpret_cast<NotificationClientData*>(event.GetClientObject());

    pNotificationPopupWindow->AddNotification(notificationClientData->Message, notificationClientData->Type);

    if (notificationClientData) {
        delete notificationClientData;
    }
}

void MainFrame::OnFromDateSelection(wxDateEvent& event)
{
    pLogger->info("MainFrame:OnFromDateSelection - Received date event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());
}

void MainFrame::OnToDateSelection(wxDateEvent& event) {}

void MainFrame::CalculateAndSetMondayAndSundayFromCurrentDate()
{
    wxDateTime currentDate = wxDateTime::Now();
    int daysToGoBackToMonday = 0;
    int daysToGoToSunday = 0;

    switch (currentDate.GetWeekDay()) {
    case wxDateTime::Mon:
        daysToGoBackToMonday = 0;
        break;
    case wxDateTime::Tue:
        daysToGoBackToMonday = 1;
        break;
    case wxDateTime::Wed:
        daysToGoBackToMonday = 2;
        break;
    case wxDateTime::Thu:
        daysToGoBackToMonday = 3;
        break;
    case wxDateTime::Fri:
        daysToGoBackToMonday = 4;
        break;
    case wxDateTime::Sat:
        daysToGoBackToMonday = 5;
        break;
    case wxDateTime::Sun:
        daysToGoBackToMonday = 6;
        break;
    case wxDateTime::Inv_WeekDay:
    default:
        break;
    }

    switch (currentDate.GetWeekDay()) {
    case wxDateTime::Mon:
        daysToGoToSunday = 6;
        break;
    case wxDateTime::Tue:
        daysToGoToSunday = 5;
        break;
    case wxDateTime::Wed:
        daysToGoToSunday = 4;
        break;
    case wxDateTime::Thu:
        daysToGoToSunday = 3;
        break;
    case wxDateTime::Fri:
        daysToGoToSunday = 2;
        break;
    case wxDateTime::Sat:
        daysToGoToSunday = 1;
        break;
    case wxDateTime::Sun:
        daysToGoToSunday = 0;
        break;
    case wxDateTime::Inv_WeekDay:
    default:
        break;
    }

    wxDateSpan toGoBackToMonday(0, 0, 0, daysToGoBackToMonday);
    wxDateTime mondayDate = currentDate.Subtract(toGoBackToMonday);

    wxDateSpan toGoToSunday(0, 0, 0, daysToGoToSunday);
    wxDateTime sundayDate = currentDate.Add(toGoToSunday);

    auto mondayTimeT = mondayDate.GetTicks();
    auto sundayTimeT = sundayDate.GetTicks();

    mFromDate = date::year_month_day{ date::floor<date::days>(std::chrono::system_clock::from_time_t(mondayTimeT)) };
    mToDate = date::year_month_day{ date::floor<date::days>(std::chrono::system_clock::from_time_t(sundayTimeT)) };

    pFromDateCtrl->SetValue(mondayDate);
    pToDateCtrl->SetValue(sundayDate);
}
} // namespace tks::UI
