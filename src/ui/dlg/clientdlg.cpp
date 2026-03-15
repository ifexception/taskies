// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include <fmt/format.h>

#include <wx/msgdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../events.h"
#include "../common/clientdata.h"
#include "../common/notificationclientdata.h"

#include "../../common/constants.h"
#include "../../common/common.h"
#include "../../common/validator.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/clientspersistence.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
ClientDialog::ClientDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t clientId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Client" : "New Client",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mClientId(clientId)
    , mClientModel()
{
    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void ClientDialog::Create()
{
    CreateControls();
    FillControls();
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

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Client name");
    pNameTextCtrl->SetToolTip("Enter a name for a client");

    pNameTextCtrl->SetValidator(NameValidator());

    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        clientNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(
        pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Client Description control */
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
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a client");
    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Employer choice control */
    auto employerLabel = new wxStaticText(this, wxID_ANY, "Employer");

    pEmployerChoiceCtrl = new wxChoice(this, tksIDC_EMPLOYERCHOICECTRL);
    pEmployerChoiceCtrl->SetToolTip("Select an employer to associate this client with");

    sizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    sizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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

void ClientDialog::FillControls()
{
    pEmployerChoiceCtrl->Append("Select an employer", new ClientData<std::int64_t>(-1));
    pEmployerChoiceCtrl->SetSelection(0);

    std::string defaultSearchTerm = "";

    std::vector<Model::EmployerModel> employers;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.Filter(defaultSearchTerm, employers);
    if (rc == -1) {
        std::string message = "A database error occured when trying to get employers";
        wxMessageDialog dialog(this,
            message,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(
            "Please try again or click \"OK\" to open your browser to log an issue");

        int ret = dialog.ShowModal();
        if (ret == wxID_OK) {
            wxLaunchDefaultBrowser("https://github.com/ifexception/taskies/issues/new?title=BUG");
        }
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(
                employer.Name, new ClientData<std::int64_t>(employer.EmployerId));

            if (employer.IsDefault) {
                pEmployerChoiceCtrl->SetStringSelection(employer.Name);
            }
        }
    }
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
        pIsActiveCheckBoxCtrl->Bind(
            wxEVT_CHECKBOX,
            &ClientDialog::OnIsActiveCheck,
            this
        );
    }
}
// clang-format on

void ClientDialog::DataToControls()
{
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    int rc = ClientsPersistence.GetById(mClientId, mClientModel);

    if (rc == -1) {
        std::string message = "A database error occured when fetching the client";
        wxMessageDialog dialog(this,
            message,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(
            "Please try again or click \"OK\" to open your browser to log an issue");

        int ret = dialog.ShowModal();
        if (ret == wxID_OK) {
            wxLaunchDefaultBrowser("https://github.com/ifexception/taskies/issues/new?title=BUG");
        }

        EndModal(wxID_OK);
    } else {
        pNameTextCtrl->ChangeValue(mClientModel.Name);

        if (mClientModel.Description.has_value()) {
            pDescriptionTextCtrl->ChangeValue(mClientModel.Description.value());
        }

        for (unsigned int i = 0; i < pEmployerChoiceCtrl->GetCount(); i++) {
            ClientData<std::int64_t>* data = reinterpret_cast<ClientData<std::int64_t>*>(
                pEmployerChoiceCtrl->GetClientObject(i));
            if (mClientModel.EmployerId == data->GetValue()) {
                pEmployerChoiceCtrl->SetSelection(i);
                break;
            }
        }

        pIsActiveCheckBoxCtrl->SetValue(mClientModel.IsActive);

        pIsActiveCheckBoxCtrl->Enable();
    }
}

void ClientDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    TransferDataFromControls();

    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";

    if (!bIsEdit) {
        std::int64_t clientId = ClientsPersistence.Create(mClientModel);
        ret = clientId > 0 ? 1 : -1;

        if (ret == -1) {
            message = "A database error occured when trying to create a client";
            QueueErrorNotificationEvent(message);
        }
    }
    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = ClientsPersistence.Update(mClientModel);

        if (ret == -1) {
            message = "A database error occured when trying to update the client";
            QueueErrorNotificationEvent(message);
        }
    }

    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
        ret = ClientsPersistence.Delete(mClientId);

        if (ret == -1) {
            message = "A database error occured when trying to delete the client";
            QueueErrorNotificationEvent(message);
        }
    }

    EndModal(wxID_OK);
}

void ClientDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void ClientDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pEmployerChoiceCtrl->Enable();
    } else {
        pNameTextCtrl->Disable();
        pDescriptionTextCtrl->Disable();
        pEmployerChoiceCtrl->Disable();
    }
}

bool ClientDialog::Validate()
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

void ClientDialog::TransferDataFromControls()
{
    mClientModel.ClientId = mClientId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mClientModel.Name = Utils::TrimWhitespace(name);

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mClientModel.Description = description.empty() ? std::nullopt : std::make_optional(description);

    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));
    mClientModel.EmployerId = employerIdData->GetValue();
}

void ClientDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ERRORNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(message);
    addNotificationEvent->SetClientObject(clientData);

    // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
    // have wxFrame
    wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
