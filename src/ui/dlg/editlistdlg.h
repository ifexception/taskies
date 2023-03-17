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

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/listctrl.h>

#include <spdlog/spdlog.h>

namespace tks::UI
{
class EditListDialog final : public wxDialog
{
public:
    EditListDialog(wxWindow* parent, std::shared_ptr<spdlog::logger> logger, const wxString& name = "editlistdlg");
    virtual ~EditListDialog() = default;

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnSearchTextChange(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    void OnItemSelected(wxCommandEvent& event);
    void OnItemDoubleClick(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    std::shared_ptr<spdlog::logger> pLogger;
    wxWindow* pParent;
    wxTextCtrl* pSearchTextCtrl;
    wxButton* pSearchButton;
    wxListCtrl* pListCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    enum { IDC_LIST = wxID_HIGHEST + 100, IDC_SEARCHTEXT, IDC_SEARCHBTN };
};
} // namespace tks::UI
