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

#include "editlistdlg.h"

#include <wx/statline.h>

#include "../../common/common.h"

namespace tks::UI
{
EditListDialog::EditListDialog(wxWindow* parent, std::shared_ptr<spdlog::logger> logger, const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Edit Employer",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , pListCtrl(nullptr)
{
    Create();

    wxIconBundle iconBundle(Common::GetIconBundleName(), 0);
    SetIcons(iconBundle);
}

void EditListDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    DataToControls();

    /*SetSize(FromDIP(wxSize(400, 380)));
    SetMinSize(FromDIP(wxSize(280, 380)));*/
}

void EditListDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Search */
    auto searchBox = new wxStaticBox(this, wxID_ANY, "Search");
    auto searchBoxSizer = new wxStaticBoxSizer(searchBox, wxHORIZONTAL);
    sizer->Add(searchBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Search Text Control */
    pSearchTextCtrl = new wxTextCtrl(searchBox, IDC_SEARCHTEXT);
    pSearchTextCtrl->SetHint("Search employers...");
    pSearchTextCtrl->SetToolTip("Enter a search term");
    searchBoxSizer->Add(pSearchTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Search Button */
    pSearchButton = new wxButton(searchBox, IDC_SEARCHBTN, "Search");
    searchBoxSizer->Add(pSearchButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Separation Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxTOP | wxBOTTOM, FromDIP(2)).Expand().Proportion(1));

    /* List Control */
    pListCtrl = new wxListCtrl(
        this, IDC_LIST, wxDefaultPosition, wxDefaultSize, wxLC_HRULES | wxLC_LIST | wxLC_SINGLE_SEL | wxLC_REPORT);

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    pListCtrl->InsertColumn(0, nameColumn);

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

//clang-format off
void EditListDialog::ConfigureEventBindings()
{
    pSearchTextCtrl->Bind(wxEVT_TEXT, &EditListDialog::OnSearchTextChange, this);

    pSearchButton->Bind(wxEVT_BUTTON, &EditListDialog::OnSearch, this, IDC_SEARCHBTN);

    pListCtrl->Bind(wxEVT_LIST_ITEM_SELECTED, &EditListDialog::OnItemSelected, this, IDC_LIST);

    pListCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, &EditListDialog::OnItemDoubleClick, this, IDC_LIST);

    pOkButton->Bind(wxEVT_BUTTON, &EditListDialog::OnOK, this, wxID_OK);

    pCancelButton->Bind(wxEVT_BUTTON, &EditListDialog::OnCancel, this, wxID_CANCEL);
}
//clang-format on

void EditListDialog::DataToControls() {}

void EditListDialog::OnSearchTextChange(wxCommandEvent& event) {}

void EditListDialog::OnSearch(wxCommandEvent& event) {}

void EditListDialog::OnItemSelected(wxCommandEvent& event) {}

void EditListDialog::OnItemDoubleClick(wxCommandEvent& event) {}

void EditListDialog::OnOK(wxCommandEvent& event) {}

void EditListDialog::OnCancel(wxCommandEvent& event) {}
} // namespace tks::UI