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
    , pClearButton(nullptr)
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

void CategoriesDialog::Initialize() {}

void CategoriesDialog::CreateControls() {}

void CategoriesDialog::FillControls() {}

void CategoriesDialog::ConfigureEventBindings() {}

void CategoriesDialog::OnClear(wxCommandEvent& event) {}

void CategoriesDialog::OnAdd(wxCommandEvent& event) {}

void CategoriesDialog::OnEdit(wxCommandEvent& event) {}

void CategoriesDialog::OnRemove(wxCommandEvent& event) {}

void CategoriesDialog::OnRemoveAll(wxCommandEvent& event) {}

void CategoriesDialog::OnOK(wxCommandEvent& event) {}

void CategoriesDialog::OnCancel(wxCommandEvent& event) {}

void CategoriesDialog::OnItemChecked(wxListEvent& event) {}

void CategoriesDialog::OnItemUnchecked(wxListEvent& event) {}

void CategoriesDialog::OnItemRightClick(wxListEvent& event) {}

void CategoriesDialog::ResetControlValues() {}

bool CategoriesDialog::TransferDataAndValidate()
{
    return false;
}
} // namespace tks::UI::dlg
