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

#pragma once

#include <cstdint>
#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/listctrl.h>
#include <wx/clrpicker.h>

#include <spdlog/spdlog.h>

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core

namespace UI::dlg
{
class CategoriesDialog final : public wxDialog
{
public:
    CategoriesDialog() = delete;
    CategoriesDialog(const CategoriesDialog&&) = delete;
    CategoriesDialog(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        const wxString& name = "categoriesdlg");
    virtual ~CategoriesDialog() = default;

    CategoriesDialog& operator=(const CategoriesDialog&&) = delete;

private:
    void Initialize();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();

    // void AppendListControlEntry(Model::CategoryModel category);
    // void UpdateListControlEntry(Model::CategoryModel category);

    void OnClear(wxCommandEvent& event);
    void OnAdd(wxCommandEvent& event);
    void OnEdit(wxCommandEvent& event);
    void OnRemove(wxCommandEvent& event);
    void OnRemoveAll(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnItemChecked(wxListEvent& event);
    void OnItemUnchecked(wxListEvent& event);
    void OnItemRightClick(wxListEvent& event);

    void ResetControlValues();

    bool TransferDataAndValidate();

    wxWindow* pParent;
    wxTextCtrl* pNameTextCtrl;
    wxTextCtrl* pDescriptionTextCtrl;
    wxColourPickerCtrl* pColorPickerCtrl;
    wxCheckBox* pBillableCtrl;
    wxListCtrl* pListCtrl;
    wxButton* pClearButton;
    wxButton* pAddButton;
    wxButton* pRemoveButton;
    wxButton* pRemoveAllButton;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    enum { IDC_NAME = wxID_HIGHEST + 1, IDC_DESCRIPTION, IDC_COLORPICKER, IDC_BILLABLE, IDC_LIST };
};
} // namespace UI::dlg
} // namespace tks
