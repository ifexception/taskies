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

#include "employerdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include <fmt/format.h>

#include "../events.h"
#include "../common/notificationclientdata.h"

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
    , pNameTextCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
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

    detailsGridSizer->Add(
        employerNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
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

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void EmployerDialog::FillControls()
{
    pIsDefaultCheckBoxCtrl->SetValue(false);
}

// clang-format off
void EmployerDialog::ConfigureEventBindings()
{
    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &EmployerDialog::OnIsActiveCheck,
        this
    );

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
    Model::EmployerModel employerModel;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.GetById(mEmployerId, employerModel);
    if (rc == -1) {
        std::string message = "Failed to get employer";
        QueueErrorNotificationEvent(message);
    } else {
        pNameTextCtrl->SetValue(employerModel.Name);
        pIsDefaultCheckBoxCtrl->SetValue(employerModel.IsDefault);

        if (employerModel.Description.has_value()) {
            pDescriptionTextCtrl->SetValue(employerModel.Description.value());
        }

        pIsActiveCheckBoxCtrl->SetValue(employerModel.IsActive);

        pIsActiveCheckBoxCtrl->Enable();
    }
}

void EmployerDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    pOkButton->Disable();

    Model::EmployerModel employerModel = TransferDataFromControls();

    int ret = 0;
    std::string message = "";

    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);
    bool canContinue = true;

    if (pIsDefaultCheckBoxCtrl->IsChecked()) {
        ret = employerPersistence.UnsetDefault();

        if (ret == -1) {
            canContinue = false;
            message = "A database error occured while trying unset the default employer";
            QueueErrorNotificationEvent(message);
        }
    }

    if (!bIsEdit && canContinue) {
        std::int64_t employerId = employerPersistence.Create(employerModel);
        ret = employerId > 0 ? 1 : -1;

        if (ret == -1) {
            message = "A database error occured when trying to create an employer";
            QueueErrorNotificationEvent(message);
        }
    }
    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked() && canContinue) {
        ret = employerPersistence.Update(employerModel);

        if (ret == -1) {
            message = "A database error occured when trying update the employer";
            QueueErrorNotificationEvent(message);
        }
    }
    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked() && canContinue) {
        ret = employerPersistence.Delete(mEmployerId);

        if (ret == -1) {
            message = "A database error occured when trying to delete the employer";
            QueueErrorNotificationEvent(message);
        }
    }

    EndModal(wxID_OK);
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
        int rc = employerPersistence.SelectDefault(model);

        if (rc == -1) {
            std::string message = "Failed to get default employer";
            QueueErrorNotificationEvent(message);
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

Model::EmployerModel EmployerDialog::TransferDataFromControls()
{
    Model::EmployerModel employerModel;
    employerModel.EmployerId = mEmployerId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    employerModel.Name = Utils::TrimWhitespace(name);

    employerModel.IsDefault = pIsDefaultCheckBoxCtrl->GetValue();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    employerModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);

    return employerModel;
}

void EmployerDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* errorNotificationEvent = new wxCommandEvent(tksEVT_ERRORNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(message);
    errorNotificationEvent->SetClientObject(clientData);

    // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then
    // we have wxFrame
    wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, errorNotificationEvent);
}
} // namespace tks::UI::dlg
