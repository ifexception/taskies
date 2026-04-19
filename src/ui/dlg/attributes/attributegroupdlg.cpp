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

#include "attributegroupdlg.h"

#include <optional>

#include <wx/richmsgdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/clientdata.h"

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/validator.h"

#include "../../../common/messages/persistencemessages.h"

#include "../../../persistence/attributegroupspersistence.h"

#include "../../../utils/utils.h"

namespace tks::UI::dlg
{
AttributeGroupDialog::AttributeGroupDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t attributeGroupId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Attribute Group" : "New Attribute Group",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pIsStaticCheckBoxCtrl(nullptr)
    , pIsDefaultCheckBoxCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mDatabaseFilePath(databaseFilePath)
    , mAttributeGroupId(attributeGroupId)
    , bIsEdit(isEdit)
    , bIsInUse(false)
    , bIsInUseStatic(false)
    , mAttributeGroupModel()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void AttributeGroupDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();

    if (bIsEdit) {
        DataToControls();
    }
}

void AttributeGroupDialog::CreateControls()
{
    /* Main dialog sizer for controls */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    /* Details static box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    mainSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Attribute group name label control */
    auto attributeGroupNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    /* Attribute group name text control*/
    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Attribute group name");
    pNameTextCtrl->SetToolTip("Set a name for the attribute group");

    /* Attribute group is static check box control */
    pIsStaticCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_ISSTATICCHECKBOXCTRL, "Is Static");
    pIsStaticCheckBoxCtrl->SetToolTip(
        "Enabling this option will use linked static values for attributes");

    /* Attribute group is default check box control */
    pIsDefaultCheckBoxCtrl = new wxCheckBox(detailsBox, tksIDC_ISDEFAULTCHECKBOXCTRL, "Is Default");
    pIsDefaultCheckBoxCtrl->SetToolTip(
        "Enabling this option will auto-select it when creating a task");

    /* Grid sizer for attribute group name controls */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        attributeGroupNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(
        pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsStaticCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Attribute group description static box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Decription");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    mainSizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Attribute group description text control */
    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTIONTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Attribute group description");
    pDescriptionTextCtrl->SetToolTip("Set a description of the attribute group");

    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Is Active static box control */
    auto isActiveStaticBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto isActiveStaticBoxSizer = new wxStaticBoxSizer(isActiveStaticBox, wxHORIZONTAL);
    mainSizer->Add(isActiveStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl =
        new wxCheckBox(isActiveStaticBox, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    isActiveStaticBoxSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    mainSizer->Add(line, wxSizerFlags().Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(mainSizer);
}

// clang-format off
void AttributeGroupDialog::ConfigureEventBindings()
{
    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &AttributeGroupDialog::OnIsActiveCheck,
        this
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &AttributeGroupDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &AttributeGroupDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void AttributeGroupDialog::DataToControls()
{
    Model::AttributeGroupModel attributeGroupModel;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = attributeGroupsPersistence.GetById(mAttributeGroupId, attributeGroupModel);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::CreateAttributeGroupMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    pNameTextCtrl->SetValue(attributeGroupModel.Name);

    if (attributeGroupModel.Description.has_value()) {
        pDescriptionTextCtrl->SetValue(attributeGroupModel.Description.value());
    }

    pIsStaticCheckBoxCtrl->SetValue(attributeGroupModel.IsStatic);
    pIsDefaultCheckBoxCtrl->SetValue(attributeGroupModel.IsDefault);

    pIsActiveCheckBoxCtrl->SetValue(attributeGroupModel.IsActive);

    pIsActiveCheckBoxCtrl->Enable();

    sqliteResult =
        attributeGroupsPersistence.CheckAttributeGroupAttributesUsage(mAttributeGroupId, bIsInUse);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::CheckUsageAttributeGroupMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    if (bIsInUse) {
        pIsStaticCheckBoxCtrl->Disable();
    }

    sqliteResult = attributeGroupsPersistence.CheckAttributeGroupStaticAttributesUsage(
        mAttributeGroupId, bIsInUseStatic);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::CheckUsageAttributeGroupMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    if (bIsInUseStatic) {
        pIsStaticCheckBoxCtrl->Disable();
    }

    pOkButton->SetFocus();
}

void AttributeGroupDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        if (!bIsInUseStatic || !bIsInUse) {
            pIsStaticCheckBoxCtrl->Enable();
        }
        pIsDefaultCheckBoxCtrl->Enable();
    } else {
        pNameTextCtrl->Disable();
        pIsStaticCheckBoxCtrl->Disable();
        pIsDefaultCheckBoxCtrl->Disable();
        pDescriptionTextCtrl->Disable();
    }
}

void AttributeGroupDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    TransferDataFromControls();

    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    if (pIsDefaultCheckBoxCtrl->GetValue()) {
        auto result = attributeGroupsPersistence.UnsetDefault();
        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when unsetting a default "
                           "attribute group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::UnsetDefaultAttributeGroupMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }

    if (!bIsEdit) {
        std::int64_t attributeGroupId = 1;
        auto result = attributeGroupsPersistence.Create(attributeGroupId, mAttributeGroupModel);
        // if (attributeGroupId == -19) { // SQLITE_CONSTRAINT * -1
        //     wxMessageBox("Attribute group with specified name already exists",
        //         Common::GetProgramName(),
        //         wxOK_DEFAULT | wxICON_WARNING);
        //     return;
        // }
        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when creating an attribute "
                           "group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::CreateAttributeGroupMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }
    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
        auto result = attributeGroupsPersistence.Update(mAttributeGroupModel);
        // if (ret == -19) { // SQLITE_CONSTRAINT * -1
        //     wxMessageBox("Attribute group with specified name already exists",
        //         Common::GetProgramName(),
        //         wxOK_DEFAULT | wxICON_WARNING);
        //     return;
        // }

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when updating an attribute "
                           "group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::UpdateAttributeGroupMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }
    }
    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
        bool isAttributeGroupAttributeValueUsed = false;
        auto result = attributeGroupsPersistence.CheckAttributeGroupAttributeValuesUsage(
            mAttributeGroupId, isAttributeGroupAttributeValueUsed);

        if (!result.Success) {
            pLogger->error(
                "A database error occurred with code \"{0}\" when checking usage of an attribute "
                "group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::CheckUsageAttributeGroupMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }

        bool isAttributeGroupAttributeUsed = false;
        result = attributeGroupsPersistence.CheckAttributeGroupAttributesUsage(
            mAttributeGroupId, isAttributeGroupAttributeUsed);

        if (!result.Success) {
            pLogger->error(
                "A database error occurred with code \"{0}\" when checking usage of an attribute "
                "group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::CheckUsageAttributeGroupMessage,
                Common::GetProgramName(),
                wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
            dialog.SetExtendedMessage(result.FriendlyErrorMessage);
            dialog.ShowDetailedText(result.GetReturnCodeAndMessage());

            dialog.ShowModal();
            return;
        }

        if (isAttributeGroupAttributeValueUsed || isAttributeGroupAttributeUsed) {
            wxMessageBox("Cannot delete this attribute group as it is in use",
                Common::GetProgramName(),
                wxOK_DEFAULT | wxICON_WARNING);
            return;
        }

        result = attributeGroupsPersistence.Delete(mAttributeGroupId);

        if (!result.Success) {
            pLogger->error("A database error occurred with code \"{0}\" when deleting an attribute "
                           "group, see earlier logs for details",
                result.ReturnCode);

            wxRichMessageDialog dialog(this,
                Messages::DeleteAttributeGroupMessage,
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

void AttributeGroupDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool AttributeGroupDialog::Validate()
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
    return true;
}

void AttributeGroupDialog::TransferDataFromControls()
{
    mAttributeGroupModel.AttributeGroupId = mAttributeGroupId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mAttributeGroupModel.Name = Utils::TrimWhitespace(name);

    mAttributeGroupModel.IsStatic = pIsStaticCheckBoxCtrl->GetValue();
    mAttributeGroupModel.IsDefault = pIsDefaultCheckBoxCtrl->GetValue();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mAttributeGroupModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);
}
} // namespace tks::UI::dlg
