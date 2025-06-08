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

#include "../../../models/attributemodel.h"

namespace tks
{
namespace Persistence
{
struct AttributesPersistence;
}
namespace UI::dlg
{
class AttributeDialog final : public wxDialog
{
public:
    AttributeDialog() = delete;
    AttributeDialog(const AttributeDialog&) = delete;
    AttributeDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t attributeId = -1,
        const wxString& name = "attributedlg");
    virtual ~AttributeDialog() = default;

    AttributeDialog& operator=(const AttributeDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnAttributeGroupSelection(wxCommandEvent& event);

    void OnIsActiveCheck(wxCommandEvent& event);
    void OnAddAnotherCheck(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();
    void TransferDataFromControls();

    bool CheckAttributeUsage(Persistence::AttributesPersistence& attributesPersistence);
    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxTextCtrl* pNameTextCtrl;
    wxCheckBox* pIsRequiredCheckBoxCtrl;

    wxTextCtrl* pDescriptionTextCtrl;

    wxChoice* pAttributeGroupChoiceCtrl;
    wxChoice* pAttributeTypeChoiceCtrl;

    wxCheckBox* pIsStaticCheckBoxCtrl;

    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxCheckBox* pAddAnotherCheckBoxCtrl;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    std::int64_t mAttributeId;
    bool bIsEdit;

    Model::AttributeModel mAttributeModel;
    bool bAddAnotherAttribute;

    std::int64_t mOriginalAttributeTypeIdEdit;

    enum {
        tksIDC_NAMETEXTCTRL = wxID_HIGHEST + 1001,
        tksIDC_DESCRIPTIONTEXTCTRL,
        tksIDC_ISREQUIREDCHECKBOXCTRL,
        tksIDC_ATTRIBUTEGROUPCHOICECTRL,
        tksIDC_ATTRIBUTETYPECHOICECTRL,
        tksIDC_ISSTATICCHECKBOXCTRL,
        tksIDC_ISACTIVECHECKBOXCTRL,
        tksIDC_ADDANOTHERCHECKBOXCTRL
    };
};
} // namespace UI::dlg
} // namespace tks
