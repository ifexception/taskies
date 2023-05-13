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

#pragma once

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/infobar.h>

#include <spdlog/spdlog.h>

namespace tks
{
enum class MenuIds : int {
    File_NewEmployer = wxID_HIGHEST + 102,
    File_NewClient,
    File_NewProject,
    File_NewCategory,
    Edit_Employer,
    Edit_Client,
    Edit_Project,
    Edit_Category
};

/* File */
static const int ID_NEW_EMPLOYER = static_cast<int>(MenuIds::File_NewEmployer);
static const int ID_NEW_CLIENT = static_cast<int>(MenuIds::File_NewClient);
static const int ID_NEW_PROJECT = static_cast<int>(MenuIds::File_NewProject);
static const int ID_NEW_CATEGORY = static_cast<int>(MenuIds::File_NewCategory);

/* Edit */
static const int ID_EDIT_EMPLOYER = static_cast<int>(MenuIds::Edit_Employer);
static const int ID_EDIT_CLIENT = static_cast<int>(MenuIds::Edit_Client);
static const int ID_EDIT_PROJECT = static_cast<int>(MenuIds::Edit_Project);
static const int ID_EDIT_CATEGORY = static_cast<int>(MenuIds::Edit_Category);

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
    virtual ~MainFrame() = default;

    MainFrame& operator=(const MainFrame&) = delete;

private:
    wxDECLARE_EVENT_TABLE();

    void Create();

    void CreateControls();
    void DataToControls();

    /* Event Table Handlers */
    /* Menu Handlers */
    void OnNewEmployer(wxCommandEvent& event);
    void OnNewClient(wxCommandEvent& event);
    void OnNewProject(wxCommandEvent& event);
    void OnNewCategory(wxCommandEvent& event);
    void OnEditEmployer(wxCommandEvent& event);
    void OnEditClient(wxCommandEvent& event);
    void OnEditProject(wxCommandEvent& event);
    void OnEditCategory(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    /* Error Event Handler */
    void OnError(wxCommandEvent& event);

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;

    wxInfoBar* pInfoBar;
};
} // namespace UI
} // namespace tks
