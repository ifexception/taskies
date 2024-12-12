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

#include "employerdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../persistence/employerpersistence.h"

#include "../../utils/utils.h"
#include "../events.h"
#include "../notificationclientdata.h"

namespace tks::UI::dlg
{
EmployerDialog::EmployerDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
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
    , pEnv(env)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mEmployerId(employerId)
    , mEmployerModel()
    , pNameTextCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
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

    /* Employer Name Control */
    auto employerNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAME);
    pNameTextCtrl->SetHint("Employer name");
    pNameTextCtrl->SetToolTip("Enter a name for an employer");

    pNameTextCtrl->SetValidator(NameValidator());

    pIsDefaultCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_ISDEFAULT, "Is Default");
    pIsDefaultCheckBoxCtrl->SetToolTip("Enabling this option will auto-select it where applicable");

    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(employerNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Employer Description */
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
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for an employer");
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
        metadataFlexGridSizer->Add(dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

        pDateModifiedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
        pDateModifiedTextCtrl->Disable();
        metadataFlexGridSizer->Add(pDateModifiedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

        /* Is Active checkbox control */
        metadataFlexGridSizer->Add(0, 0);

        pIsActiveCheckBoxCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVE, "Is Active");
        pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this employer is being used");
        metadataFlexGridSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)));
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
    Persistence::EmployerPersistence employerPersistence(pLogger, mDatabaseFilePath);

    int rc = employerPersistence.GetById(mEmployerId, employer);
    if (rc == -1) {
        std::string message = "Failed to get employer";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        pNameTextCtrl->SetValue(employer.Name);
        pIsDefaultCheckBoxCtrl->SetValue(employer.IsDefault);
        pDescriptionTextCtrl->SetValue(employer.Description.has_value() ? employer.Description.value() : "");
        pDateCreatedTextCtrl->SetValue(employer.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(employer.GetDateModifiedString());
        pIsActiveCheckBoxCtrl->SetValue(employer.IsActive);

        pOkButton->Enable();
        pOkButton->SetFocus();
    }
}

void EmployerDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    if (!TransferDataAndValidate()) {
        return;
    }

    int ret = 0;
    std::string message = "";

    Persistence::EmployerPersistence employerPersistence(pLogger, mDatabaseFilePath);

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

            ret == -1 ? message = "Failed to create employer" : message = "Successfully created employer";
        }
        if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
            ret = employerPersistence.Update(mEmployerModel);

            ret == -1 ? message = "Failed to update employer" : message = "Successfully updated employer";
        }
        if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
            ret = employerPersistence.Delete(mEmployerId);

            ret == -1 ? message = "Failed to delete employer" : message = "Successfully deleted employer";
        }
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

    mEmployerModel.EmployerId = mEmployerId;
    mEmployerModel.Name = Utils::TrimWhitespace(name);
    mEmployerModel.IsDefault = pIsDefaultCheckBoxCtrl->GetValue();
    mEmployerModel.Description = description.empty() ? std::nullopt : std::make_optional(description);

    return true;
}
} // namespace tks::UI::dlg
