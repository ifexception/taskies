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

namespace tks::UI::dlg
{
class StaticAttributeValuesDialog final : public wxDialog
{
public:
    StaticAttributeValuesDialog() = delete;
    StaticAttributeValuesDialog(const StaticAttributeValuesDialog&) = delete;
    StaticAttributeValuesDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t attributeGroupId = -1,
        const wxString& name = "staticattributevaluesdlg");
    virtual ~StaticAttributeValuesDialog() = default;

    StaticAttributeValuesDialog& operator=(const StaticAttributeValuesDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();
    void TransferDataFromControls();

    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxSizer* pMainSizer;

    wxChoice* pAttributeGroupChoiceCtrl;

    wxStaticBox* pAttributesBox;
    wxStaticBoxSizer* pAttributesBoxSizer;
    wxFlexGridSizer* pAttributesControlFlexGridSizer;

    wxButton* pOKButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    std::int64_t mAttributeGroupId;
    bool bIsEdit;

    enum { tksIDC_ATTRIBUTEGROUPCHOICECTRL = wxID_HIGHEST + 1001, tksIDC_ATTRIBUTECONTROLBASE };
};
} // namespace tks::UI::dlg
