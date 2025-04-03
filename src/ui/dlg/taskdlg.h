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
#include <optional>
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
class Configuration;
} // namespace Core
namespace UI::dlg
{
class TaskDialog final : public wxDialog
{
public:
    TaskDialog() = delete;
    TaskDialog(const TaskDialog&) = delete;
    TaskDialog(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t taskId = -1,
        const std::string& selectedDate = "",
        const wxString& name = "taskdlg");
    virtual ~TaskDialog() = default;

    TaskDialog& operator=(const TaskDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    void OnDateChange(wxDateEvent& event);
    void OnEmployerChoiceSelection(wxCommandEvent& event);

    void OnAttributeGroupChoiceSelection(wxCommandEvent& event);

    void OnClientChoiceSelection(wxCommandEvent& event);
    void OnProjectChoiceSelection(wxCommandEvent& event);
    void OnShowProjectAssociatedCategoriesCheck(wxCommandEvent& event);
    void OnCategoryChoiceSelection(wxCommandEvent& event);

    void OnIsActiveCheck(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    bool Validate();
    void TransferDataFromControls();

    void ResetClientChoiceControl(bool disable = false);
    void ResetProjectChoiceControl(bool disable = false);
    void ResetCategoryChoiceControl(bool disable = false);

    void FetchClientEntitiesByEmployer(const std::int64_t employerId);
    void FetchProjectEntitiesByEmployerOrClient(const std::optional<std::int64_t> employerId,
        const std::optional<std::int64_t> clientId);
    void FetchCategoryEntities(const std::optional<std::int64_t> projectId);

    void QueueErrorNotificationEvent(const std::string& message);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;

    wxDatePickerCtrl* pDateContextDatePickerCtrl;
    wxChoice* pEmployerChoiceCtrl;

    wxSpinCtrl* pTimeHoursSpinCtrl;
    wxSpinCtrl* pTimeMinutesSpinCtrl;

    wxCheckBox* pBillableCheckBoxCtrl;
    wxTextCtrl* pUniqueIdentiferTextCtrl;
    wxChoice* pAttributeGroupChoiceCtrl;
    wxButton* pManageAttributesButton;

    wxChoice* pClientChoiceCtrl;
    wxChoice* pProjectChoiceCtrl;
    wxCheckBox* pShowProjectAssociatedCategoriesCheckBoxCtrl;
    wxChoice* pCategoryChoiceCtrl;

    wxCheckBox* pIsActiveCheckBoxCtrl;

    wxTextCtrl* pTaskDescriptionTextCtrl;

    wxButton* pOkButton;
    wxButton* pCancelButton;

    std::string mDatabaseFilePath;
    bool bIsEdit;
    std::int64_t mTaskId;
    std::string mDate;
    std::string mOldDate;
    std::int64_t mEmployerId;
    std::int64_t mAttributeGroupId;

    Model::TaskModel mTaskModel;

    enum {
        tksIDC_DATECONTEXTDATEPICKERCTRL = wxID_HIGHEST + 100,
        tksIDC_EMPLOYERCHOICECTRL,
        tksIDC_CLIENTCHOICECTRL,
        tksIDC_PROJECTCHOICECTRL,
        tksIDC_SHOWPROJECTASSOCIATEDCATEGORIESCHECKBOXCTRL,
        tksIDC_CATEGORYCHOICECTRL,
        tksIDC_BILLABLECHECKBOXCTRL,
        tksIDC_UNIQUEIDENTIFERTEXTCTRL,
        tksIDC_ATTRIBUTEGROUPCHOICECTRL,
        tksIDC_MANAGEATTRIBUTESBUTTON,
        tksIDC_TIMEHOURSSPINCTRL,
        tksIDC_TIMEMINUTESSPINCTRL,
        tksIDC_TASKDESCRIPTIONTEXTCTRL,
        tksIDC_ISACTIVECHECKBOXCTRL
    };
};
} // namespace UI::dlg
} // namespace tks
