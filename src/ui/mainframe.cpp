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

#include "events.h"
#include "../common/common.h"
#include "../core/environment.h"
#include "../core/configuration.h"

#include "../ui/dlg/errordlg.h"
#include "../ui/dlg/employerdlg.h"
#include "../ui/dlg/editlistdlg.h"

namespace tks::UI
{
// clang-format off
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
/* Menu Handlers */
EVT_MENU(ID_NEW_EMPLOYER, MainFrame::OnNewEmployer)
EVT_MENU(ID_EDIT_EMPLOYER, MainFrame::OnEditEmployer)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
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
// clang-format on
{
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(800, 600)));
    }

    wxIconBundle iconBundle("TASKIES_ICO", 0);
    SetIcons(iconBundle);
    Create();
}

bool MainFrame::Create()
{
    CreateControls();
    return true;
}

void MainFrame::CreateControls()
{
    /* Menu Controls */
    /* Menubar */
    /* File */
    auto fileMenu = new wxMenu();
    fileMenu->Append(ID_NEW_EMPLOYER, "New &Employer", "Create new employer");
    fileMenu->AppendSeparator();

    auto exitMenuItem = fileMenu->Append(wxID_EXIT, "E&xit", "Exit the program");

    /* Edit */
    auto editMenu = new wxMenu();
    editMenu->Append(ID_EDIT_EMPLOYER, "Edit &Employer", "Edit an employer");

    /* Menu bar */
    auto menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "File");
    menuBar->Append(editMenu, "Edit");

    SetMenuBar(menuBar);

    /* Main Controls */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    auto mainPanel = new wxPanel(this);
    mainPanel->SetSizer(mainSizer);

    mainSizer->Add(new wxStaticText(mainPanel, wxID_ANY, "Taskies"), wxSizerFlags().Center());
}

void MainFrame::OnNewEmployer(wxCommandEvent& event)
{
    UI::dlg::EmployerDialog newEmployerDialog(this, pEnv, pLogger);
    newEmployerDialog.ShowModal();
}

void MainFrame::OnEditEmployer(wxCommandEvent& event)
{
    UI::dlg::EditListDialog editEmployer(this, pEnv, pLogger);
    editEmployer.ShowModal();
}

void MainFrame::OnExit(wxCommandEvent& event)
{
    Close();
}

void MainFrame::OnError(wxCommandEvent& event)
{
    UI::dlg::ErrorDialog errDialog(this, pLogger, event.GetString().ToStdString());
    errDialog.ShowModal();
}
} // namespace tks::UI
