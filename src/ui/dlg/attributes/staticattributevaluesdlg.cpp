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

#include "staticattributevaluesdlg.h"

#include <optional>

#include <fmt/format.h>

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../clientdata.h"
#include "../../events.h"
#include "../../notificationclientdata.h"

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/validator.h"

#include "../../../persistence/attributegroupspersistence.h"
#include "../../../persistence/attributespersistence.h"

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
          isEdit ? "Edit Static Attributes" : "New Static Attributes",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mAttributeGroupId(attributeGroupId)
    , pMainSizer(nullptr)
    , pAttributeGroupChoiceCtrl(nullptr)
    , pAttributesBox(nullptr)
    , pAttributesBoxSizer(nullptr)
    , pAttributesControlFlexGridSizer(nullptr)
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

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    pMainSizer->Add(line2, wxSizerFlags().Expand());

    /* Begin Button Controls */

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

    int rc = attributeGroupsPersistence.FilterByStaticFlag(attributeGroups);
    if (rc != 0) {
        std::string message = "Failed to get static attribute groups";
        QueueErrorNotificationEvent(message);

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

void StaticAttributeValuesDialog::DataToControls() {}

void StaticAttributeValuesDialog::OnAttributeGroupChoiceSelection(wxCommandEvent& event)
{
    auto selection = event.GetSelection();
    if (selection < 1) {
        return;
    }

    auto data = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeGroupChoiceCtrl->GetClientObject(selection));
    std::int64_t attributeGroupId = data->GetValue();

    std::vector<Model::AttributeModel> attributeModels;
    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    int rc = attributesPersistence.FilterByAttributeGroupIdAndIsStatic(
        attributeGroupId, attributeModels);

    if (rc != 0) {
        std::string message = "Failed to fetch attributes";
        QueueErrorNotificationEvent(message);
        return;
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Build \"{0}\" control attributes from attribute group id \"{1}\"",
        attributeModels.size(),
        attributeGroupId);

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
        attributeMetadata.Name = attributeModels[i].Name;

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

void StaticAttributeValuesDialog::OnOK(wxCommandEvent& event)
{
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
                    std::string validation =
                        fmt::format("A value is required for \"{0}\"", attributeMetadata.Name);
                    wxRichToolTip toolTip("Validation", validation);
                    toolTip.SetIcon(wxICON_WARNING);
                    toolTip.ShowFor(attributeMetadata.Control.TextControl);
                    return false;
                }
                break;
            }
            case AttributeTypes::Boolean: {
                int isUndeterminedState = attributeMetadata.Control.BooleanControl->Get3StateValue();
                if (isUndeterminedState == wxCHK_UNDETERMINED) {
                    std::string validation =
                        fmt::format("A value is required for \"{0}\"", attributeMetadata.Name);
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
                    std::string validation =
                        fmt::format("A value is required for \"{0}\"", attributeMetadata.Name);
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

void StaticAttributeValuesDialog::TransferDataFromControls() {}

void StaticAttributeValuesDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
