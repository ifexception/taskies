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

#include "staticattributevaluesdlg.h"

#include <algorithm>
#include <optional>

#include <fmt/format.h>

#include <wx/richmsgdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/clientdata.h"

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/usererrormessages.h"
#include "../../../common/validator.h"

#include "../../../common/messages/persistencemessages.h"

#include "../../../persistence/attributegroupspersistence.h"
#include "../../../persistence/attributespersistence.h"
#include "../../../persistence/staticattributevaluespersistence.h"

#include "../../../models/attributegroupmodel.h"
#include "../../../models/attributemodel.h"

namespace tks::UI::dlg
{
StaticAttributeValuesDialog::StaticAttributeValuesDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t attributeGroupId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Static Attribute Values" : "New Static Attribute Values",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pLogger(logger)
    , pParent(parent)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mAttributeGroupId(attributeGroupId)
    , pMainSizer(nullptr)
    , pAttributeGroupChoiceCtrl(nullptr)
    , pAttributesBox(nullptr)
    , pAttributesBoxSizer(nullptr)
    , pAttributesControlFlexGridSizer(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , mAttributesMetadata()
    , mAttributeControlCounter(1)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(tks::Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void StaticAttributeValuesDialog::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();

    if (bIsEdit) {
        DataToControls();
    }
}

void StaticAttributeValuesDialog::CreateControls()
{
    /* Main Sizer */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Attribute group choice control */
    auto attributeGroupNameLabel = new wxStaticText(this, wxID_ANY, "Attribute Group");
    pAttributeGroupChoiceCtrl = new wxChoice(this, tksIDC_ATTRIBUTEGROUPCHOICECTRL);

    pMainSizer->Add(attributeGroupNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(pAttributeGroupChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Initial controls and sizers for attributes */
    pAttributesBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    pAttributesBoxSizer = new wxStaticBoxSizer(pAttributesBox, wxVERTICAL);
    pMainSizer->Add(pAttributesBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pAttributesControlFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    pAttributesControlFlexGridSizer->AddGrowableCol(1, 1);
    pAttributesBoxSizer->Add(
        pAttributesControlFlexGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Is Active static box control */
    auto isActiveStaticBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto isActiveStaticBoxSizer = new wxStaticBoxSizer(isActiveStaticBox, wxHORIZONTAL);
    pMainSizer->Add(isActiveStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl =
        new wxCheckBox(isActiveStaticBox, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this static attribute value is used/active");
    pIsActiveCheckBoxCtrl->Disable();

    isActiveStaticBoxSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    pMainSizer->Add(line2, wxSizerFlags().Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonsSizer->AddStretchSpacer();

    pOKButton = new wxButton(this, wxID_OK, "OK");
    pOKButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOKButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* End of Button Controls */

    SetSizerAndFit(pMainSizer);
    pMainSizer->SetSizeHints(this);
}

void StaticAttributeValuesDialog::FillControls()
{
    pAttributeGroupChoiceCtrl->Append(
        "Select an attribute group", new ClientData<std::int64_t>(-1));
    pAttributeGroupChoiceCtrl->SetSelection(0);

    std::vector<Model::AttributeGroupModel> attributeGroups;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = attributeGroupsPersistence.FilterByStaticFlag(attributeGroups);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterAttributeGroupsByStaticFlagMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
        return;
    }

    for (auto& attributeGroupModel : attributeGroups) {
        pAttributeGroupChoiceCtrl->Append(attributeGroupModel.Name,
            new ClientData<std::int64_t>(attributeGroupModel.AttributeGroupId));
    }
}

// clang-format off
void StaticAttributeValuesDialog::ConfigureEventBindings()
{
    pAttributeGroupChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &StaticAttributeValuesDialog::OnAttributeGroupChoiceSelection,
        this
    );

    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &StaticAttributeValuesDialog::OnIsActiveCheck,
        this
    );

    pOKButton->Bind(
        wxEVT_BUTTON,
        &StaticAttributeValuesDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &StaticAttributeValuesDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void StaticAttributeValuesDialog::DataToControls()
{
    for (size_t i = 0; i < pAttributeGroupChoiceCtrl->GetCount(); i++) {
        ClientData<std::int64_t>* data = reinterpret_cast<ClientData<std::int64_t>*>(
            pAttributeGroupChoiceCtrl->GetClientObject(i));
        if (mAttributeGroupId == data->GetValue()) {
            pAttributeGroupChoiceCtrl->SetSelection(i);
            break;
        }
    }

    std::vector<Model::AttributeModel> attributeModels;
    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = attributesPersistence.FilterByAttributeGroupIdAndIsStatic(
        mAttributeGroupId, attributeModels);

    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterAttributesByStaticFlagMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Build \"{0}\" control attributes from attribute group id \"{1}\"",
        attributeModels.size(),
        mAttributeGroupId);

    if (attributeModels.size() < 1) {
        auto noAttributesLabel = new wxStaticText(pAttributesBox, wxID_ANY, "No attributes found");
        auto noAttributesLabelFont =
            wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);
        noAttributesLabel->SetFont(noAttributesLabelFont);
        pAttributesBoxSizer->Add(
            noAttributesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

        pAttributesBoxSizer->Layout();
        pMainSizer->Layout();
        SetSizerAndFit(pMainSizer);
        return;
    }

    for (size_t i = 0; i < attributeModels.size(); i++) {
        SPDLOG_LOGGER_TRACE(pLogger,
            "Build attribute control name \"{0}\" with type \"{1}\"",
            attributeModels[i].Name,
            AttributeTypeToString((AttributeTypes) attributeModels[i].AttributeTypeId));

        auto controlId = tksIDC_ATTRIBUTECONTROLBASE + mAttributeControlCounter;

        AttributeMetadata attributeMetadata;
        attributeMetadata.IsRequired = attributeModels[i].IsRequired;
        attributeMetadata.AttributeName = attributeModels[i].Name;

        AttributeControl attributeControl;
        attributeControl.ControlId = controlId;

        switch ((AttributeTypes) attributeModels[i].AttributeTypeId) {
        case AttributeTypes::Text: {
            auto attributeLabel =
                new wxStaticText(pAttributesBox, wxID_ANY, attributeModels[i].Name);
            auto attributeTextControl = new wxTextCtrl(pAttributesBox, controlId);
            attributeTextControl->SetHint(attributeModels[i].Name);

            pAttributesControlFlexGridSizer->Add(
                attributeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
            pAttributesControlFlexGridSizer->Add(
                attributeTextControl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

            attributeControl.TextControl = attributeTextControl;

            break;
        }
        case AttributeTypes::Boolean: {
            auto attributeBooleanControl = new wxCheckBox(pAttributesBox,
                controlId,
                attributeModels[i].Name,
                wxDefaultPosition,
                wxDefaultSize,
                wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);

            pAttributesControlFlexGridSizer->Add(0, 0);
            pAttributesControlFlexGridSizer->Add(
                attributeBooleanControl, wxSizerFlags().Border(wxALL, FromDIP(4)));

            attributeControl.BooleanControl = attributeBooleanControl;

            break;
        }
        case AttributeTypes::Numeric: {
            auto attributeLabel =
                new wxStaticText(pAttributesBox, wxID_ANY, attributeModels[i].Name);
            auto attributeNumericControl = new wxTextCtrl(pAttributesBox,
                controlId,
                wxEmptyString,
                wxDefaultPosition,
                wxDefaultSize,
                wxTE_LEFT,
                wxTextValidator(wxFILTER_NUMERIC));
            attributeNumericControl->SetHint(attributeModels[i].Name);

            pAttributesControlFlexGridSizer->Add(
                attributeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
            pAttributesControlFlexGridSizer->Add(
                attributeNumericControl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

            attributeControl.NumericControl = attributeNumericControl;

            break;
        }
        default:
            break;
        }

        attributeMetadata.AttributeType = (AttributeTypes) attributeModels[i].AttributeTypeId;
        attributeMetadata.AttributeId = attributeModels[i].AttributeId;

        attributeMetadata.Control = attributeControl;

        mAttributeControlCounter++;

        mAttributesMetadata.push_back(attributeMetadata);
    }

    pAttributesBoxSizer->Layout();
    pMainSizer->Layout();

    SetSizerAndFit(pMainSizer);

    std::vector<Model::StaticAttributeValueModel> staticAttributeValueModels;
    Persistence::StaticAttributeValuesPersistence staticAttributeValuesPersistence(
        pLogger, mDatabaseFilePath);

    sqliteResult = staticAttributeValuesPersistence.FilterByAttributeGroupId(
        mAttributeGroupId, staticAttributeValueModels);
    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterStaticAttributesByAttributeGroupIdMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    assert(mAttributesMetadata.size() == staticAttributeValueModels.size());

    for (size_t i = 0; i < mAttributesMetadata.size(); i++) {
        switch (mAttributesMetadata[i].AttributeType) {
        case AttributeTypes::Text: {
            if (staticAttributeValueModels[i].TextValue.has_value()) {
                mAttributesMetadata[i].Control.TextControl->ChangeValue(
                    staticAttributeValueModels[i].TextValue.value());
            }
            break;
        }
        case AttributeTypes::Boolean: {
            if (staticAttributeValueModels[i].BooleanValue.has_value()) {
                if (staticAttributeValueModels[i].BooleanValue.value()) {
                    mAttributesMetadata[i].Control.BooleanControl->Set3StateValue(wxCHK_CHECKED);
                } else {
                    mAttributesMetadata[i].Control.BooleanControl->Set3StateValue(wxCHK_UNCHECKED);
                }
            } else {
                mAttributesMetadata[i].Control.BooleanControl->Set3StateValue(wxCHK_UNDETERMINED);
            }
            break;
        }
        case AttributeTypes::Numeric: {
            if (staticAttributeValueModels[i].NumericValue.has_value()) {
                mAttributesMetadata[i].Control.NumericControl->ChangeValue(
                    std::to_string(staticAttributeValueModels[i].NumericValue.value()));
            }
            break;
        }
        default:
            pLogger->error("Unmatched attribute type, cannot set control values");
            break;
        }

        mAttributesMetadata[i].StaticAttributeValueId =
            staticAttributeValueModels[i].StaticAttributeValueId;
    }

    pIsActiveCheckBoxCtrl->Enable();
    pIsActiveCheckBoxCtrl->SetValue(true); // TODO: might need to look at this again
}

void StaticAttributeValuesDialog::OnAttributeGroupChoiceSelection(wxCommandEvent& event)
{
    auto selection = event.GetSelection();
    if (selection < 1) {
        return;
    }

    auto data = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeGroupChoiceCtrl->GetClientObject(selection));
    mAttributeGroupId = data->GetValue();

    std::vector<Model::AttributeModel> attributeModels;
    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    auto sqliteResult = attributesPersistence.FilterByAttributeGroupIdAndIsStatic(
        mAttributeGroupId, attributeModels);

    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterAttributesByStaticFlagMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Build \"{0}\" control attributes from attribute group id \"{1}\"",
        attributeModels.size(),
        mAttributeGroupId);

    if (attributeModels.size() < 1) {
        auto noAttributesLabel = new wxStaticText(pAttributesBox, wxID_ANY, "No attributes found");
        auto noAttributesLabelFont =
            wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL);
        noAttributesLabel->SetFont(noAttributesLabelFont);
        pAttributesBoxSizer->Add(
            noAttributesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

        pAttributesBoxSizer->Layout();
        pMainSizer->Layout();
        SetSizerAndFit(pMainSizer);
        return;
    }

    for (size_t i = 0; i < attributeModels.size(); i++) {
        SPDLOG_LOGGER_TRACE(pLogger,
            "Build attribute control name \"{0}\" with type \"{1}\"",
            attributeModels[i].Name,
            AttributeTypeToString((AttributeTypes) attributeModels[i].AttributeTypeId));

        auto controlId = tksIDC_ATTRIBUTECONTROLBASE + mAttributeControlCounter;

        AttributeMetadata attributeMetadata;
        attributeMetadata.IsRequired = attributeModels[i].IsRequired;
        attributeMetadata.AttributeName = attributeModels[i].Name;

        AttributeControl attributeControl;
        attributeControl.ControlId = controlId;

        switch ((AttributeTypes) attributeModels[i].AttributeTypeId) {
        case AttributeTypes::Text: {
            auto attributeLabel =
                new wxStaticText(pAttributesBox, wxID_ANY, attributeModels[i].Name);
            auto attributeTextControl = new wxTextCtrl(pAttributesBox, controlId);
            attributeTextControl->SetHint(attributeModels[i].Name);

            pAttributesControlFlexGridSizer->Add(
                attributeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
            pAttributesControlFlexGridSizer->Add(
                attributeTextControl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

            attributeControl.TextControl = attributeTextControl;

            break;
        }
        case AttributeTypes::Boolean: {
            auto attributeBooleanControl = new wxCheckBox(pAttributesBox,
                controlId,
                attributeModels[i].Name,
                wxDefaultPosition,
                wxDefaultSize,
                wxCHK_3STATE | wxCHK_ALLOW_3RD_STATE_FOR_USER);

            pAttributesControlFlexGridSizer->Add(0, 0);
            pAttributesControlFlexGridSizer->Add(
                attributeBooleanControl, wxSizerFlags().Border(wxALL, FromDIP(4)));

            attributeControl.BooleanControl = attributeBooleanControl;

            break;
        }
        case AttributeTypes::Numeric: {
            auto attributeLabel =
                new wxStaticText(pAttributesBox, wxID_ANY, attributeModels[i].Name);
            auto attributeNumericControl = new wxTextCtrl(pAttributesBox,
                controlId,
                wxEmptyString,
                wxDefaultPosition,
                wxDefaultSize,
                wxTE_LEFT,
                wxTextValidator(wxFILTER_NUMERIC));
            attributeNumericControl->SetHint(attributeModels[i].Name);

            pAttributesControlFlexGridSizer->Add(
                attributeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
            pAttributesControlFlexGridSizer->Add(
                attributeNumericControl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

            attributeControl.NumericControl = attributeNumericControl;

            break;
        }
        default:
            break;
        }

        attributeMetadata.AttributeType = (AttributeTypes) attributeModels[i].AttributeTypeId;
        attributeMetadata.AttributeId = attributeModels[i].AttributeId;

        attributeMetadata.Control = attributeControl;

        mAttributeControlCounter++;

        mAttributesMetadata.push_back(attributeMetadata);
    }

    pAttributesBoxSizer->Layout();
    pMainSizer->Layout();

    SetSizerAndFit(pMainSizer);
}

void StaticAttributeValuesDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (!event.IsChecked()) {
        pAttributeGroupChoiceCtrl->Disable();

        for (size_t i = 0; i < mAttributesMetadata.size(); i++) {
            switch (mAttributesMetadata[i].AttributeType) {
            case AttributeTypes::Text: {
                mAttributesMetadata[i].Control.TextControl->Disable();
                break;
            }
            case AttributeTypes::Boolean: {
                mAttributesMetadata[i].Control.BooleanControl->Disable();
                break;
            }
            case AttributeTypes::Numeric: {
                mAttributesMetadata[i].Control.NumericControl->Disable();
                break;
            }
            default:
                pLogger->warn("Unmatched attribute type, cannot set control values");
                break;
            }
        }
    } else {
        pAttributeGroupChoiceCtrl->Enable();

        for (size_t i = 0; i < mAttributesMetadata.size(); i++) {
            switch (mAttributesMetadata[i].AttributeType) {
            case AttributeTypes::Text: {
                mAttributesMetadata[i].Control.TextControl->Enable();
                break;
            }
            case AttributeTypes::Boolean: {
                mAttributesMetadata[i].Control.BooleanControl->Enable();
                break;
            }
            case AttributeTypes::Numeric: {
                mAttributesMetadata[i].Control.NumericControl->Enable();
                break;
            }
            default:
                pLogger->warn("Unmatched attribute type, cannot set control values");
                break;
            }
        }
    }
}

void StaticAttributeValuesDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    auto staticAttributeValueModels = TransferDataFromControls();
    if (staticAttributeValueModels.size() > 1) {
        Persistence::StaticAttributeValuesPersistence staticAttributeValuesPersistence(
            pLogger, mDatabaseFilePath);

        if (!bIsEdit) {
            auto sqliteResult =
                staticAttributeValuesPersistence.CreateMultiple(staticAttributeValueModels);

            if (!sqliteResult.Success) {
                wxRichMessageDialog dialog(this,
                    Messages::CreateStaticAttributeMessage,
                    Common::GetProgramName(),
                    wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                dialog.ShowModal();
            }
        } else if (bIsEdit && pIsActiveCheckBoxCtrl->GetValue()) {
            auto sqliteResult =
                staticAttributeValuesPersistence.UpdateMultiple(staticAttributeValueModels);

            if (!sqliteResult.Success) {
                wxRichMessageDialog dialog(this,
                    Messages::UpdateStaticAttributeMessage,
                    Common::GetProgramName(),
                    wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                dialog.ShowModal();
            }
        } else if (bIsEdit && !pIsActiveCheckBoxCtrl->GetValue()) {
            bool areStaticAttributeValuesUsed = false;
            std::vector<std::int64_t> attributeIds;

            // clang-format off
            std::transform(
                std::begin(staticAttributeValueModels),
                std::end(staticAttributeValueModels),
                std::back_inserter(attributeIds),
                [](const Model::StaticAttributeValueModel& staticAttributeValueModel) {
                    return staticAttributeValueModel.AttributeId;
                }
            );
            // clang-format on

            auto sqliteResult = staticAttributeValuesPersistence.CheckUsage(
                attributeIds, areStaticAttributeValuesUsed);

            if (!sqliteResult.Success) {
                wxRichMessageDialog dialog(this,
                    Messages::CheckUsageStaticAttributeMessage,
                    Common::GetProgramName(),
                    wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                dialog.ShowModal();
                return;
            }

            if (areStaticAttributeValuesUsed) {
                wxMessageBox("Unable to delete static attribute values as they are in use",
                    Common::GetProgramName(),
                    wxOK_DEFAULT | wxICON_WARNING);
                return;
            }

            std::vector<std::int64_t> staticAttributeValueIds;

            // clang-format off
            std::transform(
                std::begin(staticAttributeValueModels),
                std::end(staticAttributeValueModels),
                std::back_inserter(staticAttributeValueIds),
                [](const Model::StaticAttributeValueModel& staticAttributeValueModel) {
                    return staticAttributeValueModel.StaticAttributeValueId;
                }
            );
            // clang-format on

            sqliteResult = staticAttributeValuesPersistence.Delete(staticAttributeValueIds);

            if (!sqliteResult.Success) {
                wxRichMessageDialog dialog(this,
                    Messages::DeleteStaticAttributeMessage,
                    Common::GetProgramName(),
                    wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
                dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
                dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

                dialog.ShowModal();
                return;
            }
        }
    }

    EndModal(wxID_OK);
}

void StaticAttributeValuesDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool StaticAttributeValuesDialog::Validate()
{
    for (const auto& attributeMetadata : mAttributesMetadata) {
        if (attributeMetadata.IsRequired) {
            switch (attributeMetadata.AttributeType) {
            case AttributeTypes::Text: {
                std::string value = attributeMetadata.Control.TextControl->GetValue().ToStdString();
                if (value.empty()) {
                    std::string validation = fmt::format(
                        "A value is required for \"{0}\"", attributeMetadata.AttributeName);
                    wxRichToolTip toolTip("Validation", validation);
                    toolTip.SetIcon(wxICON_WARNING);
                    toolTip.ShowFor(attributeMetadata.Control.TextControl);
                    return false;
                }
                break;
            }
            case AttributeTypes::Boolean: {
                int isUndeterminedState =
                    attributeMetadata.Control.BooleanControl->Get3StateValue();
                if (isUndeterminedState == wxCHK_UNDETERMINED) {
                    std::string validation = fmt::format(
                        "A value is required for \"{0}\"", attributeMetadata.AttributeName);
                    wxRichToolTip toolTip("Validation", validation);
                    toolTip.SetIcon(wxICON_WARNING);
                    toolTip.ShowFor(attributeMetadata.Control.BooleanControl);
                    return false;
                }
                break;
            }
            case AttributeTypes::Numeric: {
                std::string value =
                    attributeMetadata.Control.NumericControl->GetValue().ToStdString();
                if (value.empty()) {
                    std::string validation = fmt::format(
                        "A value is required for \"{0}\"", attributeMetadata.AttributeName);
                    wxRichToolTip toolTip("Validation", validation);
                    toolTip.SetIcon(wxICON_WARNING);
                    toolTip.ShowFor(attributeMetadata.Control.NumericControl);
                    return false;
                }
                break;
            }
            default:
                return false;
            }
        }
    }

    return true;
}

std::vector<Model::StaticAttributeValueModel>
    StaticAttributeValuesDialog::TransferDataFromControls()
{
    std::vector<Model::StaticAttributeValueModel> staticAttributeValueModels;
    for (const auto& attributeMetadata : mAttributesMetadata) {
        Model::StaticAttributeValueModel staticAttributeValueModel;

        staticAttributeValueModel.StaticAttributeValueId = attributeMetadata.StaticAttributeValueId;
        staticAttributeValueModel.AttributeGroupId = mAttributeGroupId;
        staticAttributeValueModel.AttributeId = attributeMetadata.AttributeId;

        switch (attributeMetadata.AttributeType) {
        case AttributeTypes::Text: {
            std::string value = attributeMetadata.Control.TextControl->GetValue().ToStdString();
            if (!value.empty()) {
                staticAttributeValueModel.TextValue = std::make_optional<std::string>(value);
            }
            break;
        }
        case AttributeTypes::Boolean: {
            std::optional<bool> boolValue;

            int threeStateValue = attributeMetadata.Control.BooleanControl->Get3StateValue();
            switch (threeStateValue) {
            case wxCHK_UNCHECKED:
                boolValue = std::make_optional<bool>(false);
                break;
            case wxCHK_CHECKED:
                boolValue = std::make_optional<bool>(true);
                break;
            case wxCHK_UNDETERMINED:
                boolValue = std::nullopt;
                break;
            default:
                break;
            }
            staticAttributeValueModel.BooleanValue = boolValue;
            break;
        }
        case AttributeTypes::Numeric: {
            std::string intValue =
                attributeMetadata.Control.NumericControl->GetValue().ToStdString();
            if (!intValue.empty()) {
                int value =
                    std::stoi(attributeMetadata.Control.NumericControl->GetValue().ToStdString());
                staticAttributeValueModel.NumericValue = std::make_optional<int>(value);
            }
            break;
        }
        default:
            break;
        }

        staticAttributeValueModels.push_back(staticAttributeValueModel);
    }

    return staticAttributeValueModels;
}
} // namespace tks::UI::dlg
