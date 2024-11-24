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

#include "projectdlg.h"

#include <vector>

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../dao/employerdao.h"
#include "../../dao/clientdao.h"
#include "../../dao/projectdao.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"

#include "../../utils/utils.h"

#include "../clientdata.h"
#include "../events.h"
#include "../notificationclientdata.h"

namespace tks::UI::dlg
{
ProjectDialog::ProjectDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
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
    , pEnv(env)
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

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAME);
    pNameTextCtrl->SetHint("Project name");
    pNameTextCtrl->SetToolTip("Enter a name for a project");
    pNameTextCtrl->SetValidator(NameValidator());

    /* Display Name Ctrl */
    auto displayNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Display Name");

    pDisplayNameCtrl = new wxTextCtrl(detailsBox, tksIDC_DISPLAYNAME);
    pDisplayNameCtrl->SetHint("Display name");
    pDisplayNameCtrl->SetToolTip("Enter a nickname, abbreviation or common name for a project (if applicable)");
    pDisplayNameCtrl->SetValidator(NameValidator());

    /* Is Default Checkbox Ctrl */
    pIsDefaultCtrl = new wxCheckBox(detailsBox, tksIDC_ISDEFAULT, "Is Default");
    pIsDefaultCtrl->SetToolTip("Enabling this option for a project will auto-select it on a task entry");

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

    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTION,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a project");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Files */

    /* Choices */
    auto choiceBox = new wxStaticBox(this, wxID_ANY, "Selection");
    auto choiceBoxSizer = new wxStaticBoxSizer(choiceBox, wxVERTICAL);
    sizer->Add(choiceBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Employer choice control */
    auto employerLabel = new wxStaticText(choiceBox, wxID_ANY, "Employer");

    pEmployerChoiceCtrl = new wxChoice(choiceBox, tksIDC_EMPLOYERCHOICE);
    pEmployerChoiceCtrl->SetToolTip("Select an employer to associate this project with");

    /* Client choice control */
    auto clientLabel = new wxStaticText(choiceBox, wxID_ANY, "Client");

    pClientChoiceCtrl = new wxChoice(choiceBox, tksIDC_CLIENTCHOICE);
    pClientChoiceCtrl->SetToolTip("Select an (optional) client to associate this project with");

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

        pIsActiveCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVE, "Is Active");
        pIsActiveCtrl->SetToolTip("Indicates if this project is being used");
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
    pEmployerChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(0));
    pEmployerChoiceCtrl->SetSelection(0);

    std::vector<Model::EmployerModel> employers;
    std::string defaultSearhTerm = "";
    DAO::EmployerDao employerDao(pLogger, mDatabaseFilePath);

    int rc = employerDao.Filter(defaultSearhTerm, employers);
    if (rc != 0) {
        std::string message = "Failed to get employers";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(employer.Name, new ClientData<std::int64_t>(employer.EmployerId));
        }
    }

    pOkButton->Enable();

    pClientChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
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
    DAO::ProjectDao projectDao(pLogger, mDatabaseFilePath);
    bool isSuccess = false;

    int rc = projectDao.GetById(mProjectId, project);
    if (rc != 0) {
        std::string message = "Failed to get project";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
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
    DAO::EmployerDao employerDao(pLogger, mDatabaseFilePath);

    rc = employerDao.GetById(project.EmployerId, employer);
    if (rc == -1) {
        std::string message = "Failed to get employer";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        isSuccess = false;
        return;
    } else {
        pEmployerChoiceCtrl->SetStringSelection(employer.Name);
        isSuccess = true;
    }

    std::vector<Model::ClientModel> clients;
    DAO::ClientDao clientDao(pLogger, mDatabaseFilePath);
    std::string defaultSearchTerm = "";

    rc = clientDao.FilterByEmployerId(project.EmployerId, clients);
    if (rc == -1) {
        std::string message = "Failed to get clients";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        isSuccess = false;
        return;
    } else {
        if (!clients.empty()) {
            for (const auto& client : clients) {
                pClientChoiceCtrl->Append(client.Name, new ClientData<std::int64_t>(client.ClientId));
            }

            if (project.ClientId.has_value()) {
                Model::ClientModel client;
                rc = clientDao.GetById(project.ClientId.value(), client);
                if (rc == -1) {
                    std::string message = "Failed to get client";
                    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
                    NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
                    addNotificationEvent->SetClientObject(clientData);

                    // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have
                    // wxFrame
                    wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

                    isSuccess = false;
                } else {
                    pClientChoiceCtrl->SetStringSelection(client.Name);
                }
            }

            pClientChoiceCtrl->Enable();
        }
        isSuccess = true;
    }

    if (isSuccess) {
        pOkButton->Enable();
        pOkButton->SetFocus();
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
    pClientChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);

    if (event.GetSelection() < 1) {
        pClientChoiceCtrl->Disable();
        pOkButton->Enable();

        return;
    }

    int employerIndex = event.GetSelection();
    ClientData<std::int64_t>* employerIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        pClientChoiceCtrl->Disable();
        pOkButton->Enable();

        return;
    }

    auto employerId = employerIdData->GetValue();
    std::vector<Model::ClientModel> clients;
    DAO::ClientDao clientDao(pLogger, mDatabaseFilePath);

    int rc = clientDao.FilterByEmployerId(employerId, clients);

    if (rc != 0) {
        std::string message = "Failed to get clients";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        if (clients.empty()) {
            pClientChoiceCtrl->Disable();
            pOkButton->Enable();

            return;
        }

        for (auto& client : clients) {
            pClientChoiceCtrl->Append(client.Name, new ClientData<std::int64_t>(client.ClientId));
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
        DAO::ProjectDao projectDao(pLogger, mDatabaseFilePath);
        int ret = 0;
        std::string message = "";
        if (pIsDefaultCtrl->IsChecked()) {
            ret = projectDao.UnmarkDefault();
        }

        if (!bIsEdit) {
            std::int64_t projectId = projectDao.Create(mProjectModel);
            ret = projectId > 0 ? 0 : -1;

            ret == -1
                ? message = "Failed to create project"
                : message = "Successfully created project";
        }
        if (bIsEdit && pIsActiveCtrl->IsChecked()) {
            ret = projectDao.Update(mProjectModel);

            ret == -1
                ? message = "Failed to update project"
                : message = "Successfully updated project";
        }
        if (bIsEdit && !pIsActiveCtrl->IsChecked()) {
            ret = projectDao.Delete(mProjectId);

            ret == -1
                ? message = "Failed to delete project"
                : message = "Successfully deleted project";
        }

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        if (ret == -1) {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
            wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

            pOkButton->Enable();
        } else {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
            addNotificationEvent->SetClientObject(clientData);

            // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
            wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

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
        auto valMsg = "Display name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDisplayNameCtrl);
        return false;
    }

    if (displayName.length() < MIN_CHARACTER_COUNT || displayName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Display name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDisplayNameCtrl);
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

    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        auto valMsg = "An employer selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pEmployerChoiceCtrl);
        return false;
    }

    if (pClientChoiceCtrl->IsEnabled()) {
        int clientIndex = pClientChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* clientIdData =
            reinterpret_cast<ClientData<std::int64_t>*>(pClientChoiceCtrl->GetClientObject(clientIndex));

        if (clientIdData->GetValue() > 0) {
            auto clientId = clientIdData->GetValue();
            mProjectModel.ClientId = std::make_optional(clientId);
        } else {
            mProjectModel.ClientId = std::nullopt;
        }
    }

    mProjectModel.Name = Utils::TrimWhitespace(name);
    mProjectModel.DisplayName = Utils::TrimWhitespace(displayName);
    mProjectModel.IsDefault = pIsDefaultCtrl->GetValue();
    mProjectModel.Description = description.empty() ? std::nullopt : std::make_optional(description);
    mProjectModel.EmployerId = employerIdData->GetValue();
    mProjectModel.ProjectId = mProjectId;

    return true;
}
} // namespace tks::UI::dlg
