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

#include <wx/statline.h>

#include "../../clientdata.h"

#include "../../../common/common.h"

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
    , pDateCreatedReadonlyTextCtrl(nullptr)
    , pDateModifiedReadonlyTextCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pSaveAndAddAnotherButton(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
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
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Decription (optional)");
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

    /* Begin edit metadata controls */

    /* Horizontal Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    mainSizer->Add(line1, wxSizerFlags().Border(wxTOP | wxBOTTOM, FromDIP(4)).Expand());

    /*auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
    mainSizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());*/

    /* Date Created text control */
    auto dateCreatedLabel = new wxStaticText(this, wxID_ANY, "Date Created");

    pDateCreatedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateCreatedReadonlyTextCtrl->Disable();

    /* Date Modified text control */
    auto dateModifiedLabel = new wxStaticText(this, wxID_ANY, "Date Modified");

    pDateModifiedReadonlyTextCtrl = new wxTextCtrl(this, wxID_ANY, "-");
    pDateModifiedReadonlyTextCtrl->Disable();

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    /* Metadata flex grid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    mainSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand());
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
    auto line2 = new wxStaticLine(this, wxID_ANY);
    mainSizer->Add(line2, wxSizerFlags().Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pSaveAndAddAnotherButton =
        new wxButton(this, tksIDC_SAVEANDADDANOTHERBUTTON, "Save + Add Another");

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pSaveAndAddAnotherButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->AddStretchSpacer();
    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(mainSizer);
}

// clang-format off
void AttributeDialog::ConfigureEventBindings()
{

}
// clang-format on

void AttributeDialog::FillControls()
{
    pAttributeGroupChoiceCtrl->Append(
        "Select an attribute group", new ClientData<std::int64_t>(-1));
    pAttributeGroupChoiceCtrl->SetSelection(0);

    pAttributeTypeChoiceCtrl->Append("Select a field type", new ClientData<std::int64_t>(-1));
    pAttributeTypeChoiceCtrl->SetSelection(0);
}

void AttributeDialog::DataToControls() {}

void AttributeDialog::OnIsActiveCheck(wxCommandEvent& event) {}

void AttributeDialog::OnOK(wxCommandEvent& event) {}

void AttributeDialog::OnCancel(wxCommandEvent& event) {}

bool AttributeDialog::Validate()
{
    return false;
}

void AttributeDialog::TransferData() {}
} // namespace tks::UI::dlg
