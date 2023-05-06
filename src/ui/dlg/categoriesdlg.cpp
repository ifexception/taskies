// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

#include "categoriesdlg.h"

#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <fmt/format.h>

#include "../../common/constants.h"
#include "../../common/common.h"
#include "../../core/environment.h"
#include "../../utils/utils.h"

#include "errordlg.h"

namespace tks::UI::dlg
{
CategoriesDialog::CategoriesDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Add Categories",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pNameTextCtrl(nullptr)
    , pDescriptionTextCtrl(nullptr)
    , pColorPickerCtrl(nullptr)
    , pBillableCtrl(nullptr)
    , pListCtrl(nullptr)
    , pAddButton(nullptr)
    , pRemoveButton(nullptr)
    , pRemoveAllButton(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetIconBundleName(), 0);
    SetIcons(iconBundle);
}

void CategoriesDialog::Initialize()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void CategoriesDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Layout Sizer */
    auto layoutSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(layoutSizer, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Left Sizer */
    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(leftSizer, 0);

    /* Details Box */
    auto detailsBox = new wxStaticBox(this, wxID_ANY, "Details");
    auto detailsBoxSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);
    leftSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Name Ctrl */
    auto categoryNameLabel = new wxStaticText(detailsBox, wxID_ANY, "Name");

    pNameTextCtrl = new wxTextCtrl(detailsBox, IDC_NAME);
    pNameTextCtrl->SetHint("Category name");
    pNameTextCtrl->SetToolTip("Enter a name for a Category");

    wxTextValidator nameValidator(wxFILTER_ALPHANUMERIC | wxFILTER_INCLUDE_CHAR_LIST);
    wxArrayString allowedCharacters;
    allowedCharacters.Add(" ");
    allowedCharacters.Add("-");
    allowedCharacters.Add(":");
    allowedCharacters.Add(";");
    allowedCharacters.Add(".");
    allowedCharacters.Add("|");
    allowedCharacters.Add("(");
    allowedCharacters.Add(")");
    allowedCharacters.Add("+");
    nameValidator.SetIncludes(allowedCharacters);

    pNameTextCtrl->SetValidator(nameValidator);

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(detailsBox, IDC_COLORPICKER);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCtrl = new wxCheckBox(detailsBox, IDC_BILLABLE, "Billable");
    pBillableCtrl->SetToolTip("Indicates if a task captured with associated category is billable");

    /* Details Grid Sizer */
    auto detailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    detailsGridSizer->AddGrowableCol(1, 1);

    detailsGridSizer->Add(categoryNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    detailsGridSizer->Add(pNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pColorPickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsGridSizer->Add(0, 0);
    detailsGridSizer->Add(pBillableCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    detailsBoxSizer->Add(detailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Description Box */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description (optional)");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);
    leftSizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Description Text Ctrl */
    pDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, IDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxTE_MULTILINE);
    pDescriptionTextCtrl->SetHint("Description (optional)");
    pDescriptionTextCtrl->SetToolTip("Enter an optional description for a category");
    descriptionBoxSizer->Add(pDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Center Sizer */
    auto centerSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(centerSizer, 0);

    pAddButton = new wxButton(this, wxID_ADD, "Add >>");
    centerSizer->Add(pAddButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pRemoveButton = new wxButton(this, wxID_REMOVE, "Remove");
    centerSizer->Add(pRemoveButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pRemoveAllButton = new wxButton(this, wxID_DELETE, "Remove all");
    centerSizer->Add(pRemoveAllButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Right Sizer */
    auto rightSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(rightSizer, 0);

    /* List Box */
    auto listStaticBox = new wxStaticBox(this, wxID_ANY, "Categories to add");
    auto listStaticBoxSizer = new wxStaticBoxSizer(listStaticBox, wxVERTICAL);
    rightSizer->Add(detailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* List Ctrl */
    pListCtrl = new wxListCtrl(listStaticBox, IDC_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_HRULES);
    pListCtrl->EnableCheckBoxes();

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    nameColumn.SetWidth(wxLIST_AUTOSIZE_USEHEADER);
    pListCtrl->InsertColumn(0, nameColumn);

    listStaticBoxSizer->Add(pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Bottom Sizer */
    auto layoutBottomSizer = new wxBoxSizer(wxHORIZONTAL);
    layoutSizer->Add(layoutBottomSizer, wxSizerFlags().Border(wxALL, 5).Expand());

    auto bottomSizer = new wxBoxSizer(wxVERTICAL);
    layoutBottomSizer->Add(bottomSizer, 1);

    /* Horizontal Line*/
    auto bottomSeparationLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    bottomSizer->Add(bottomSeparationLine, wxSizerFlags().Expand());

    pOkButton = new wxButton(this, wxID_OK, "OK");
    layoutBottomSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    layoutBottomSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
}

void CategoriesDialog::FillControls()
{
    pOkButton->Disable();
    pRemoveButton->Disable();
    pRemoveAllButton->Disable();
}

void CategoriesDialog::ConfigureEventBindings() {}

void CategoriesDialog::OnAdd(wxCommandEvent& event) {}

void CategoriesDialog::OnEdit(wxCommandEvent& event) {}

void CategoriesDialog::OnRemove(wxCommandEvent& event) {}

void CategoriesDialog::OnRemoveAll(wxCommandEvent& event) {}

void CategoriesDialog::OnOK(wxCommandEvent& event) {}

void CategoriesDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void CategoriesDialog::OnItemChecked(wxListEvent& event) {}

void CategoriesDialog::OnItemUnchecked(wxListEvent& event) {}

void CategoriesDialog::OnItemRightClick(wxListEvent& event) {}

void CategoriesDialog::ResetControlValues() {}

bool CategoriesDialog::TransferDataAndValidate()
{
    return true;
}
} // namespace tks::UI::dlg
