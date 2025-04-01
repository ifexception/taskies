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
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/clrpicker.h>

#include <spdlog/spdlog.h>

#include "../../models/categorymodel.h"

namespace tks::UI::dlg
{
class CategoryDialog final : public wxDialog
{
public:
    CategoryDialog() = delete;
    CategoryDialog(const CategoryDialog&) = delete;
    CategoryDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        std::int64_t categoryId,
        const wxString& name = "categorydlg");
    virtual ~CategoryDialog() = default;

    CategoryDialog& operator=(const CategoryDialog&) = delete;

private:
    void Initialize();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnIsActiveCheck(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();

    void TransferDataFromControls();

    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxTextCtrl* pNameTextCtrl;

    wxColourPickerCtrl* pColorPickerCtrl;
    wxCheckBox* pBillableCheckBoxCtrl;

    wxTextCtrl* pDescriptionTextCtrl;

    wxChoice* pProjectChoiceCtrl;

    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    std::int64_t mCategoryId;
    Model::CategoryModel mCategoryModel;

    enum {
        tksIDC_NAMETEXTCTRL = wxID_HIGHEST + 1001,
        tksIDC_COLORPICKERCTRL,
        tksIDC_BILLABLECHECKBOXCTRL,
        tksIDC_PROJECTCHOICECTRL,
        tksIDC_DESCRIPTIONTEXTCTRL,
        tksIDC_ISACTIVECHECKBOXCTRL
    };
};
} // namespace tks::UI::dlg
