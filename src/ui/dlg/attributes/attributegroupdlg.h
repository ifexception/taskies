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

#include <spdlog/logger.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../../models/attributegroupmodel.h"

namespace tks::UI::dlg
{
class AttributeGroupDialog : public wxDialog
{
public:
    AttributeGroupDialog() = delete;
    AttributeGroupDialog(const AttributeGroupDialog&) = delete;
    AttributeGroupDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t attributeGroupId = -1,
        const wxString& name = "attributegroupdlg");
    virtual ~AttributeGroupDialog() = default;

    AttributeGroupDialog& operator=(const AttributeGroupDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnIsActiveCheck(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();
    void TransferDataFromControls();

    void DisableControlsIfUsed();
    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxTextCtrl* pNameTextCtrl;
    wxCheckBox* pIsStaticGroupCheckBoxCtrl;

    wxTextCtrl* pDescriptionTextCtrl;

    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    std::int64_t mAttributeGroupId;
    bool bIsEdit;

    Model::AttributeGroupModel mAttributeGroupModel;

    enum {
        tksIDC_NAMETEXTCTRL = wxID_HIGHEST + 1001,
        tksIDC_ISSTATICGROUPCHECKBOXCTRL,
        tksIDC_DESCRIPTIONTEXTCTRL,
        tksIDC_ISACTIVECHECKBOXCTRL
    };
};
} // namespace tks::UI::dlg
