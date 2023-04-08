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

#include "employerdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../core/environment.h"
#include "../../common/common.h"
#include "../../common/constants.h"
#include "errordlg.h"

namespace tks::UI::dlg
{
EmployerDialog::EmployerDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    bool isEdit,
    std::int64_t employerId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Employer" : "Add Employer",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , pEnv(env)
    , bIsEdit(isEdit)
    , mEmployerId(employerId)
    , mEmployer()
    , mData(pEnv, pLogger)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
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

void EmployerDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();

    if (bIsEdit) {
        DataToControls();
    }

    //SetSize(FromDIP(wxSize(400, 380)));
    SetMinSize(FromDIP(wxSize(280, 380)));
}

void EmployerDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Details */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    sizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Employer Name Control */
    auto employerNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, IDC_NAME);
    pNameTextCtrl->SetHint("Employer name");
    pNameTextCtrl->SetToolTip("Enter a name for an Employer");

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

    detailsGridSizer->Add(employerNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Employer Description */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    sizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    pDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for an Employer");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

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
void EmployerDialog::ConfigureEventBindings()
{
    pOkButton->Bind(
        wxEVT_BUTTON,
        &EmployerDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &EmployerDialog::OnCancel,
        this,
        wxID_CANCEL
    );

    if (bIsEdit) {
        pIsActiveCtrl->Bind(
            wxEVT_CHECKBOX,
            &EmployerDialog::OnIsActiveCheck,
            this
        );
    }
}
// clang-format on

void EmployerDialog::DataToControls()
{
    pOkButton->Disable();
    pCancelButton->Disable();

    Model::EmployerModel employer;
    int rc = mData.GetById(mEmployerId, employer);
    if (rc == -1) {
        ErrorDialog errorDialog(
            this, pLogger, fmt::format("Error occured when fetching employer with ID: \"{}\"", mEmployerId));
        errorDialog.ShowModal();

        pCancelButton->Enable();
    } else {
        pNameTextCtrl->SetValue(employer.Name);
        pDescriptionTextCtrl->SetValue(employer.Description.has_value() ? employer.Description.value() : "");
        pDateCreatedTextCtrl->SetValue(employer.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(employer.GetDateModifiedString());
        pIsActiveCtrl->SetValue(employer.IsActive);

        pOkButton->Enable();
        pCancelButton->Enable();
    }
}

void EmployerDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();
    pCancelButton->Disable();

    if (TransferDataAndValidate()) {
        int ret = 0;
        if (!bIsEdit) {
            std::int64_t employerId = mData.Create(mEmployer);
            ret = static_cast<int>(employerId);
        }
        if (bIsEdit && pIsActiveCtrl->IsChecked()) {
            ret = mData.Update(mEmployer);
        }
        if (bIsEdit && !pIsActiveCtrl->IsChecked()) {
            ret = mData.Delete(mEmployerId);
        }

        if (ret == -1) {
            pLogger->error("Error occured when creating employer");
            ErrorDialog errorDialog(this, pLogger, "Error occured when creating employer");
            errorDialog.ShowModal();

            pOkButton->Enable();
            pCancelButton->Enable();
        } else {
            EndModal(wxID_OK);
        }
    }
}

void EmployerDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void EmployerDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pDescriptionTextCtrl->Enable();
    } else {
        pNameTextCtrl->Disable();
        pDescriptionTextCtrl->Disable();
    }
}

bool EmployerDialog::TransferDataAndValidate()
{
    auto name = pNameTextCtrl->GetValue().ToStdString();
    if (name.empty()) {
        auto valMsg = "Name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pNameTextCtrl);
        return false;
    }

    if (name.length() < MIN_CHARACTER_COUNT || name.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pNameTextCtrl);
        return false;
    }

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    if (!description.empty() &&
        (description.length() < MIN_CHARACTER_COUNT || description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS)) {
        auto valMsg = fmt::format("Description must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_DESCRIPTIONS);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDescriptionTextCtrl);
        return false;
    }

    mEmployer.Name = name;
    mEmployer.Description = description.empty() ? std::nullopt : std::make_optional(description);

    return true;
}
} // namespace tks::UI::dlg
