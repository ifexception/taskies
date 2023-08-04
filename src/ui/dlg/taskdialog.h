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
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/spinctrl.h>

#include <spdlog/logger.h>

#include "../../models/taskmodel.h"

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core
namespace UI::dlg
{
class TaskDialog final : public wxDialog
{
public:
    TaskDialog() = delete;
    TaskDialog(const TaskDialog&) = delete;
    TaskDialog(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t taskId = -1,
        const wxString& name = "taskdlg");
    virtual ~TaskDialog() = default;

    TaskDialog& operator=(const TaskDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnEmployerChoiceSelection(wxCommandEvent& event);
    void OnClientChoiceSelection(wxCommandEvent& event);
    void OnCategoryChoiceSelection(wxCommandEvent& event);
    void OnDateChange(wxDateEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool TransferDataAndValidate();

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;
    wxDatePickerCtrl* pDateContextCtrl;
    wxChoice* pEmployerChoiceCtrl;
    wxChoice* pClientChoiceCtrl;
    wxChoice* pProjectChoiceCtrl;
    wxChoice* pCategoryChoiceCtrl;
    wxCheckBox* pBillableCheckBoxCtrl;
    wxTextCtrl* pUniqueIdentiferTextCtrl;
    wxSpinCtrl* pTimeHoursCtrl;
    wxSpinCtrl* pTimeMinutesCtrl;
    wxTextCtrl* pTaskDescriptionTextCtrl;
    wxTextCtrl* pDateCreatedTextCtrl;
    wxTextCtrl* pDateModifiedTextCtrl;
    wxCheckBox* pIsActiveCtrl;
    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    bool bIsEdit;
    Model::TaskModel mTaskModel;
    std::int64_t mTaskId;
    std::string mDate;
    int mEmployerIndex;

    enum {
        tksIDC_DATECONTEXT = wxID_HIGHEST + 100,
        tksIDC_EMPLOYERCHOICE,
        tksIDC_CLIENTCHOICE,
        tksIDC_PROJECTCHOICE,
        tksIDC_CATEGORYCHOICE,
        tksIDC_BILLABLE,
        tksIDC_UNIQUEIDENTIFIER,
        tksIDC_DURATIONHOURS,
        tksIDC_DURATIONMINUTES,
        tksIDC_DESCRIPTION,
        tksIDC_ISACTIVE
    };
};
} // namespace UI::dlg
} // namespace tks
