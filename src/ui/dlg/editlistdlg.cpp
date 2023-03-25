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

#include <wx/artprov.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/common.h"
#include "../../utils/utils.h"
#include "../../core/environment.h"
#include "errordlg.h"
#include "employerdlg.h"

namespace tks::UI::dlg
{
EditListDialog::EditListDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Edit Employer",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , mData(pEnv, pLogger)
    , pSearchTextCtrl(nullptr)
    , pSearchButton(nullptr)
    , pResetButton(nullptr)
    , pListCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mSearchTerm()
    , mEmployerId(-1)
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

    SetMinSize(FromDIP(wxSize(334, 290)));
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
    pSearchTextCtrl = new wxTextCtrl(searchBox,
        IDC_SEARCHTEXT,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT /* | wxTE_PROCESS_ENTER*/);
    pSearchTextCtrl->SetHint("Search employers...");
    pSearchTextCtrl->SetToolTip("Enter a search term");
    searchBoxSizer->Add(pSearchTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Search Button */
    auto providedFindBitmap =
        wxArtProvider::GetBitmapBundle(wxART_FIND, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pSearchButton = new wxBitmapButton(searchBox, IDC_SEARCHBTN, providedFindBitmap);
    pSearchButton->SetToolTip("Search for an employer based on the search term");
    searchBoxSizer->Add(pSearchButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Reset Button */
    auto providedCloseBitmap =
        wxArtProvider::GetBitmapBundle(wxART_CLOSE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pResetButton = new wxBitmapButton(searchBox, IDC_RESETBTN, providedCloseBitmap);
    pResetButton->SetToolTip("Reset search term");
    searchBoxSizer->Add(pResetButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Separation Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxTOP | wxBOTTOM, FromDIP(2)).Expand());

    /* List Control */
    pListCtrl =
        new wxListCtrl(this, IDC_LIST, wxDefaultPosition, wxDefaultSize, wxLC_HRULES | wxLC_REPORT | wxLC_SINGLE_SEL);
    sizer->Add(pListCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    wxListItem nameColumn;
    nameColumn.SetId(0);
    nameColumn.SetText("Name");
    nameColumn.SetWidth(wxLIST_AUTOSIZE_USEHEADER);
    pListCtrl->InsertColumn(0, nameColumn);

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line2, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    pOkButton->Disable();
    pCancelButton->Disable();

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

//clang-format off
void EditListDialog::ConfigureEventBindings()
{
    pSearchTextCtrl->Bind(
        wxEVT_TEXT,
        &EditListDialog::OnSearchTextChange,
        this
    );

    pSearchButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnSearch,
        this,
        IDC_SEARCHBTN
    );

    /*pSearchTextCtrl->Bind(
        wxEVT_KEY_DOWN,
        &EditListDialog::OnSearchEnterKeyPressed,
        this
    );*/

    pResetButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnReset,
        this,
        IDC_RESETBTN
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_SELECTED,
        &EditListDialog::OnItemSelected,
        this,
        IDC_LIST
    );

    pListCtrl->Bind(
        wxEVT_LIST_ITEM_ACTIVATED,
        &EditListDialog::OnItemDoubleClick,
        this,
        IDC_LIST
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &EditListDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
//clang-format on

void EditListDialog::DataToControls()
{
    auto employersTuple = mData.Filter(mSearchTerm);
    if (std::get<0>(employersTuple) != 0) {
        ErrorDialog errorDialog(this, pLogger, "Error occured when filtering employers");
        errorDialog.ShowModal();
    } else {
        std::vector<Model::EmployerModel> employers = std::get<1>(employersTuple);
        int listIndex = 0;
        int columnIndex = 0;
        for (auto& employer : employers) {
            listIndex = pListCtrl->InsertItem(columnIndex++, employer.Name);
            pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(employer.EmployerId));
            columnIndex = 0;
        }
    }

    pOkButton->Enable();
    pCancelButton->Enable();
}

void EditListDialog::OnSearchTextChange(wxCommandEvent& event)
{
    mSearchTerm = pSearchTextCtrl->GetValue().Trim().ToStdString();
}

void EditListDialog::OnSearch(wxCommandEvent& event)
{
    if (mSearchTerm.length() < 3) {
        wxRichToolTip toolTip("", "Please enter 3 or more characters to search");
        toolTip.ShowFor(pSearchTextCtrl);
    } else {
        SearchEmployers();
    }
}

//void EditListDialog::OnSearchEnterKeyPressed(wxKeyEvent& event)
//{
//    if (event.GetKeyCode() == WXK_RETURN) {
//        SearchEmployers();
//    }
//
//    event.Skip();
//}

void EditListDialog::OnReset(wxCommandEvent& event)
{
    mSearchTerm = "";
    pSearchTextCtrl->SetValue(wxEmptyString);
    SearchEmployers();
}

void EditListDialog::OnItemSelected(wxListEvent& event)
{
    mEmployerId = static_cast<std::int64_t>(event.GetData());
}

void EditListDialog::OnItemDoubleClick(wxListEvent& event)
{
    mEmployerId = static_cast<std::int64_t>(event.GetData());
    EmployerDialog employerDlg(this, pEnv, pLogger, true, mEmployerId);
    employerDlg.ShowModal();

    mEmployerId = -1;
}

void EditListDialog::OnOK(wxCommandEvent& event)
{
    if (mEmployerId > 0) {
        EmployerDialog employerDlg(this, pEnv, pLogger, true, mEmployerId);
        employerDlg.ShowModal();

        mEmployerId = -1;
    } else {
        wxRichToolTip toolTip("Selection Required", "Please select an employer to edit");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pOkButton);
    }
}

void EditListDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void EditListDialog::SearchEmployers()
{
    pOkButton->Disable();
    pCancelButton->Disable();

    pListCtrl->DeleteAllItems();

    auto employersTuple = mData.Filter(mSearchTerm);
    if (std::get<0>(employersTuple) != 0) {
        ErrorDialog errorDialog(this, pLogger, "Error occured when filtering employers");
        errorDialog.ShowModal();
    } else {
        std::vector<Model::EmployerModel> employers = std::get<1>(employersTuple);
        int listIndex = 0;
        int columnIndex = 0;
        for (auto& employer : employers) {
            listIndex = pListCtrl->InsertItem(columnIndex++, employer.Name);
            pListCtrl->SetItemPtrData(listIndex, static_cast<wxUIntPtr>(employer.EmployerId));
            columnIndex = 0;
        }
    }

    pOkButton->Enable();
    pCancelButton->Enable();
}
} // namespace tks::UI::dlg
