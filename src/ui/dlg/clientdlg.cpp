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

#include "clientdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../core/environment.h"

namespace tks::UI::dlg
{
ClientDialog::ClientDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    bool isEdit,
    std::int64_t clientId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Client" : "Add Client",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , bIsEdit(isEdit)
    , mClientId(clientId)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
    , pIsActiveCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
{
    Create();

    wxIconBundle iconBundle(Common::GetIconBundleName(), 0);
    SetIcons(iconBundle);
}

void ClientDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();

    if (bIsEdit) {
        DataToControls();
    }
}

void ClientDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Details */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    sizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Client Name control */
    auto clientNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, IDC_NAME);
    pNameTextCtrl->SetHint("Client name");
    pNameTextCtrl->SetToolTip("Enter a name for a Client");

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
    nameValidator.SetIncludes(allowedCharacters);

    pNameTextCtrl->SetValidator(nameValidator);

    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(clientNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Client Description control */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    sizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    pDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a Client");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Employer selection */
    auto employerChoiceBox = new wxStaticBox(this, wxID_ANY, "Employer selection");
    auto employerChoiceBoxSizer = new wxStaticBoxSizer(employerChoiceBox, wxVERTICAL);
    sizer->Add(employerChoiceBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Employer choice control */
    auto employerLabel = new wxStaticText(employerChoiceBox, wxID_ANY, "Employer");

    pEmployerChoiceCtrl = new wxChoice(employerChoiceBox, IDC_CHOICE);
    pEmployerChoiceCtrl->AppendString("Please select");
    pEmployerChoiceCtrl->SetSelection(0);
    pEmployerChoiceCtrl->SetToolTip("Select an Employer to associate this Client with");

    auto employerChoiceGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    employerChoiceGridSizer->AddGrowableCol(1, 1);

    employerChoiceGridSizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    employerChoiceGridSizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    employerChoiceBoxSizer->Add(employerChoiceGridSizer, wxSizerFlags().Expand().Proportion(1));

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
        metadataFlexGridSizer->Add(dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

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

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

// clang-format off
void ClientDialog::ConfigureEventBindings()
{
    pOkButton->Bind(
        wxEVT_BUTTON,
        &ClientDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &ClientDialog::OnCancel,
        this,
        wxID_CANCEL
    );

    if (bIsEdit) {
        pIsActiveCtrl->Bind(
            wxEVT_CHECKBOX,
            &ClientDialog::OnIsActiveCheck,
            this
        );
    }
}
// clang-format on

void ClientDialog::DataToControls() {}

void ClientDialog::OnOK(wxCommandEvent& event) {}

void ClientDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void ClientDialog::OnIsActiveCheck(wxCommandEvent& event) {}

bool ClientDialog::TransferDataAndValidate()
{
    return false;
}
} // namespace tks::UI::dlg
