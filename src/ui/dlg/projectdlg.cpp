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

#include <vector>

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"

#include "../../utils/utils.h"

#include "errordlg.h"

namespace tks::UI::dlg
{
ProjectDialog::ProjectDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
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
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
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

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
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
    pOkButton->Disable();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

void ProjectDialog::FillControls()
{
    pEmployerChoiceCtrl->AppendString("Please select");
    pEmployerChoiceCtrl->SetSelection(0);

    std::vector<Model::EmployerModel> employers;
    std::string defaultSearhTerm = "";
    Data::EmployerData employerData(pLogger, mDatabaseFilePath);

    int rc = employerData.Filter(defaultSearhTerm, employers);
    if (rc != 0) {
        auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                            "check the logs for more information...";

        ErrorDialog errorDialog(this, pLogger, errorMessage);
        errorDialog.ShowModal();
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(employer.Name, Utils::Int64ToVoidPointer(employer.EmployerId));
        }
    }

    pOkButton->Enable();

    pClientChoiceCtrl->AppendString("Please select");
    pClientChoiceCtrl->SetSelection(0);
    pClientChoiceCtrl->Disable();
}

//clang-format off
void ProjectDialog::ConfigureEventBindings()
{
    pNameTextCtrl->Bind(
        wxEVT_TEXT,
        &ProjectDialog::OnNameChange,
        this
    );

    pEmployerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ProjectDialog::OnEmployerChoiceSelection,
        this
    );

    if (bIsEdit) {
        pIsActiveCtrl->Bind(
            wxEVT_CHECKBOX,
            &ProjectDialog::OnIsActiveCheck,
            this
        );
    }

    pOkButton->Bind(
        wxEVT_BUTTON,
        &ProjectDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &ProjectDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void ProjectDialog::DataToControls()
{
    Model::ProjectModel project;
    Data::ProjectData data(pLogger, mDatabaseFilePath);
    bool isSuccess = false;

    int rc = data.GetById(mProjectId, project);
    if (rc != 0) {
        auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                            "check the logs for more information...";

        ErrorDialog errorDialog(this, pLogger, errorMessage);
        errorDialog.ShowModal();
    } else {
        mProjectId = project.ProjectId;
        pNameTextCtrl->ChangeValue(project.Name);
        pDisplayNameCtrl->ChangeValue(project.DisplayName);
        pIsDefaultCtrl->SetValue(project.IsDefault);
        pDescriptionTextCtrl->SetValue(project.Description.has_value() ? project.Description.value() : "");
        pIsActiveCtrl->SetValue(project.IsActive);
        pDateCreatedTextCtrl->SetValue(project.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(project.GetDateModifiedString());
        isSuccess = true;
    }

    Model::EmployerModel employer;
    Data::EmployerData employerData(pLogger, mDatabaseFilePath);

    rc = employerData.GetById(project.EmployerId, employer);
    if (rc == -1) {
        pLogger->error("Failed to execute action with employer. Check further logs for more information...");
        auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                            "check logs for more information...";

        ErrorDialog errorDialog(this, pLogger, errorMessage);
        errorDialog.ShowModal();
        isSuccess = false;
    } else {
        pEmployerChoiceCtrl->SetStringSelection(employer.Name);
        isSuccess = true;
    }

    if (project.ClientId.has_value()) {
        Model::ClientModel client;
        Data::ClientData clientData(pLogger, mDatabaseFilePath);

        rc = clientData.GetById(project.ClientId.value(), client);
        if (rc == -1) {
            pLogger->error("Failed to execute action with client. Check further logs for more information...");
            auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                                "check logs for more information...";

            ErrorDialog errorDialog(this, pLogger, errorMessage);
            errorDialog.ShowModal();
            isSuccess = false;
        } else {
            pClientChoiceCtrl->SetStringSelection(client.Name);
            isSuccess = true;
        }
    }

    if (isSuccess) {
        pOkButton->Enable();
    }
}

void ProjectDialog::OnNameChange(wxCommandEvent& event)
{
    auto name = pNameTextCtrl->GetValue().ToStdString();
    pDisplayNameCtrl->ChangeValue(name);
}

void ProjectDialog::OnEmployerChoiceSelection(wxCommandEvent& event)
{
    pOkButton->Disable();
    pClientChoiceCtrl->Clear();
    pClientChoiceCtrl->AppendString("Please select");
    pClientChoiceCtrl->SetSelection(0);

    if (event.GetSelection() < 1) {
        pClientChoiceCtrl->Disable();
        pOkButton->Enable();

        return;
    }

    auto employerChoiceClientData = pEmployerChoiceCtrl->GetClientData(event.GetSelection());
    if (employerChoiceClientData == nullptr) {
        pClientChoiceCtrl->Disable();

        return;
    }

    auto employerId = Utils::VoidPointerToInt64(employerChoiceClientData);
    Data::ClientData clientData(pLogger, mDatabaseFilePath);

    std::vector<Model::ClientModel> clients;
    int rc = clientData.FilterByEmployerId(employerId, clients);

    if (rc != 0) {
        auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                            "check the logs for more information...";

        ErrorDialog errorDialog(this, pLogger, errorMessage);
        errorDialog.ShowModal();
    } else {
        if (clients.empty()) {
            pClientChoiceCtrl->Disable();
            pOkButton->Enable();

            return;
        }

        for (auto& client : clients) {
            pClientChoiceCtrl->Append(client.Name, Utils::Int64ToVoidPointer(client.ClientId));
        }

        if (!pClientChoiceCtrl->IsEnabled()) {
            pClientChoiceCtrl->Enable();
        }
    }

    pOkButton->Enable();
}

void ProjectDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    if (TransferDataAndValidate()) {
        Data::ProjectData projectData(pLogger, mDatabaseFilePath);
        int ret = 0;

        if (pIsDefaultCtrl->IsChecked()) {
            ret = projectData.UnmarkDefault();
        }

        if (!bIsEdit) {
            std::int64_t projectId = projectData.Create(mProjectModel);
            ret = projectId > 0 ? 0 : -1;
        }
        if (bIsEdit && pIsActiveCtrl->IsChecked()) {
            ret = projectData.Update(mProjectModel);
        }
        if (bIsEdit && !pIsActiveCtrl->IsChecked()) {
            ret = projectData.Delete(mProjectId);
        }

        if (ret == -1) {
            pLogger->error("Failed to execute action with project. Check further logs for more information");
            auto errorMessage = "An unexpected error occured and the specified action could not be completed. Please "
                                "check logs for more information...";

            ErrorDialog errorDialog(this, pLogger, errorMessage);
            errorDialog.ShowModal();

            pOkButton->Enable();
        } else {
            EndModal(wxID_OK);
        }
    }
}

void ProjectDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void ProjectDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pDisplayNameCtrl->Enable();
        pIsDefaultCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pEmployerChoiceCtrl->Enable();

        if (mProjectModel.ClientId.has_value()) {
            pClientChoiceCtrl->Enable();
        }

    } else {
        pNameTextCtrl->Disable();
        pDisplayNameCtrl->Disable();
        pIsDefaultCtrl->Disable();
        pDescriptionTextCtrl->Disable();
        pEmployerChoiceCtrl->Disable();
        pClientChoiceCtrl->Disable();
    }
}

bool ProjectDialog::TransferDataAndValidate()
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

    auto displayName = pDisplayNameCtrl->GetValue().ToStdString();
    if (displayName.empty()) {
        auto valMsg = "Display Name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pNameTextCtrl);
        return false;
    }

    if (displayName.length() < MIN_CHARACTER_COUNT || displayName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Display Name must be at minimum {0} or maximum {1} characters long",
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

    auto employerId =
        Utils::VoidPointerToInt64(pEmployerChoiceCtrl->GetClientData(pEmployerChoiceCtrl->GetSelection()));
    if (employerId < 1) {
        auto valMsg = "An Employer selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pEmployerChoiceCtrl);
        return false;
    }

    if (pClientChoiceCtrl->IsEnabled()) {
        auto clientChoiceCtrlData = pClientChoiceCtrl->GetClientData(pClientChoiceCtrl->GetSelection());
        if (clientChoiceCtrlData != nullptr) {
            auto clientId =
                Utils::VoidPointerToInt64(pClientChoiceCtrl->GetClientData(pClientChoiceCtrl->GetSelection()));
            mProjectModel.ClientId = std::make_optional(clientId);
        } else {
            mProjectModel.ClientId = std::nullopt;
        }
    }

    mProjectModel.Name = name;
    mProjectModel.DisplayName = displayName;
    mProjectModel.IsDefault = pIsDefaultCtrl->GetValue();
    mProjectModel.Description = description.empty() ? std::nullopt : std::make_optional(description);
    mProjectModel.EmployerId = employerId;
    mProjectModel.ProjectId = mProjectId;

    return true;
}
} // namespace tks::UI::dlg
