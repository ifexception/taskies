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
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/listctrl.h>

#include <spdlog/spdlog.h>

#include "../../common/enums.h"

#include "../../data/employerdata.h"
#include "../../data/clientdata.h"
#include "../../data/projectdata.h"
#include "../../data/categorydata.h"

namespace tks
{
namespace UI::dlg
{
class EditListDialog final : public wxDialog
{
public:
    EditListDialog() = delete;
    EditListDialog(const EditListDialog&) = delete;
    EditListDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        EditListEntityType editListEntityType,
        const wxString& name = "editlistdlg");
    virtual ~EditListDialog() = default;

    EditListDialog& operator=(const EditListDialog&) = delete;

private:
    std::string GetEditTitle();
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void DataToControls();

    void EmployerDataToControls();
    void ClientDataToControls();
    void ProjectDataToControls();
    void CategoryDataToControls();

    void OnSearchTextChange(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    // void OnSearchEnterKeyPressed(wxKeyEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnItemSelected(wxListEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void Search();
    void SearchEmployers();
    void SearchClients();
    void SearchProjects();
    void SearchCategories();

    std::string GetSearchHintText();

    std::shared_ptr<spdlog::logger> pLogger;

    std::string mDatabaseFilePath;
    EditListEntityType mType;

    wxWindow* pParent;
    wxTextCtrl* pSearchTextCtrl;
    wxBitmapButton* pSearchButton;
    wxBitmapButton* pResetButton;
    wxListCtrl* pListCtrl;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mSearchTerm;
    std::int64_t mEntityId;

    enum { IDC_LIST = wxID_HIGHEST + 100, IDC_SEARCHTEXT, IDC_SEARCHBTN, IDC_RESETBTN };
};
} // namespace UI::dlg
} // namespace tks
