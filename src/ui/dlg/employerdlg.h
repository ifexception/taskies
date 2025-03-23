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

#include "../../models/employermodel.h"

namespace tks::UI::dlg
{
class EmployerDialog final : public wxDialog
{
public:
    EmployerDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t employerId = -1,
        const wxString& name = "employerdlg");
    virtual ~EmployerDialog() = default;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnIsActiveCheck(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();

    void TransferDataFromControls();

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    wxWindow* pParent;

    wxTextCtrl* pNameTextCtrl;
    wxCheckBox* pIsDefaultCheckBoxCtrl;

    wxTextCtrl* pDescriptionTextCtrl;

    wxTextCtrl* pDateCreatedReadonlyTextCtrl;
    wxTextCtrl* pDateModifiedReadonlyTextCtrl;
    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    bool bIsEdit;
    std::int64_t mEmployerId;

    Model::EmployerModel mEmployerModel;

    enum {
        tksIDC_NAME = wxID_HIGHEST + 1001,
        tksIDC_ISDEFAULT,
        tksIDC_DESCRIPTION,
        tksIDC_ISACTIVECHECKBOXCTRL,
    };
};
} // namespace tks::UI::dlg
