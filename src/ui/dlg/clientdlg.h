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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <spdlog/spdlog.h>

#include "../../models/clientmodel.h"

namespace tks::UI::dlg
{
class ClientDialog : public wxDialog
{
public:
    ClientDialog() = delete;
    ClientDialog(const ClientDialog&) = delete;
    ClientDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std ::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t clientId = -1,
        const wxString& name = "clientdlg");
    virtual ~ClientDialog() = default;

    ClientDialog& operator=(const ClientDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnIsActiveCheck(wxCommandEvent& event);

    bool Validate();
    void TransferDataFromControls();

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxTextCtrl* pNameTextCtrl;

    wxTextCtrl* pDescriptionTextCtrl;

    wxChoice* pEmployerChoiceCtrl;

    wxTextCtrl* pDateCreatedReadonlyTextCtrl;
    wxTextCtrl* pDateModifiedReadonlyTextCtrl;
    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;

    bool bIsEdit;
    std::int64_t mClientId;
    Model::ClientModel mClientModel;

    enum {
        tksIDC_NAMETEXTCTRL = wxID_HIGHEST + 1001,
        tksIDC_DESCRIPTIONTEXTCTRL,
        tksIDC_EMPLOYERCHOICECTRL,
        tksIDC_ISACTIVECHECKBOXCTRL,
    };
};
} // namespace tks::UI::dlg
