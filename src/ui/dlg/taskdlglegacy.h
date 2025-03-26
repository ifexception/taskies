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
class Configuration;
} // namespace Core
namespace UI::dlg
{
/*
 * NOTE this dialog is deprecated
 * No further enhancements nor bug fixes will be provided
 */
class TaskDialogLegacy final : public wxDialog
{
public:
    TaskDialogLegacy() = delete;
    TaskDialogLegacy(const TaskDialogLegacy&) = delete;
    TaskDialogLegacy(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t taskId = -1,
        const std::string& selectedDate = "",
        const wxString& name = "taskdlglegacy");
    virtual ~TaskDialogLegacy() = default;

    TaskDialogLegacy& operator=(const TaskDialogLegacy&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnEmployerChoiceSelection(wxCommandEvent& event);
    void OnClientChoiceSelection(wxCommandEvent& event);
    void OnProjectChoiceSelection(wxCommandEvent& event);
    void OnShowProjectAssociatedCategoriesCheck(wxCommandEvent& event);
    void OnCategoryChoiceSelection(wxCommandEvent& event);
    void OnDateChange(wxDateEvent& event);
    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool TransferDataAndValidate();

    void ResetClientChoiceControl(bool disable = false);
    void ResetProjectChoiceControl(bool disable = false);
    void ResetCategoryChoiceControl(bool disable = false);
    void QueueErrorNotificationEventToParent(const std::string& message);

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;
    wxDatePickerCtrl* pDateContextCtrl;
    wxChoice* pEmployerChoiceCtrl;
    wxChoice* pClientChoiceCtrl;
    wxChoice* pProjectChoiceCtrl;
    wxCheckBox* pShowProjectAssociatedCategoriesCheckBoxCtrl;
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
    std::string mOldDate;
    int mEmployerIndex;

    enum {
        tksIDC_DATECONTEXT = wxID_HIGHEST + 100,
        tksIDC_EMPLOYERCHOICE,
        tksIDC_CLIENTCHOICE,
        tksIDC_PROJECTCHOICE,
        tksIDC_SHOWASSOCIATEDCATEGORIES,
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
