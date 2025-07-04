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

#include "attributedlg.h"

#include <vector>

#include <fmt/format.h>

#include <wx/statline.h>
#include <wx/richtooltip.h>

#include "../../events.h"
#include "../../common/clientdata.h"
#include "../../common/notificationclientdata.h"

#include "../../../common/common.h"
#include "../../../common/constants.h"

#include "../../../persistence/attributespersistence.h"
#include "../../../persistence/attributegroupspersistence.h"
#include "../../../persistence/attributetypespersistence.h"

#include "../../../models/attributegroupmodel.h"
#include "../../../models/attributetypemodel.h"

#include "../../../utils/utils.h"

namespace tks::UI::dlg
{
AttributeDialog::AttributeDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t attributeId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Attribute" : "New Attribute",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mAttributeId(attributeId)
    , pNameTextCtrl(nullptr)
    , pIsRequiredCheckBoxCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pAttributeGroupChoiceCtrl(nullptr)
    , pAttributeTypeChoiceCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pAddAnotherCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mAttributeModel()
    , bAddAnotherAttribute(false)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void AttributeDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();

    if (bIsEdit) {
        DataToControls();

        pAddAnotherCheckBoxCtrl->Disable();
    }
}

void AttributeDialog::CreateControls()
{
    /* Main dialog sizer for controls */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    /* Details static box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    mainSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Attribute name label control */
    auto attributeNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    /* Attribute name text control */
    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Attribute name");
    pNameTextCtrl->SetToolTip("Set a name for the attribute");

    /* Is required check box control */
    pIsRequiredCheckBoxCtrl =
        new wxCheckBox(detailsBox, tksIDC_ISREQUIREDCHECKBOXCTRL, "Is Required");
    pIsRequiredCheckBoxCtrl->SetToolTip("The attribute will be required");

    /* Grid sizer for attribute name controls */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        attributeNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(
        pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pIsRequiredCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Attribute description static box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Decription");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    mainSizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Attribute description text control */
    pDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_DESCRIPTIONTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Attribute description");
    pDescriptionTextCtrl->SetToolTip("Set a description of the attribute");

    descriptionBoxSizer->Add(
        pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Attribute selection controls */
    /* Attribute group label */
    auto attributeGroupLabel = new wxStaticText(this, wxID_ANY, "Attribute Group");

    /* Attribute group choice */
    pAttributeGroupChoiceCtrl = new wxChoice(this, tksIDC_ATTRIBUTEGROUPCHOICECTRL);
    pAttributeGroupChoiceCtrl->SetToolTip("Select an attribute group to associate with");

    /* Field (Attribute) type label */
    auto fieldTypeLabel = new wxStaticText(this, wxID_ANY, "Field Type");

    /* Attribute type choice */
    pAttributeTypeChoiceCtrl = new wxChoice(this, tksIDC_ATTRIBUTETYPECHOICECTRL);
    pAttributeTypeChoiceCtrl->SetToolTip("Select a type for the attribute");

    mainSizer->Add(attributeGroupLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    mainSizer->Add(pAttributeGroupChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());
    mainSizer->Add(fieldTypeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    mainSizer->Add(pAttributeTypeChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pIsStaticCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISSTATICCHECKBOXCTRL, "Is Static");
    pIsStaticCheckBoxCtrl->SetToolTip(
        "Set from the attribute group and values captured will be static");
    pIsStaticCheckBoxCtrl->Disable();
    mainSizer->Add(pIsStaticCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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
    auto line2 = new wxStaticLine(this, wxID_ANY);
    mainSizer->Add(line2, wxSizerFlags().Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pAddAnotherCheckBoxCtrl = new wxCheckBox(this, tksIDC_ADDANOTHERCHECKBOXCTRL, "Add Another");

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(
        pAddAnotherCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    buttonsSizer->AddStretchSpacer();
    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(mainSizer);
}

// clang-format off
void AttributeDialog::ConfigureEventBindings()
{
    pAttributeGroupChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &AttributeDialog::OnAttributeGroupSelection,
        this
    );

    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &AttributeDialog::OnIsActiveCheck,
        this
    );

    pAddAnotherCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &AttributeDialog::OnAddAnotherCheck,
        this
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &AttributeDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &AttributeDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void AttributeDialog::FillControls()
{
    // Add default prompt attribute group
    pAttributeGroupChoiceCtrl->Append(
        "Select an attribute group", new ClientData<std::int64_t>(-1));
    pAttributeGroupChoiceCtrl->SetSelection(0);

    // Add default prompt field type
    pAttributeTypeChoiceCtrl->Append("Select a field type", new ClientData<std::int64_t>(-1));
    pAttributeTypeChoiceCtrl->SetSelection(0);

    // filter attribute groups
    std::vector<Model::AttributeGroupModel> attributeGroups;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupsPersistence.Filter("", attributeGroups);
    if (rc == -1) {
        std::string message = "Failed to get attribute groups";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        for (const auto& attributeGroup : attributeGroups) {
            pAttributeGroupChoiceCtrl->Append(
                attributeGroup.Name, new ClientData<std::int64_t>(attributeGroup.AttributeGroupId));
        }
    }

    // filter attribute types
    std::vector<Model::AttributeTypeModel> attributeTypes;
    Persistence::AttributeTypesPersistence attributeTypesPersistence(pLogger, mDatabaseFilePath);

    rc = attributeTypesPersistence.Filter("", attributeTypes);
    if (rc == -1) {
        std::string message = "Failed to get attribute types";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        for (const auto& attributeType : attributeTypes) {
            pAttributeTypeChoiceCtrl->Append(
                attributeType.Name, new ClientData<std::int64_t>(attributeType.AttributeTypeId));
        }
    }
}

void AttributeDialog::DataToControls()
{
    assert(mAttributeId != -1);

    pOkButton->Disable();

    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    int rc = attributesPersistence.GetById(mAttributeId, mAttributeModel);
    if (rc == -1) {
        std::string message = "Failed to get attribute group";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
    } else {
        pNameTextCtrl->ChangeValue(mAttributeModel.Name);
        pIsRequiredCheckBoxCtrl->SetValue(mAttributeModel.IsRequired);
        if (mAttributeModel.Description.has_value()) {
            pDescriptionTextCtrl->ChangeValue(mAttributeModel.Description.value());
        }

        for (unsigned int i = 0; i < pAttributeGroupChoiceCtrl->GetCount(); i++) {
            ClientData<std::int64_t>* data = reinterpret_cast<ClientData<std::int64_t>*>(
                pAttributeGroupChoiceCtrl->GetClientObject(i));
            if (mAttributeModel.AttributeGroupId == data->GetValue()) {
                pAttributeGroupChoiceCtrl->SetSelection(i);
                break;
            }
        }

        Model::AttributeGroupModel attributeGroupModel;
        Persistence::AttributeGroupsPersistence attributeGroupsPersistence(
            pLogger, mDatabaseFilePath);
        rc = attributeGroupsPersistence.GetById(
            mAttributeModel.AttributeGroupId, attributeGroupModel);
        if (rc == -1) {
            std::string message = "Failed to get attribute group";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData =
                new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then
            // we have wxFrame
            wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
        }

        if (attributeGroupModel.IsStatic) {
            pIsStaticCheckBoxCtrl->SetValue(true);
        }

        for (unsigned int i = 0; i < pAttributeTypeChoiceCtrl->GetCount(); i++) {
            ClientData<std::int64_t>* data = reinterpret_cast<ClientData<std::int64_t>*>(
                pAttributeTypeChoiceCtrl->GetClientObject(i));
            if (mAttributeModel.AttributeTypeId == data->GetValue()) {
                pAttributeTypeChoiceCtrl->SetSelection(i);
                break;
            }
        }

        pIsActiveCheckBoxCtrl->SetValue(mAttributeModel.IsActive);
        pIsActiveCheckBoxCtrl->Enable();

        pOkButton->Enable();
    }
}

void AttributeDialog::OnAttributeGroupSelection(wxCommandEvent& event)
{
    if (event.GetSelection() < 1) {
        pIsStaticCheckBoxCtrl->SetValue(false);
        return;
    }

    ClientData<std::int64_t>* data = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeTypeChoiceCtrl->GetClientObject(event.GetSelection()));

    std::int64_t attributeGroupId = data->GetValue();

    Model::AttributeGroupModel attributeGroupModel;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupsPersistence.GetById(attributeGroupId, attributeGroupModel);
    if (rc != 0) {
        std::string message = "Failed to get attribute group";
        QueueErrorNotificationEvent(message);
        return;
    }

    if (attributeGroupModel.IsStatic) {
        pIsStaticCheckBoxCtrl->SetValue(true);
    } else {
        pIsStaticCheckBoxCtrl->SetValue(false);
    }
}

void AttributeDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pNameTextCtrl->Enable();
        pIsRequiredCheckBoxCtrl->Enable();
        pDescriptionTextCtrl->Enable();
        pAttributeGroupChoiceCtrl->Enable();
        pAttributeTypeChoiceCtrl->Enable();
    } else {
        pNameTextCtrl->Disable();
        pIsRequiredCheckBoxCtrl->Disable();
        pDescriptionTextCtrl->Disable();
        pAttributeGroupChoiceCtrl->Disable();
        pAttributeTypeChoiceCtrl->Disable();
    }
}

void AttributeDialog::OnAddAnotherCheck(wxCommandEvent& event)
{
    bAddAnotherAttribute = event.IsChecked();
}

void AttributeDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    TransferDataFromControls();

    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    int ret = 0;
    std::string message = "";

    if (!bIsEdit) {
        Persistence::AttributeGroupsPersistence attributeGroupsPersistence(
            pLogger, mDatabaseFilePath);
        bool isAttributeGroupAlreadyAssociated = false;
        ret = attributeGroupsPersistence.CheckAttributeGroupAttributeValuesUsage(
            mAttributeModel.AttributeGroupId, isAttributeGroupAlreadyAssociated);

        if (ret == -1) {
            message = "Failed to check attribute group associations";
            QueueErrorNotificationEvent(message);
            return;
        }

        if (isAttributeGroupAlreadyAssociated) {
            wxMessageBox("Selected attribute group is already associated with attribute values",
                Common::GetProgramName(),
                wxOK_DEFAULT | wxICON_WARNING);
            return;
        }

        std::int64_t attributeId = attributesPersistence.Create(mAttributeModel);
        ret = attributeId > 0 ? 1 : -1;

        message =
            attributeId == -1 ? "Failed to create attribute" : "Successfully created attribute";
    }

    if (bIsEdit && pIsActiveCheckBoxCtrl->IsChecked()) {
        bool isAttributeUsed = CheckAttributeUsage(attributesPersistence);
        if (isAttributeUsed) {
            wxMessageBox("Unable to edit attribute as it is in use",
                Common::GetProgramName(),
                wxOK_DEFAULT | wxICON_WARNING);
            return;
        }

        ret = attributesPersistence.Update(mAttributeModel);

        message = ret == -1 ? "Failed to update attribute" : "Successfully updated attribute";
    }

    if (bIsEdit && !pIsActiveCheckBoxCtrl->IsChecked()) {
        bool isAttributeUsed = CheckAttributeUsage(attributesPersistence);
        if (isAttributeUsed) {
            wxMessageBox("Unable to delete attribute as it is in use",
                Common::GetProgramName(),
                wxOK_DEFAULT | wxICON_WARNING);
            return;
        }
        ret = attributesPersistence.Delete(mAttributeId);

        message = ret == -1 ? "Failed to delete attribute" : "Successfully deleted attribute";
    }

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    if (ret == -1) {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

    } else {
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        // if we are editing, pParent is EditListDlg. We need to get parent of pParent and then we
        // have wxFrame
        wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);

        if (!bAddAnotherAttribute) {
            EndModal(wxID_OK);
        } else {
            pNameTextCtrl->ChangeValue("");
            pIsRequiredCheckBoxCtrl->SetValue(false);
            pDescriptionTextCtrl->ChangeValue("");
            pDescriptionTextCtrl->SetHint("Attribute description");
            pAttributeGroupChoiceCtrl->SetSelection(0);
            pAttributeTypeChoiceCtrl->SetSelection(0);

            pIsStaticCheckBoxCtrl->SetValue(false);
        }
    }
}

void AttributeDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool AttributeDialog::Validate()
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

    int attributeGroupIndex = pAttributeGroupChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* attributeGroupData = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeGroupChoiceCtrl->GetClientObject(attributeGroupIndex));
    if (attributeGroupData->GetValue() < 1) {
        auto validationMessage = "An attribute group selection is required";
        wxRichToolTip toolTip("Validation", validationMessage);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pAttributeGroupChoiceCtrl);
        return false;
    }

    int attributeTypeIndex = pAttributeTypeChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* attributeTypeData = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeTypeChoiceCtrl->GetClientObject(attributeTypeIndex));
    if (attributeTypeData->GetValue() < 1) {
        auto validationMessage = "A field type selection is required";
        wxRichToolTip toolTip("Validation", validationMessage);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pAttributeTypeChoiceCtrl);
        return false;
    }

    return true;
}

void AttributeDialog::TransferDataFromControls()
{
    mAttributeModel.AttributeId = mAttributeId;

    auto name = pNameTextCtrl->GetValue().ToStdString();
    mAttributeModel.Name = Utils::TrimWhitespace(name);
    mAttributeModel.IsRequired = pIsRequiredCheckBoxCtrl->GetValue();

    auto description = pDescriptionTextCtrl->GetValue().ToStdString();
    mAttributeModel.Description =
        description.empty() ? std::nullopt : std::make_optional(description);

    int attributeGroupIndex = pAttributeGroupChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* attributeGroupData = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeGroupChoiceCtrl->GetClientObject(attributeGroupIndex));
    mAttributeModel.AttributeGroupId = attributeGroupData->GetValue();

    int attributeTypeIndex = pAttributeTypeChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* attributeTypeData = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeTypeChoiceCtrl->GetClientObject(attributeTypeIndex));
    mAttributeModel.AttributeTypeId = attributeTypeData->GetValue();
}

bool AttributeDialog::CheckAttributeUsage(Persistence::AttributesPersistence& attributesPersistence)
{
    bool value = false;

    int ret = attributesPersistence.CheckAttributeUsage(mAttributeId, value);

    if (ret == -1) {
        std::string message = "Failed to check attribute usage";
        QueueErrorNotificationEvent(message);
    }

    return value;
}

void AttributeDialog::DisableChoiceControlsIfUsed()
{
    pAttributeGroupChoiceCtrl->Disable();
    pAttributeTypeChoiceCtrl->Disable();
    pIsActiveCheckBoxCtrl->Disable();
}

void AttributeDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    // if we are editing, pParent is EditListDlg. We need to get parent of pParent and
    // then we have wxFrame
    wxQueueEvent(bIsEdit ? pParent->GetParent() : pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
