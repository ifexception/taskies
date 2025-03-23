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

#include "employerdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include <fmt/format.h>

#include "../events.h"
#include "../notificationclientdata.h"

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../persistence/employerspersistence.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
EmployerDialog::EmployerDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t employerId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Employer" : "New Employer",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mEmployerId(employerId)
    , mEmployerModel()
    , pNameTextCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pDateCreatedReadonlyTextCtrl(nullptr)
    , pDateModifiedReadonlyTextCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
{
    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void EmployerDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();

    if (bIsEdit) {
        DataToControls();
    }
}

void EmployerDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Details */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    sizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Employer Name Control */
    auto employerNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAME);
    pNameTextCtrl->SetHint("Employer name");
    pNameTextCtrl->SetToolTip("Enter a name for an employer");

    pNameTextCtrl->SetValidator(NameValidator());

    pIsDefaultCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_ISDEFAULT, "Is Default");
    pIsDefaultCheckBoxCtrl->SetToolTip("Enabling this option will auto-select it where applicable");

    detailsGridSizer->Add(employerNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Description controls */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    sizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTION,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for an employer");
    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Begin edit metadata controls */

    /* Horizontal Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line1, wxSizerFlags().Border(wxTOP | wxBOTTOM, FromDIP(4)).Expand());

    /* Date Created text control */
    auto dateCreatedLabel = new wxStaticText(this, wxID_ANY, "Date Created");

    pDateCreatedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateCreatedReadonlyTextCtrl->Disable();

    /* Date Modified text control */
    auto dateModifiedLabel = new wxStaticText(this, wxID_ANY, "Date Modified");

    pDateModifiedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateModifiedReadonlyTextCtrl->Disable();

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Toggle the deleted state of an employer");
    pIsActiveCheckBoxCtrl->Disable();

    /* Metadata flex grid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    sizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand());
    metadataFlexGridSizer->AddGrowableCol(1, 1);

    metadataFlexGridSizer->Add(
        dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateCreatedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(
        dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateModifiedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(0, 0);
    metadataFlexGridSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* End of edit metadata controls */

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

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void EmployerDialog::FillControls()
{
    pIsDefaultCheckBoxCtrl->SetValue(true);
}

// clang-format off
void EmployerDialog::ConfigureEventBindings()
{
    if (bIsEdit) {
        pIsActiveCheckBoxCtrl->Bind(
            wxEVT_CHECKBOX,
            &EmployerDialog::OnIsActiveCheck,
            this
        );
    }

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
}
// clang-format on

void EmployerDialog::DataToControls()
{
    pOkButton->Disable();

    Model::EmployerModel employer;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.GetById(mEmployerId, employer);
    if (rc == -1) {
        std::string message = "Failed to get employer";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        pNameTextCtrl->SetValue(employer.Name);
        pIsDefaultCheckBoxCtrl->SetValue(employer.IsDefault);
        pDescriptionTextCtrl->SetValue(
            employer.Description.has_value() ? employer.Description.value() : "");
        pDateCreatedReadonlyTextCtrl->SetValue(employer.GetDateCreatedString());
        pDateModifiedReadonlyTextCtrl->SetValue(employer.GetDateModifiedString());
        pIsActiveCheckBoxCtrl->SetValue(employer.IsActive);

        pOkButton->Enable();
    }
}

void EmployerDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    pOkButton->Disable();

    TransferDataFromControls();

    int ret = 0;
    std::string message = "";

    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    if (pIsDefaultCheckBoxCtrl->IsChecked()) {
        ret = employerPersistence.UnsetDefault();

        if (ret == -1) {
            message = "Failed to unset default employer";
        }
    }

    if (ret != -1) {
        if (!bIsEdit) {
            std::int64_t employerId = employerPersistence.Create(mEmployerModel);
            ret = employerId > 0 ? 1 : -1;

            ret == -1 ? message = "Failed to create employer"
                      : message = "Successfully created employer";
        }
        if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
            ret = employerPersistence.Update(mEmployerModel);

            ret == -1 ? message = "Failed to update employer"
                      : message = "Successfully updated employer";
        }
        if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
            ret = employerPersistence.Delete(mEmployerId);

            ret == -1 ? message = "Failed to delete employer"
                      : message = "Successfully deleted employer";
        }
    }

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    if (ret == -1) {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        pOkButton->Enable();
    } else {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        EndModal(wxID_OK);
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
        pIsDefaultCheckBoxCtrl->Enable();
        pDescriptionTextCtrl->Enable();
    } else {
        pNameTextCtrl->Disable();
        pIsDefaultCheckBoxCtrl->Disable();
        pDescriptionTextCtrl->Disable();
    }
}

bool EmployerDialog::Validate()
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
        auto validationMessage =
            fmt::format("Description must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_DESCRIPTIONS);
        wxRichToolTip toolTip("Validation", validationMessage);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pDescriptionTextCtrl);
        return false;
    }

    if (!pIsDefaultCheckBoxCtrl->IsChecked()) {
        Model::EmployerModel model;
        Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);
        int rc = employerPersistence.TrySelectDefault(model);

        if (rc == -1) {
            std::string message = "Failed to get default employer";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData =
                new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then
            // we have wxFrame
            wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
        } else {
            if (!model.IsDefault) {
                std::string validationMessage = "Required default employer not found";
                wxRichToolTip toolTip("Validation", validationMessage);
                toolTip.SetIcon(wxICON_WARNING);
                toolTip.ShowFor(pIsDefaultCheckBoxCtrl);
                return false;
            }
        }
    }

    return true;
}

void EmployerDialog::TransferDataFromControls()
{
    mEmployerModel.EmployerId = mEmployerId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mEmployerModel.Name = Utils::TrimWhitespace(name);

    mEmployerModel.IsDefault = pIsDefaultCheckBoxCtrl->GetValue();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mEmployerModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);
}
} // namespace tks::UI::dlg
