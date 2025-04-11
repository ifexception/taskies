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

#include "taskmanageattributesdlg.h"

#include <vector>

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/valtext.h>

#include "../events.h"
#include "../notificationclientdata.h"

#include "../common/taskattributevalueclientdata.h"

#include "../../common/common.h"

#include "../../persistence/attributegroupspersistence.h"
#include "../../persistence/attributespersistence.h"

#include "../../models/attributegroupmodel.h"

namespace tks::UI::dlg
{
TaskManageAttributesDialog::TaskManageAttributesDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    std::int64_t attributeGroupId,
    bool isEdit,
    std::int64_t taskId,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Manage Attributes",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , mAttributeGroupId(attributeGroupId)
    , bIsEdit(isEdit)
    , mTaskId(taskId)
    , pMainSizer(nullptr)
    , pAttributeGroupNameTextCtrl(nullptr)
    , pAttributesBox(nullptr)
    , pAttributesBoxSizer(nullptr)
    , pAttributesControlFlexGridSizer(nullptr)
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , mAttributeControlCounter(1)
    , mAttributeControls()
    , mTaskAttributeValueModels()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(tks::Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void TaskManageAttributesDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();

    if (bIsEdit) {
        DataToControls();
    }
}

void TaskManageAttributesDialog::CreateControls()
{
    /* Main Sizer */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Attribute group name horizontal sizer */
    auto attributeGroupNameHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    pMainSizer->Add(attributeGroupNameHorizontalSizer, wxSizerFlags().Expand());

    /* Attribute group name text control */
    auto attributeGroupNameLabel = new wxStaticText(this, wxID_ANY, "Attribute Group Name");
    pAttributeGroupNameTextCtrl = new wxTextCtrl(
        this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    attributeGroupNameHorizontalSizer->Add(
        attributeGroupNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    attributeGroupNameHorizontalSizer->Add(
        pAttributeGroupNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Proportion(1));

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

void TaskManageAttributesDialog::FillControls()
{
    Model::AttributeGroupModel attributeGroupModel;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupsPersistence.GetById(mAttributeGroupId, attributeGroupModel);
    if (rc != 0) {
        std::string message = "Failed to fetch attribute group";
        QueueErrorNotificationEvent(message);
        return;
    }

    pAttributeGroupNameTextCtrl->ChangeValue(attributeGroupModel.Name);

    std::vector<Model::AttributeModel> attributeModels;
    Persistence::AttributesPersistence attributesPersistence(pLogger, mDatabaseFilePath);

    rc = attributesPersistence.FilterByAttributeGroupId(mAttributeGroupId, attributeModels);
    if (rc != 0) {
        std::string message = "Failed to fetch attributes";
        QueueErrorNotificationEvent(message);
        return;
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Build \"{0}\" control attributes from attribute group id \"{1}\"",
        attributeModels.size(),
        mAttributeGroupId);

    if (attributeModels.size() < 1) {
        auto noAttributesLabel = new wxStaticText(pAttributesBox, wxID_ANY, "No attributes found");
        pAttributesBoxSizer->Add(
            noAttributesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
        pMainSizer->Layout();
        return;
    }

    for (size_t i = 0; i < attributeModels.size(); i++) {
        SPDLOG_LOGGER_TRACE(pLogger,
            "Build attribute control name \"{0}\" with type \"{1}\"",
            attributeModels[i].Name,
            AttributeTypeToString((AttributeTypes) attributeModels[i].AttributeTypeId));

        auto controlId = tksIDC_ATTRIBUTECONTROLBASE + mAttributeControlCounter;

        AttributeControlData attributeControlData;
        attributeControlData.ControlId = controlId;
        attributeControlData.IsRequired = attributeModels[i].IsRequired;
        attributeControlData.Name = attributeModels[i].Name;

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

            attributeControlData.TextControl = attributeTextControl;

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

            attributeControlData.BooleanControl = attributeBooleanControl;

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

            attributeControlData.NumericControl = attributeNumericControl;

            break;
        }
        default:
            break;
        }

        attributeControlData.AttributeType = (AttributeTypes) attributeModels[i].AttributeTypeId;
        attributeControlData.AttributeId = attributeModels[i].AttributeId;
        mAttributeControlCounter++;

        mAttributeControls.push_back(attributeControlData);
    }

    pAttributesBoxSizer->Layout();
    pMainSizer->Layout();

    SetSizerAndFit(pMainSizer);
}

// clang-format off
void TaskManageAttributesDialog::ConfigureEventBindings()
{
    pOKButton->Bind(
        wxEVT_BUTTON,
        &TaskManageAttributesDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &TaskManageAttributesDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void TaskManageAttributesDialog::DataToControls() {}

void TaskManageAttributesDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    TransferDataFromControls();
    return;

    if (mTaskAttributeValueModels.size() >= 1) {
        wxCommandEvent* taskAttributeValuesAddedEvent =
            new wxCommandEvent(tksEVT_TASKDLGATTRIBUTESADDED);

        Common::TaskAttributeValueClientData* clientData =
            new Common::TaskAttributeValueClientData(mTaskAttributeValueModels);
        taskAttributeValuesAddedEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, taskAttributeValuesAddedEvent);
    }

    EndModal(wxID_OK);
}

void TaskManageAttributesDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool TaskManageAttributesDialog::Validate()
{
    for (const auto& attributeControl : mAttributeControls) {
        if (attributeControl.IsRequired) {
            switch (attributeControl.AttributeType) {
            case AttributeTypes::Text: {
                std::string value = attributeControl.TextControl->GetValue().ToStdString();
                if (value.empty()) {
                    std::string validation =
                        fmt::format("A value is required for \"{0}\"", attributeControl.Name);
                    wxRichToolTip toolTip("Validation", validation);
                    toolTip.SetIcon(wxICON_WARNING);
                    toolTip.ShowFor(attributeControl.TextControl);
                    return false;
                }
                break;
            }
            case AttributeTypes::Boolean:
                [[fallthrough]];
            case AttributeTypes::Numeric:
                [[fallthrough]];
            default:
                return false;
            }
        }
    }

    return true;
}

void TaskManageAttributesDialog::TransferDataFromControls()
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Begin transferring of controls (count of controls: {0})",
        mAttributeControls.size());

    for (const auto& attributeControl : mAttributeControls) {
        Model::TaskAttributeValueModel taskAttributeModel;
        taskAttributeModel.AttributeId = attributeControl.AttributeId;

        switch (attributeControl.AttributeType) {
        case AttributeTypes::Text: {
            std::string value = attributeControl.TextControl->GetValue().ToStdString();
            if (!value.empty()) {
                taskAttributeModel.TextValue = std::make_optional<std::string>(value);
            }
            break;
        }
        case AttributeTypes::Boolean: {
            auto threeStateValue = attributeControl.BooleanControl->Get3StateValue();
            std::optional<bool> boolValue;
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
            taskAttributeModel.BooleanValue = boolValue;
            break;
        }
        case AttributeTypes::Numeric: {
            auto intValue = attributeControl.NumericControl->GetValue().ToStdString();
            if (!intValue.empty()) {
                int value = std::stoi(attributeControl.NumericControl->GetValue().ToStdString());
                taskAttributeModel.NumericValue = std::make_optional<int>(value);
            }
            break;
        }
        default:
            pLogger->warn("No matching attribute type found");
            break;
        }

        mTaskAttributeValueModels.push_back(taskAttributeModel);
    }
}

void TaskManageAttributesDialog::AppendAttributeControl(const Model::AttributeModel& model) {}

void TaskManageAttributesDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
