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

#include "attributegroupdlg.h"

#include <wx/statline.h>

#include "../../../common/common.h"
#include "../../../common/constants.h"

namespace tks::UI::dlg
{
AttributeGroupDialog::AttributeGroupDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    std::int64_t attributeGroupId,
    bool isEdit,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Attribute Group" : "New Attribute Group",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , mDatabaseFilePath(databaseFilePath)
    , mAttributeGroupId(attributeGroupId)
    , bIsEdit(isEdit)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pDateCreatedReadonlyTextCtrl(nullptr)
    , pDateModifiedReadonlyTextCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
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
    FillControls();

    // do edit
}

void AttributeGroupDialog::CreateControls()
{
    /* Main dialog sizer for controls */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    /* Details static box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    mainSizer->Add(detailsBoxSizer, wxSizerFlags().Expand());

    /* Attribute group name label control */
    auto attributeGroupNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    /* Attribute group name text control*/
    pNameTextCtrl = new wxTextCtrl(detailsBox, tksIDC_NAMETEXTCTRL);
    pNameTextCtrl->SetHint("Attribute group name");
    pNameTextCtrl->SetToolTip("Set an attribute group name");

    /* Grid sizer for attribute group name controls */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(
        attributeGroupNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(
        pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Attribute group description static box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Decription (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    mainSizer->Add(descriptionBoxSizer, wxSizerFlags().Expand());

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

    /* Begin edit metadata controls */

    auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
    mainSizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Date Created text control */
    auto dateCreatedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Created");

    pDateCreatedReadonlyTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, "-");
    pDateCreatedReadonlyTextCtrl->Disable();

    /* Date Modified text control */
    auto dateModifiedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Modified");

    pDateModifiedReadonlyTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, "-");
    pDateModifiedReadonlyTextCtrl->Disable();

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    /* Metadata flex grid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(8));
    metadataBoxSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand());
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
    mainSizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(mainSizer);
}

void AttributeGroupDialog::FillControls() {}

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

void AttributeGroupDialog::OnIsActiveCheck(wxCommandEvent& event) {}

void AttributeGroupDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }

    EndModal(wxID_OK);
}

void AttributeGroupDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool AttributeGroupDialog::Validate()
{
    return true;
}

void AttributeGroupDialog::TransferData() {}
} // namespace tks::UI::dlg
