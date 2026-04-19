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

#include <wx/richmsgdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../common/results/sqliteresult.h"
#include "../../common/messages/persistencemessages.h"

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
    , pNameTextCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mEmployerId(employerId)
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

    auto sqliteResult = employerPersistence.GetById(mEmployerId, employerModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::GetByIdEmployerMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
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

    Model::EmployerModel employerModel = TransferDataFromControls();

    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    if (pIsDefaultCheckBoxCtrl->IsChecked()) {
        auto result = employerPersistence.UnsetDefault();

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when unsetting a default "
                           "employer, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::UnsetDefaultEmployerMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }

    if (!bIsEdit) {
        std::int64_t employerId = -1;
        auto result = employerPersistence.Create(employerId, employerModel);

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when creating an "
                           "employer, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::CreateEmployerMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }
    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
        auto result = employerPersistence.Update(employerModel);

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when updating an "
                           "employer, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::UpdateEmployerMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }
    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
        auto result = employerPersistence.Delete(mEmployerId);

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when deleting an "
                           "employer, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::DeleteEmployerMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
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
        auto result = employerPersistence.SelectDefault(model);

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when querying for default "
                           "employers, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::SelectDefaultEmployerMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
        } else {
            if (!model.IsDefault) {
                std::string validationMessage = "An employer is required to be default";
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
} // namespace tks::UI::dlg
