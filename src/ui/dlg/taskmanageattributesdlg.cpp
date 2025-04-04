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

#include <wx/statline.h>

#include "../events.h"
#include "../notificationclientdata.h"

#include "../../common/common.h"

#include "../../persistence/attributegroupspersistence.h"

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
    , pNoAttributesPanel(nullptr)
    , pAttributesPanel(nullptr)
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , mAttributeControlCounter(0)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
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

    /* Static Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    pMainSizer->Add(line1, wxSizerFlags().Expand());

    /* No attributes panel */
    pNoAttributesPanel = new wxPanel(this, wxID_ANY);

    auto noAttributesPanelSizer = new wxBoxSizer(wxVERTICAL);
    pNoAttributesPanel->SetSizer(noAttributesPanelSizer);

    auto noAttributesLabel = new wxStaticText(pNoAttributesPanel, wxID_ANY, "No attributes found");
    noAttributesLabel->SetFont(
        wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    noAttributesPanelSizer->Add(
        noAttributesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

    pMainSizer->Add(pNoAttributesPanel, wxSizerFlags().Expand());

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
    } else {
        pAttributeGroupNameTextCtrl->ChangeValue(attributeGroupModel.Name);
    }
}

void TaskManageAttributesDialog::ConfigureEventBindings() {}

void TaskManageAttributesDialog::DataToControls() {}

void TaskManageAttributesDialog::AppendAttributeControl() {}

void TaskManageAttributesDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
