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
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Project" : "Add Project",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
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
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetIconBundleName(), 0);
    SetIcons(iconBundle);
}

void ProjectDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();

    if (bIsEdit) {
        DataToControls();
    }
}

void ProjectDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Details Box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    sizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Project Name Ctrl */
    auto projectNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, IDC_NAME);
    pNameTextCtrl->SetHint("Project name");
    pNameTextCtrl->SetToolTip("Enter a name for a Project");

    wxTextValidator nameValidator(wxFILTER_ALPHANUMERIC | wxFILTER_INCLUDE_CHAR_LIST);
    wxArrayString allowedCharacters;
    allowedCharacters.Add(" ");
    allowedCharacters.Add("-");
    allowedCharacters.Add(":");
    allowedCharacters.Add(";");
    allowedCharacters.Add(".");
    allowedCharacters.Add("|");
    allowedCharacters.Add("(");
    allowedCharacters.Add(")");
    allowedCharacters.Add("+");
    nameValidator.SetIncludes(allowedCharacters);

    pNameTextCtrl->SetValidator(nameValidator);

    /* Display Name Ctrl */
    auto displayNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Display Name");

    pDisplayNameCtrl = new wxTextCtrl(detailsBox, IDC_DISPLAYNAME);
    pDisplayNameCtrl->SetHint("Display name");
    pDisplayNameCtrl->SetToolTip("Enter a nickname, abbreviation or common name for a project");

    /* Is Default Checkbox Ctrl */
    pIsDefaultCtrl = new wxCheckBox(detailsBox, IDC_ISDEFAULT, "Is Default");
    pIsDefaultCtrl->SetToolTip("Enabling this option for a project will auto-select this project on task item entry");

    /* Details Grid Sizer */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(projectNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(displayNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pDisplayNameCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsDefaultCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Project Description control */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    sizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    pDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a Project");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Files */

    /* Choices */
    auto choiceBox = new wxStaticBox(this, wxID_ANY, "Selection");
    auto choiceBoxSizer = new wxStaticBoxSizer(choiceBox, wxVERTICAL);
    sizer->Add(choiceBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Employer choice control */
    auto employerLabel = new wxStaticText(choiceBox, wxID_ANY, "Employer");

    pEmployerChoiceCtrl = new wxChoice(choiceBox, IDC_EMPLOYERCHOICE);
    pEmployerChoiceCtrl->SetToolTip("Select an Employer to associate this Project with");

    /* Client choice control */
    auto clientLabel = new wxStaticText(choiceBox, wxID_ANY, "Client");

    pClientChoiceCtrl = new wxChoice(choiceBox, IDC_CLIENTCHOICE);
    pClientChoiceCtrl->SetToolTip("Select an (optional) Client to associate this Project with");

    auto choiceGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    choiceGridSizer->AddGrowableCol(1, 1);

    choiceGridSizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    choiceGridSizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    choiceGridSizer->Add(clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    choiceGridSizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    choiceBoxSizer->Add(choiceGridSizer, wxSizerFlags().Expand().Proportion(1));

    if (bIsEdit) {
        auto metadataLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(3), FromDIP(3)));
        sizer->Add(metadataLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

        auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
        auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
        sizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

        /* FlexGrid sizer */
        auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
        metadataBoxSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand().Proportion(1));
        metadataFlexGridSizer->AddGrowableCol(1, 1);

        /* Date Created */
        auto dateCreatedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Created");
        metadataFlexGridSizer->Add(dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

        pDateCreatedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
        pDateCreatedTextCtrl->Disable();
        metadataFlexGridSizer->Add(pDateCreatedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

        /* Date Modified */
        auto dateModifiedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Modified");
        metadataFlexGridSizer->Add(dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

        pDateModifiedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
        pDateModifiedTextCtrl->Disable();
        metadataFlexGridSizer->Add(pDateModifiedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

        /* Is Active checkbox control */
        metadataFlexGridSizer->Add(0, 0);

        pIsActiveCtrl = new wxCheckBox(metadataBox, IDC_ISACTIVE, "Is Active");
        pIsActiveCtrl->SetToolTip("Indicates if this employer entry is being used");
        metadataFlexGridSizer->Add(pIsActiveCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)));
    }

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    pOkButton->Disable();
    pCancelButton->Disable();

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

void ProjectDialog::FillControls()
{
    pEmployerChoiceCtrl->AppendString("Please select");
    pEmployerChoiceCtrl->SetSelection(0);

    pClientChoiceCtrl->AppendString("Please select");
    pClientChoiceCtrl->SetSelection(0);
}

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
