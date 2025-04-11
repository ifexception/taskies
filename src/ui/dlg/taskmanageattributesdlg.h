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

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "../../common/enums.h"

#include "../../models/attributemodel.h"
#include "../../models/taskattributevaluemodel.h"

namespace tks::UI::dlg
{
class TaskManageAttributesDialog final : public wxDialog
{
public:
    TaskManageAttributesDialog() = delete;
    TaskManageAttributesDialog(const TaskManageAttributesDialog&) = delete;
    TaskManageAttributesDialog(wxWindow* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        std::int64_t attributeGroupId,
        bool isEdit = false,
        std::int64_t taskId = -1,
        const wxString& name = "taskmanageattributedlg");
    virtual ~TaskManageAttributesDialog() = default;

    TaskManageAttributesDialog& operator=(const TaskManageAttributesDialog&) = delete;

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

    void AppendAttributeControl(const Model::AttributeModel& model);

    void QueueErrorNotificationEvent(const std::string& message);

    wxWindow* pParent;

    wxSizer* pMainSizer;

    wxTextCtrl* pAttributeGroupNameTextCtrl;

    wxStaticBox* pAttributesBox;
    wxStaticBoxSizer* pAttributesBoxSizer;
    wxFlexGridSizer* pAttributesControlFlexGridSizer;

    wxButton* pOKButton;
    wxButton* pCancelButton;

    std::shared_ptr<spdlog::logger> pLogger;

    std::string mDatabaseFilePath;
    std::int64_t mAttributeGroupId;
    bool bIsEdit;
    std::int64_t mTaskId;
    int mAttributeControlCounter;

    struct AttributeControlData {
        int ControlId;
        AttributeTypes AttributeType;
        bool IsRequired;
        std::string Name;

        wxTextCtrl* TextControl;
        wxCheckBox* BooleanControl;
        wxSpinCtrl* NumericControl;

        std::int64_t AttributeId;

        AttributeControlData()
            : ControlId(-1)
            , AttributeType()
            , IsRequired(false)
            , Name()
            , TextControl(nullptr)
            , BooleanControl(nullptr)
            , NumericControl(nullptr)
            , AttributeId(-1)
        {
        }
    };

    std::vector<AttributeControlData> mAttributeControls;
    std::vector<Model::TaskAttributeValueModel> mTaskAttributeValueModels;

    enum { tksIDC_ATTRIBUTECONTROLBASE = wxID_HIGHEST + 1001 };
};
} // namespace tks::UI::dlg
