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

#include "projectdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/constants.h"
#include "../../common/common.h"
#include "../../core/environment.h"
#include "../../utils/utils.h"

#include "errordlg.h"

namespace tks::UI::dlg
{
ProjectDialog::ProjectDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    bool isEdit,
    std::int64_t projectId,
    const wxString& name)
    : pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , bIsEdit(isEdit)
    , mProjectId(projectId)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pClientChoiceCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
    , pIsActiveCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
{
}

void ProjectDialog::Create() {}

void ProjectDialog::CreateControls() {}

void ProjectDialog::FillControls() {}

void ProjectDialog::ConfigureEventBindings() {}

void ProjectDialog::DataToControls() {}

void ProjectDialog::OnNameChange(wxCommandEvent& event) {}

void ProjectDialog::OnEmployerChoiceSelection(wxCommandEvent& event) {}

void ProjectDialog::OnIsDefaultCheck(wxCommandEvent& event) {}

void ProjectDialog::OnOK(wxCommandEvent& event) {}

void ProjectDialog::OnCancel(wxCommandEvent& event) {}

void ProjectDialog::OnIsActiveCheck(wxCommandEvent& event) {}

bool ProjectDialog::TransferDataAndValidate()
{
    return false;
}
} // namespace tks::UI::dlg
