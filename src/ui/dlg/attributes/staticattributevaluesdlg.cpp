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

#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../clientdata.h"
#include "../../events.h"
#include "../../notificationclientdata.h"

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/validator.h"

#include "../../../persistence/attributegroupspersistence.h"

#include "../../../models/attributegroupmodel.h"

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
{ /* Main Sizer */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Attribute group name horizontal sizer */
    auto attributeGroupNameHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    pMainSizer->Add(attributeGroupNameHorizontalSizer, wxSizerFlags().Expand());

    /* Attribute group name text control */
    auto attributeGroupNameLabel = new wxStaticText(this, wxID_ANY, "Attribute Group");
    pAttributeGroupChoiceCtrl = new wxChoice(this, tksIDC_ATTRIBUTEGROUPCHOICECTRL);

    attributeGroupNameHorizontalSizer->Add(
        attributeGroupNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    attributeGroupNameHorizontalSizer->Add(
        pAttributeGroupChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Proportion(1));

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

void StaticAttributeValuesDialog::FillControls() {}

void StaticAttributeValuesDialog::ConfigureEventBindings() {}

void StaticAttributeValuesDialog::DataToControls() {}

void StaticAttributeValuesDialog::OnIsActiveCheck(wxCommandEvent& event) {}

void StaticAttributeValuesDialog::OnOK(wxCommandEvent& event) {}

void StaticAttributeValuesDialog::OnCancel(wxCommandEvent& event) {}

bool StaticAttributeValuesDialog::Validate()
{
    return false;
}

void StaticAttributeValuesDialog::TransferDataFromControls() {}
} // namespace tks::UI::dlg
