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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <spdlog/logger.h>

#include "../../models/projectmodel.h"

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core
namespace UI::dlg
{
class ProjectDialog : public wxDialog
{
public:
    ProjectDialog() = delete;
    ProjectDialog(const ProjectDialog&) = delete;
    explicit ProjectDialog(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t projectId = -1,
        const wxString& name = "projectdlg");
    virtual ~ProjectDialog() = default;

    ProjectDialog& operator=(const ProjectDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnNameChange(wxCommandEvent& event);
    void OnEmployerChoiceSelection(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnIsActiveCheck(wxCommandEvent& event);

    bool TransferDataAndValidate();

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;
    wxTextCtrl* pNameTextCtrl;
    wxTextCtrl* pDisplayNameCtrl;
    wxTextCtrl* pDescriptionTextCtrl;
    wxCheckBox* pIsDefaultCtrl;
    wxChoice* pEmployerChoiceCtrl;
    wxChoice* pClientChoiceCtrl;
    wxTextCtrl* pDateCreatedTextCtrl;
    wxTextCtrl* pDateModifiedTextCtrl;
    wxCheckBox* pIsActiveCtrl;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    Model::ProjectModel mProjectModel;
    std::int64_t mProjectId;
    bool bIsEdit;

    enum {
        tksIDC_NAME = wxID_HIGHEST + 1,
        tksIDC_DISPLAYNAME,
        tksIDC_DESCRIPTION,
        tksIDC_ISDEFAULT,
        tksIDC_EMPLOYERCHOICE,
        tksIDC_CLIENTCHOICE,
        tksIDC_ISACTIVE
    };
};
} // namespace UI::dlg
} // namespace tks
