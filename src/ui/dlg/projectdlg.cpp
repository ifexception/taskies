// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include <fmt/format.h>

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../events.h"
#include "../common/clientdata.h"
#include "../common/notificationclientdata.h"

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/clientspersistence.h"
#include "../../persistence/projectspersistence.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"

#include "../../utils/utils.h"

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
          isEdit ? "Edit Project" : "New Project",
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
    , pDisplayNameCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pClientChoiceCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
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

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Project name");
    pNameTextCtrl->SetToolTip("Enter a name for a project");
    pNameTextCtrl->SetValidator(NameValidator());

    /* Display Name Ctrl */
    auto displayNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Display Name");

    pDisplayNameCtrl = new wxTextCtrl(detailsBox, tksIDC_DISPLAYNAMETEXTCTRL);
    pDisplayNameCtrl->SetHint("Display name");
    pDisplayNameCtrl->SetToolTip(
        "Enter a nickname, abbreviation or common name for a project (if applicable)");
    pDisplayNameCtrl->SetValidator(NameValidator());

    /* Is Default Checkbox Ctrl */
    pIsDefaultCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_ISDEFAULTCHECKBOXCTRL, "Is Default");
    pIsDefaultCheckBoxCtrl->SetToolTip(
        "Enabling this option for a project will auto-select it on a task entry");

    /* Details Grid Sizer */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        projectNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    detailsGridSizer->Add(
        displayNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pDisplayNameCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Project Description control */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    sizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTIONTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a project");
    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Employer choice control */
    auto employerLabel = new wxStaticText(this, wxID_ANY, "Employer");

    pEmployerChoiceCtrl = new wxChoice(this, tksIDC_EMPLOYERCHOICECTRL);
    pEmployerChoiceCtrl->SetToolTip("Select an employer to associate this project with");

    /* Client choice control */
    auto clientLabel = new wxStaticText(this, wxID_ANY, "Client");

    pClientChoiceCtrl = new wxChoice(this, tksIDC_CLIENTCHOICECTRL);
    pClientChoiceCtrl->SetToolTip("Select an (optional) client to associate this project with");

    sizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    sizer->Add(clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    sizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line2, wxSizerFlags().Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void ProjectDialog::FillControls()
{
    pEmployerChoiceCtrl->Append("Select employer", new ClientData<std::int64_t>(0));
    pEmployerChoiceCtrl->SetSelection(0);

    std::string defaultSearchTerm = "";
    bool hasDefaultEmployer = false;
    std::int64_t defaultEmployerId = -1;
    std::vector<Model::EmployerModel> employers;

    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.Filter(defaultSearchTerm, employers);
    if (rc != 0) {
        std::string message = "Failed to get employers";

        QueueErrorNotificationEvent(message);
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(
                employer.Name, new ClientData<std::int64_t>(employer.EmployerId));

            if (employer.IsDefault) {
                pEmployerChoiceCtrl->SetStringSelection(employer.Name);
                hasDefaultEmployer = true;
                defaultEmployerId = employer.EmployerId;
            }
        }
    }

    pClientChoiceCtrl->Append("Select client", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);
    pClientChoiceCtrl->Disable();

    if (!bIsEdit && hasDefaultEmployer) {
        FillClientChoiceControl(defaultEmployerId);
    }
}

// clang-format off
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

    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ProjectDialog::OnIsActiveCheck,
        this
    );

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
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.GetById(mProjectId, mProjectModel);
    if (rc != 0) {
        std::string message = "Failed to get project";

        QueueErrorNotificationEvent(message);
    } else {
        pNameTextCtrl->ChangeValue(mProjectModel.Name);
        pDisplayNameCtrl->ChangeValue(mProjectModel.DisplayName);
        pIsDefaultCheckBoxCtrl->SetValue(mProjectModel.IsDefault);

        if (mProjectModel.Description.has_value()) {
            pDescriptionTextCtrl->SetValue(mProjectModel.Description.value());
        }

        pIsActiveCheckBoxCtrl->SetValue(mProjectModel.IsActive);
        pIsActiveCheckBoxCtrl->Enable();

        for (unsigned int i = 0; i < pEmployerChoiceCtrl->GetCount(); i++) {
            auto* data = reinterpret_cast<ClientData<std::int64_t>*>(
                pEmployerChoiceCtrl->GetClientObject(i));

            if (mProjectModel.EmployerId == data->GetValue()) {
                pEmployerChoiceCtrl->SetSelection(i);
                break;
            }
        }

        FillClientChoiceControl(mProjectModel.EmployerId);

        if (mProjectModel.ClientId.has_value()) {
            for (unsigned int i = 0; i < pClientChoiceCtrl->GetCount(); i++) {
                auto* data = reinterpret_cast<ClientData<std::int64_t>*>(
                    pClientChoiceCtrl->GetClientObject(i));

                if (mProjectModel.ClientId.value() == data->GetValue()) {
                    pClientChoiceCtrl->SetSelection(i);
                    break;
                }
            }
        }
    }
}

void ProjectDialog::OnNameChange(wxCommandEvent& event)
{
    auto name = pNameTextCtrl->GetValue().ToStdString();
    pDisplayNameCtrl->ChangeValue(name);
}

void ProjectDialog::OnEmployerChoiceSelection(wxCommandEvent& event)
{
    pClientChoiceCtrl->Clear();
    pClientChoiceCtrl->Append("Select client", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);

    int employerIndex = event.GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));

    if (employerIdData->GetValue() < 1) {
        pClientChoiceCtrl->Disable();

        return;
    }

    std::int64_t employerId = employerIdData->GetValue();

    FillClientChoiceControl(employerId);
}

void ProjectDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    pOkButton->Disable();

    TransferDataFromControls();

    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";

    if (pIsDefaultCheckBoxCtrl->IsChecked()) {
        ret = projectPersistence.UnsetDefault();
    }

    if (!bIsEdit) {
        std::int64_t projectId = projectPersistence.Create(mProjectModel);
        ret = projectId > 0 ? 0 : -1;

        ret == -1 ? message = "Failed to create project" : message = "Successfully created project";
    }
    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = projectPersistence.Update(mProjectModel);

        ret == -1 ? message = "Failed to update project" : message = "Successfully updated project";
    }
    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = projectPersistence.Delete(mProjectId);

        ret == -1 ? message = "Failed to delete project" : message = "Successfully deleted project";
    }

    if (ret == -1) {
        QueueErrorNotificationEvent(message);

        pOkButton->Enable();
    } else {
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then
        // we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        EndModal(wxID_OK);
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
        pIsDefaultCheckBoxCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pEmployerChoiceCtrl->Enable();

        if (mProjectModel.ClientId.has_value()) {
            pClientChoiceCtrl->Enable();
        }

    } else {
        pNameTextCtrl->Disable();
        pDisplayNameCtrl->Disable();
        pIsDefaultCheckBoxCtrl->Disable();
        pDescriptionTextCtrl->Disable();
        pEmployerChoiceCtrl->Disable();
        pClientChoiceCtrl->Disable();
    }
}

bool ProjectDialog::Validate()
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
        auto valMsg = "Display name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDisplayNameCtrl);
        return false;
    }

    if (displayName.length() < MIN_CHARACTER_COUNT ||
        displayName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg =
            fmt::format("Display name must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDisplayNameCtrl);
        return false;
    }

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    if (!description.empty() && (description.length() < MIN_CHARACTER_COUNT ||
                                    description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS)) {
        auto valMsg =
            fmt::format("Description must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_DESCRIPTIONS);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDescriptionTextCtrl);
        return false;
    }

    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        auto valMsg = "An employer selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pEmployerChoiceCtrl);
        return false;
    }

    return true;
}

void ProjectDialog::TransferDataFromControls()
{
    mProjectModel.ProjectId = mProjectId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mProjectModel.Name = Utils::TrimWhitespace(name);

    auto displayName = pDisplayNameCtrl->GetValue().ToStdString();
    mProjectModel.DisplayName = Utils::TrimWhitespace(displayName);

    mProjectModel.IsDefault = pIsDefaultCheckBoxCtrl->GetValue();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mProjectModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);

    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));
    mProjectModel.EmployerId = employerIdData->GetValue();

    if (pClientChoiceCtrl->IsEnabled()) {
        int clientIndex = pClientChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* clientIdData = reinterpret_cast<ClientData<std::int64_t>*>(
            pClientChoiceCtrl->GetClientObject(clientIndex));

        if (clientIdData->GetValue() > 0) {
            auto clientId = clientIdData->GetValue();
            mProjectModel.ClientId = std::make_optional(clientId);
        } else {
            mProjectModel.ClientId = std::nullopt;
        }
    }
}
void ProjectDialog::FillClientChoiceControl(const std::int64_t employerId)
{
    std::vector<Model::ClientModel> clients;
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);
    std::string defaultSearchTerm = "";

    int rc = ClientsPersistence.FilterByEmployerId(employerId, clients);
    if (rc == -1) {
        std::string message = "Failed to get clients";
        QueueErrorNotificationEvent(message);
    } else {
        if (!clients.empty()) {
            pClientChoiceCtrl->Enable();

            for (const auto& client : clients) {
                pClientChoiceCtrl->Append(
                    client.Name, new ClientData<std::int64_t>(client.ClientId));
            }
        }
    }
}

void ProjectDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
    // have wxFrame
    wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
