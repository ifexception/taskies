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

#include <spdlog/spdlog.h>

#include "../../models/employermodel.h"
#include "../../data/employerdata.h"

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core

namespace UI::dlg
{
class EmployerDialog final : public wxDialog
{
public:
    EmployerDialog(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        bool isEdit = false,
        std::int64_t employerId = -1,
        const wxString& name = "employerdlg");
    virtual ~EmployerDialog() = default;

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnIsActiveCheck(wxCommandEvent& event);

    bool TransferDataAndValidate();

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;
    wxTextCtrl* pNameTextCtrl;
    wxTextCtrl* pDescriptionTextCtrl;
    wxCheckBox* pIsActiveCtrl;
    wxTextCtrl* pDateCreatedTextCtrl;
    wxTextCtrl* pDateModifiedTextCtrl;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    bool bIsEdit;
    std::int64_t mEmployerId;
    Model::EmployerModel mEmployer;
    Data::EmployerData mData;

    enum {
        IDC_NAME = wxID_HIGHEST + 1,
        IDC_DESCRIPTION,
        IDC_ISACTIVE,
    };
};
} // namespace UI::dlg
} // namespace tks
