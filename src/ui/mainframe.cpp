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

#include <wx/persist/toplevel.h>

#include <sqlite3.h>

#include "events.h"
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

namespace tks::UI
{
// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
/* Menu Handlers */
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
/* Error Event Handler */
EVT_COMMAND(wxID_ANY, tksEVT_ERROR, MainFrame::OnError)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxFrame(nullptr, wxID_ANY, Common::GetProgramName(), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, name)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , pInfoBar(nullptr)
// clang-format on
{
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(800, 600)));
    }

    wxIconBundle iconBundle("TASKIES_ICO", 0);
    SetIcons(iconBundle);

    Create();
}

void MainFrame::Create()
{
    CreateControls();
    DataToControls();
}

void MainFrame::CreateControls()
{
    /* Menu Controls */
    /* Menubar */
    /* File */
    auto fileMenu = new wxMenu();
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
    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    auto mainPanel = new wxPanel(this);
    mainPanel->SetSizer(mainSizer);

    /* InfoBar */
    pInfoBar = new wxInfoBar(mainPanel, wxID_ANY);
    mainSizer->Add(pInfoBar, wxSizerFlags().Expand());

    mainSizer->Add(new wxStaticText(mainPanel, wxID_ANY, "Taskies"), wxSizerFlags().Center());
}

void MainFrame::DataToControls()
{
    // Set InfoBar
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
        auto infoBarMessage = fmt::format("Running {0} {1} - v{2}.{3}.{4}",
            Common::GetProgramName(),
            tks::BuildConfigurationToString(pEnv->GetBuildConfiguration()),
            TASKIES_MAJOR,
            TASKIES_MINOR,
            TASKIES_PATCH);
        pInfoBar->ShowMessage(infoBarMessage, wxICON_INFORMATION);
    }
}

void MainFrame::OnNewEmployer(wxCommandEvent& event)
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pEnv, pLogger);
    newEmployerDialog.ShowModal();
}

void MainFrame::OnNewClient(wxCommandEvent& event)
{
    UI::dlg::ClientDialog newClientDialog(this, pEnv, pLogger);
    newClientDialog.ShowModal();
}

void MainFrame::OnNewProject(wxCommandEvent& event)
{
    UI::dlg::ProjectDialog newProjectDialog(this, pEnv, pLogger);
    newProjectDialog.ShowModal();
}

void MainFrame::OnNewCategory(wxCommandEvent& event)
{
    UI::dlg::CategoriesDialog addCategories(this, pEnv, pLogger);
    addCategories.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    sqlite3* db = nullptr;
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &db);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "MainFrame",
            pEnv->GetDatabaseName(),
            pEnv->GetDatabasePath().string(),
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(db, Utils::sqlite::pragmas::Optimize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::ExecQueryTemplate, "MainFrame", Utils::sqlite::pragmas::Optimize, rc, err);
    }

    sqlite3_close(db);

    Close();
}

void MainFrame::OnEditEmployer(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editEmployer(this, pEnv, pLogger, EditListEntityType::Employer);
    editEmployer.ShowModal();
}

void MainFrame::OnEditClient(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editClient(this, pEnv, pLogger, EditListEntityType::Client);
    editClient.ShowModal();
}

void MainFrame::OnEditProject(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editProject(this, pEnv, pLogger, EditListEntityType::Project);
    editProject.ShowModal();
}

void MainFrame::OnEditCategory(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editCategory(this, pEnv, pLogger, EditListEntityType::Category);
    editCategory.ShowModal();
}

void MainFrame::OnViewPreferences(wxCommandEvent& event)
{
    UI::dlg::PreferencesDialog preferencesDlg(this, pEnv, pCfg, pLogger);
    preferencesDlg.ShowModal();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
    UI::dlg::AboutDialog aboutDlg(this);
    aboutDlg.ShowModal();
}

void MainFrame::OnError(wxCommandEvent& event)
{
    UI::dlg::ErrorDialog errDialog(this, pLogger, event.GetString().ToStdString());
    errDialog.ShowModal();
}
} // namespace tks::UI
