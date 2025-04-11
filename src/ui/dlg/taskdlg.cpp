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

#include "taskdlg.h"

#include <date/date.h>
#include <fmt/format.h>

#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "taskmanageattributesdlg.h"

#include "../clientdata.h"
#include "../events.h"
#include "../notificationclientdata.h"
#include "../common/taskattributevalueclientdata.h"

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/configuration.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"
#include "../../models/categorymodel.h"
#include "../../models/attributegroupmodel.h"

#include "../../persistence/employerspersistence.h"
#include "../../persistence/ClientsPersistence.h"
#include "../../persistence/projectspersistence.h"
#include "../../persistence/categoriespersistence.h"
#include "../../persistence/workdayspersistence.h"
#include "../../persistence/taskspersistence.h"
#include "../../persistence/attributegroupspersistence.h"
#include "../../persistence/taskattributevaluespersistence.h"

#include "../../repository/categoryrepositorymodel.h"

#include "../../repository/categoryrepository.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
TaskDialog::TaskDialog(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    std::int64_t taskId,
    const std::string& selectedDate,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Task" : "New Task",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsEdit(isEdit)
    , mTaskId(taskId)
    , pDateContextDatePickerCtrl(nullptr)
    , pTimeHoursSpinCtrl(nullptr)
    , pTimeMinutesSpinCtrl(nullptr)
    , pBillableCheckBoxCtrl(nullptr)
    , pUniqueIdentiferTextCtrl(nullptr)
    , pAttributeGroupChoiceCtrl(nullptr)
    , pManageAttributesButton(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pClientChoiceCtrl(nullptr)
    , pProjectChoiceCtrl(nullptr)
    , pShowProjectAssociatedCategoriesCheckBoxCtrl(nullptr)
    , pCategoryChoiceCtrl(nullptr)
    , pIsActiveCheckBoxCtrl(nullptr)
    , pTaskDescriptionTextCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , mDate()
    , mOldDate()
    , mEmployerId(-1)
    , mAttributeGroupId(-1)
    , mTaskModel()
    , mTaskAttributeValueModels()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!selectedDate.empty()) {
        mDate = selectedDate;
    } else {
        auto todaysDate = date::floor<date::days>(std::chrono::system_clock::now());
        mDate = date::format("%F", todaysDate);
    }

    mOldDate = mDate;

    Create();

    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(420, 440)));
    }

    wxIconBundle iconBundle(tks::Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void TaskDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();

    if (bIsEdit) {
        DataToControls();
    }
}

void TaskDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Left and Right Sizer for choice and configurations */
    auto layoutSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(layoutSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(leftSizer, wxSizerFlags().Expand().Proportion(1));

    /* Begin of Left Aligned Controls */

    /* Defaults Box */
    auto defaultsStaticBox = new wxStaticBox(this, wxID_ANY, "Defaults");
    auto defaultsStaticBoxSizer = new wxStaticBoxSizer(defaultsStaticBox, wxVERTICAL);
    leftSizer->Add(defaultsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Date Label */
    auto dateLabel = new wxStaticText(defaultsStaticBox, wxID_ANY, "Date");

    /* Date Control */
    pDateContextDatePickerCtrl =
        new wxDatePickerCtrl(defaultsStaticBox, tksIDC_DATECONTEXTDATEPICKERCTRL);

    /* Employer Choice Controls */
    auto employerLabel = new wxStaticText(defaultsStaticBox, wxID_ANY, "Employer");
    pEmployerChoiceCtrl = new wxChoice(defaultsStaticBox, tksIDC_EMPLOYERCHOICECTRL);
    pEmployerChoiceCtrl->SetToolTip("Select employer to associate task with");

    auto defaultsFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(8));
    defaultsFlexGridSizer->AddGrowableCol(1, 1);

    defaultsFlexGridSizer->Add(
        dateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    defaultsFlexGridSizer->Add(
        pDateContextDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    defaultsFlexGridSizer->Add(
        employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    defaultsFlexGridSizer->Add(
        pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    defaultsStaticBoxSizer->Add(
        defaultsFlexGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Time static box */
    auto timeStaticBox = new wxStaticBox(this, wxID_ANY, "Time");
    auto timeStaticBoxSizer = new wxStaticBoxSizer(timeStaticBox, wxVERTICAL);
    leftSizer->Add(timeStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Time Controls */
    auto timeLabel = new wxStaticText(timeStaticBox, wxID_ANY, "Time (H : M)");

    /* Hours spin control */
    pTimeHoursSpinCtrl = new wxSpinCtrl(timeStaticBox,
        tksIDC_TIMEHOURSSPINCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        16);
    pTimeHoursSpinCtrl->SetToolTip("Number of hours the task took");

    /* Minutes spin control */
    pTimeMinutesSpinCtrl = new wxSpinCtrl(timeStaticBox,
        tksIDC_TIMEMINUTESSPINCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        59);
    pTimeMinutesSpinCtrl->SetToolTip("Number of minutes the task took");
    pTimeMinutesSpinCtrl->SetValue(pCfg->GetMinutesIncrement());
    pTimeMinutesSpinCtrl->SetIncrement(pCfg->GetMinutesIncrement());

    auto timeSizer = new wxBoxSizer(wxHORIZONTAL);
    timeSizer->Add(timeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    timeSizer->AddStretchSpacer(1);
    timeSizer->Add(pTimeHoursSpinCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    timeSizer->Add(pTimeMinutesSpinCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    timeStaticBoxSizer->Add(timeSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Task Attributes box */
    auto taskAttributesStaticBox = new wxStaticBox(this, wxID_ANY, "Attributes");
    auto taskAttributesStaticBoxSizer = new wxStaticBoxSizer(taskAttributesStaticBox, wxVERTICAL);
    leftSizer->Add(taskAttributesStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Billable Check Box Control */
    pBillableCheckBoxCtrl =
        new wxCheckBox(taskAttributesStaticBox, tksIDC_BILLABLECHECKBOXCTRL, "Billable");
    pBillableCheckBoxCtrl->SetToolTip("Indicates if a task is billable");

    /* Unique Identifier Text Control */
    auto uniqueIdLabel = new wxStaticText(taskAttributesStaticBox, wxID_ANY, "Unique ID");
    pUniqueIdentiferTextCtrl =
        new wxTextCtrl(taskAttributesStaticBox, tksIDC_UNIQUEIDENTIFERTEXTCTRL);
    pUniqueIdentiferTextCtrl->SetHint("Unique identifier");
    pUniqueIdentiferTextCtrl->SetToolTip("Enter a unique identifier, ticket number, or other "
                                         "identifier to associate a task with");

    /* Task Attributes control flex grid sizer */
    auto taskAttributesControlFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(6));
    taskAttributesControlFlexGridSizer->AddGrowableCol(1, 1);
    taskAttributesStaticBoxSizer->Add(
        taskAttributesControlFlexGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    taskAttributesControlFlexGridSizer->Add(0, 0);
    taskAttributesControlFlexGridSizer->Add(
        pBillableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    taskAttributesControlFlexGridSizer->Add(
        uniqueIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CentreVertical());
    taskAttributesControlFlexGridSizer->Add(
        pUniqueIdentiferTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Attrbute Group choice control */
    auto attributeGroupLabel =
        new wxStaticText(taskAttributesStaticBox, wxID_ANY, "Attribute Group");
    pAttributeGroupChoiceCtrl =
        new wxChoice(taskAttributesStaticBox, tksIDC_ATTRIBUTEGROUPCHOICECTRL);

    taskAttributesStaticBoxSizer->Add(
        attributeGroupLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    taskAttributesStaticBoxSizer->Add(
        pAttributeGroupChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Manage attributes button */
    pManageAttributesButton = new wxButton(
        taskAttributesStaticBox, tksIDC_MANAGEATTRIBUTESBUTTON, "Manage Attributes...");
    pManageAttributesButton->SetToolTip("Manage attributes to associate with task");
    pManageAttributesButton->Disable();

    auto manageAttributesHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    taskAttributesStaticBoxSizer->Add(manageAttributesHorizontalSizer, wxSizerFlags().Expand());

    manageAttributesHorizontalSizer->AddStretchSpacer(1);
    manageAttributesHorizontalSizer->Add(
        pManageAttributesButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    // CHOICES

    /* Client choice control */
    auto clientLabel = new wxStaticText(this, wxID_ANY, "Client");
    pClientChoiceCtrl = new wxChoice(this, tksIDC_CLIENTCHOICECTRL);
    pClientChoiceCtrl->SetToolTip("Select client to associate task with");

    /* Project choice control */
    auto projectLabel = new wxStaticText(this, wxID_ANY, "Project");
    pProjectChoiceCtrl = new wxChoice(this, tksIDC_PROJECTCHOICECTRL);
    pProjectChoiceCtrl->SetToolTip("Select project to associate task with");

    /* Associated categories control */
    pShowProjectAssociatedCategoriesCheckBoxCtrl = new wxCheckBox(this,
        tksIDC_SHOWPROJECTASSOCIATEDCATEGORIESCHECKBOXCTRL,
        "Only show associated categories");
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetToolTip(
        "Only show categories associated to selected project");

    /* Category choice control*/
    auto categoryLabel = new wxStaticText(this, wxID_ANY, "Category");
    pCategoryChoiceCtrl = new wxChoice(this, tksIDC_CATEGORYCHOICECTRL);
    pCategoryChoiceCtrl->SetToolTip("Select project to associate task with");

    leftSizer->Add(clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->Add(0, 0);
    leftSizer->Add(
        pShowProjectAssociatedCategoriesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    leftSizer->Add(categoryLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pCategoryChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Is Active static box control */
    auto isActiveStaticBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto isActiveStaticBoxSizer = new wxStaticBoxSizer(isActiveStaticBox, wxHORIZONTAL);
    leftSizer->Add(isActiveStaticBoxSizer, wxSizerFlags().Expand());

    /* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(this, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    isActiveStaticBoxSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* End of Left Aligned Controls */

    /* Begin Center Aligned Controls */

    auto centerVerticalStaticLine =
        new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    layoutSizer->Add(centerVerticalStaticLine, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* End of Center Aligned Controls*/

    /* Begin Right Aligned Controls */

    auto rightSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

    /* Task Description Text Control */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Description");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);

    pTaskDescriptionTextCtrl = new wxTextCtrl(descriptionBox,
        tksIDC_TASKDESCRIPTIONTEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE);
    pTaskDescriptionTextCtrl->SetHint("Task description");
    pTaskDescriptionTextCtrl->SetToolTip("Enter the description of the task");

    descriptionBoxSizer->Add(
        pTaskDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    rightSizer->Add(
        descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* End of Right Aligned Controls */

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* Begin Button Controls */

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* End of Button Controls */

    SetSizerAndFit(sizer);
    sizer->SetSizeHints(this);
}

void TaskDialog::FillControls()
{
    auto bottomRangeDate = wxDateTime::GetCurrentYear() - 1;
    auto& bottomDateContext = wxDateTime::Now().SetYear(bottomRangeDate);
    pDateContextDatePickerCtrl->SetRange(bottomDateContext, wxDateTime::Now());

    wxDateTime dateTaskContext = wxDefaultDateTime;
    bool success = dateTaskContext.ParseDate(mDate);
    if (success) {
        pDateContextDatePickerCtrl->SetValue(dateTaskContext);
    } else {
        pLogger->error("TaskDialog::FillControls - wxDateTime failed to parse date \"{0}\". "
                       "Keep default date",
            mDate);
    }

    std::string defaultSearhTerm = "";

    pAttributeGroupChoiceCtrl->Append("Select attribute group", new ClientData<std::int64_t>(-1));
    pAttributeGroupChoiceCtrl->SetSelection(0);

    pEmployerChoiceCtrl->Append("Select employer", new ClientData<std::int64_t>(-1));
    pEmployerChoiceCtrl->SetSelection(0);

    ResetClientChoiceControl(true);

    ResetProjectChoiceControl(true);

    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetValue(pCfg->ShowProjectAssociatedCategories());

    ResetCategoryChoiceControl();

    // Fill controls with data

    // Fill Attribute Group choice control with data
    std::vector<Model::AttributeGroupModel> attributeGroups;
    Persistence::AttributeGroupsPersistence attributeGroupsPersistence(pLogger, mDatabaseFilePath);

    int rc = attributeGroupsPersistence.Filter(defaultSearhTerm, attributeGroups);
    if (rc != 0) {
        std::string message = "Failed to get employers";
        QueueErrorNotificationEvent(message);
    } else {
        for (const auto& attributeGroup : attributeGroups) {
            pAttributeGroupChoiceCtrl->Append(
                attributeGroup.Name, new ClientData<std::int64_t>(attributeGroup.AttributeGroupId));
        }
    }

    std::vector<Model::EmployerModel> employers;
    Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

    rc = employerPersistence.Filter(defaultSearhTerm, employers);
    if (rc != 0) {
        std::string message = "Failed to get employers";
        QueueErrorNotificationEvent(message);
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(
                employer.Name, new ClientData<std::int64_t>(employer.EmployerId));

            if (employer.IsDefault) {
                mEmployerId = employer.EmployerId;
                pEmployerChoiceCtrl->SetStringSelection(employer.Name);
            }
        }
    }

    std::vector<Model::ClientModel> clients;
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    rc = ClientsPersistence.FilterByEmployerId(mEmployerId, clients);

    if (rc != 0) {
        std::string message = "Failed to get clients";
        QueueErrorNotificationEvent(message);
    } else {
        if (!clients.empty()) {
            if (!pClientChoiceCtrl->IsEnabled()) {
                pClientChoiceCtrl->Enable();
            }

            for (auto& client : clients) {
                pClientChoiceCtrl->Append(
                    client.Name, new ClientData<std::int64_t>(client.ClientId));
            }
        } else {
            pClientChoiceCtrl->Disable();
        }
    }

    FetchProjectEntitiesByEmployerOrClient(
        std::make_optional<std::int64_t>(mEmployerId), std::nullopt);
}

// clang-format off
void TaskDialog::ConfigureEventBindings()
{
    pDateContextDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &TaskDialog::OnDateChange,
        this
    );

    pEmployerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnEmployerChoiceSelection,
        this
    );

    pAttributeGroupChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnAttributeGroupChoiceSelection,
        this
    );

    pManageAttributesButton->Bind(
        wxEVT_BUTTON,
        &TaskDialog::OnManageAttributes,
        this,
        tksIDC_MANAGEATTRIBUTESBUTTON
    );

    Bind(
        tksEVT_TASKDLGATTRIBUTESADDED,
        &TaskDialog::OnTaskAttributesAdded,
        this,
        wxID_ANY
    );

    pClientChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnClientChoiceSelection,
        this
    );

    pProjectChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnProjectChoiceSelection,
        this
    );

    pShowProjectAssociatedCategoriesCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &TaskDialog::OnShowProjectAssociatedCategoriesCheck,
        this
    );

    pCategoryChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnCategoryChoiceSelection,
        this
    );

    pIsActiveCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &TaskDialog::OnIsActiveCheck,
        this
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &TaskDialog::OnOK,
        this,
        wxID_OK
    );

    pCancelButton->Bind(
        wxEVT_BUTTON,
        &TaskDialog::OnCancel,
        this,
        wxID_CANCEL
    );
}
// clang-format on

void TaskDialog::DataToControls()
{
    // load task
    Model::TaskModel taskModel;
    Persistence::TasksPersistence taskPersistence(pLogger, mDatabaseFilePath);
    bool isSuccess = false;

    int ret = taskPersistence.GetById(mTaskId, taskModel);
    if (ret != 0) {
        std::string message = "Failed to get taskModel";
        QueueErrorNotificationEvent(message);
    } else {
        pBillableCheckBoxCtrl->SetValue(taskModel.Billable);
        pUniqueIdentiferTextCtrl->ChangeValue(
            taskModel.UniqueIdentifier.has_value() ? taskModel.UniqueIdentifier.value() : "");
        pTimeHoursSpinCtrl->SetValue(taskModel.Hours);
        pTimeMinutesSpinCtrl->SetValue(taskModel.Minutes);
        pTaskDescriptionTextCtrl->ChangeValue(taskModel.Description);
        pIsActiveCheckBoxCtrl->SetValue(taskModel.IsActive);

        pIsActiveCheckBoxCtrl->Enable();

        isSuccess = true;
    }

    // -- load related entities

    bool employerSelected = mEmployerId != -1;

    // load project
    Model::ProjectModel projectModel;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    ret = projectPersistence.GetById(taskModel.ProjectId, projectModel);
    if (ret != 0) {
        std::string message = "Failed to get project";
        QueueErrorNotificationEvent(message);

        return;
    }

    if (!employerSelected) {
        // load projects
        std::vector<Model::ProjectModel> projects;
        ret = projectPersistence.FilterByEmployerIdOrClientId(
            std::make_optional(projectModel.EmployerId),
            projectModel.ClientId.has_value() ? projectModel.ClientId : std::nullopt,
            projects);
        if (ret != 0) {
            std::string message = "Failed to get projects";
            QueueErrorNotificationEvent(message);
            isSuccess = false;
        } else {
            if (!projects.empty()) {
                if (!pProjectChoiceCtrl->IsEnabled()) {
                    pProjectChoiceCtrl->Enable();
                }

                for (auto& project : projects) {
                    pProjectChoiceCtrl->Append(
                        project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
                }
            }
        }
    }

    pProjectChoiceCtrl->SetStringSelection(projectModel.DisplayName);
    isSuccess = true;

    if (!employerSelected) {
        // load employer
        Model::EmployerModel employer;
        Persistence::EmployersPersistence employerPersistence(pLogger, mDatabaseFilePath);

        ret = employerPersistence.GetById(projectModel.EmployerId, employer);
        if (ret == -1) {
            std::string message = "Failed to get employer";
            QueueErrorNotificationEvent(message);

            isSuccess = false;
        } else {
            pEmployerChoiceCtrl->SetStringSelection(employer.Name);
            isSuccess = true;
        }
    }

    // load clients
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    if (!employerSelected) {
        std::vector<Model::ClientModel> clients;
        std::string defaultSearchTerm = "";
        ret = ClientsPersistence.FilterByEmployerId(projectModel.EmployerId, clients);
        if (ret == -1) {
            std::string message = "Failed to get clients";
            QueueErrorNotificationEvent(message);

            isSuccess = false;
        } else {
            // load client
            if (!clients.empty()) {
                for (const auto& client : clients) {
                    pClientChoiceCtrl->Append(
                        client.Name, new ClientData<std::int64_t>(client.ClientId));
                }

                if (projectModel.ClientId.has_value()) {
                    Model::ClientModel client;
                    ret = ClientsPersistence.GetById(projectModel.ClientId.value(), client);
                    if (ret == -1) {
                        std::string message = "Failed to get client";
                        QueueErrorNotificationEvent(message);

                        isSuccess = false;
                    } else {
                        pClientChoiceCtrl->SetStringSelection(client.Name);
                        isSuccess = true;
                    }
                }

                pClientChoiceCtrl->Enable();
            }
        }
    } else {
        if (projectModel.ClientId.has_value()) {
            Model::ClientModel client;
            ret = ClientsPersistence.GetById(projectModel.ClientId.value(), client);
            if (ret == -1) {
                std::string message = "Failed to get client";
                QueueErrorNotificationEvent(message);

                isSuccess = false;
            } else {
                pClientChoiceCtrl->SetStringSelection(client.Name);
                isSuccess = true;
            }
        }
    }

    // load categories
    ResetCategoryChoiceControl();

    repos::CategoryRepository categoryRepo(pLogger, mDatabaseFilePath);
    std::vector<repos::CategoryRepositoryModel> categories;

    if (pCfg->ShowProjectAssociatedCategories()) {
        ret = categoryRepo.FilterByProjectId(taskModel.ProjectId, categories);
    } else {
        ret = categoryRepo.Filter(categories);
    }

    if (ret == -1) {
        std::string message = "Failed to get categories";
        QueueErrorNotificationEvent(message);

        isSuccess = false;
    }

    if (!categories.empty()) {
        if (!pCategoryChoiceCtrl->IsEnabled()) {
            pCategoryChoiceCtrl->Enable();
        }

        for (auto& category : categories) {
            pCategoryChoiceCtrl->Append(
                category.GetFormattedName(), new ClientData<std::int64_t>(category.CategoryId));
        }

        repos::CategoryRepositoryModel category;
        ret = categoryRepo.GetById(taskModel.CategoryId, category);
        if (ret != 0) {
            std::string message = "Failed to get category";
            QueueErrorNotificationEvent(message);

            isSuccess = false;
        } else {
            pCategoryChoiceCtrl->SetStringSelection(category.GetFormattedName());
            isSuccess = true;
        }
    } else {
        ResetCategoryChoiceControl(true);
    }

    if (isSuccess) {
        pOkButton->Enable();
        pOkButton->SetFocus();
        pOkButton->SetDefault();
    }
}

void TaskDialog::OnDateChange(wxDateEvent& event)
{
    pLogger->info("TaskDialog::OnDateChange - Received date change event \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    // save old date in case further down the line we are editing a task and changing its date
    mOldDate = mDate;

    // get the newly selected date
    wxDateTime eventDate = wxDateTime(event.GetDate());
    auto& eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);
    auto dateTicks = eventDateUtc.GetTicks();

    auto date = date::floor<date::days>(std::chrono::system_clock::from_time_t(dateTicks));
    mDate = date::format("%F", date);
    pLogger->info("TaskDialog::OnDateChange - mDate is \"{0}\"", mDate);
}

void TaskDialog::OnEmployerChoiceSelection(wxCommandEvent& event)
{
    ResetClientChoiceControl();
    ResetProjectChoiceControl();
    ResetCategoryChoiceControl();

    int employerIndex = event.GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));

    if (employerIdData->GetValue() < 1) {
        pClientChoiceCtrl->Disable();
        pProjectChoiceCtrl->Disable();

        if (pCfg->ShowProjectAssociatedCategories()) {
            pCategoryChoiceCtrl->Disable();
        }

        mEmployerId = -1;

        return;
    }

    mEmployerId = employerIdData->GetValue();

    FetchClientEntitiesByEmployer(mEmployerId);

    FetchProjectEntitiesByEmployerOrClient(
        std::make_optional<std::int64_t>(mEmployerId), std::nullopt);
}

void TaskDialog::OnAttributeGroupChoiceSelection(wxCommandEvent& event)
{
    int index = event.GetSelection();
    ClientData<std::int64_t>* attributeGroupIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pAttributeGroupChoiceCtrl->GetClientObject(index));
    std::int64_t attributeGroupId = attributeGroupIdData->GetValue();

    if (attributeGroupId < 1) {
        pManageAttributesButton->Disable();
        return;
    }

    pManageAttributesButton->Enable();
    mAttributeGroupId = attributeGroupId;
}

void TaskDialog::OnManageAttributes(wxCommandEvent& WXUNUSED(event))
{
    const std::string TAG = "TaskDialog::OnManageAttributes";

    dlg::TaskManageAttributesDialog taskManageAttributes(
        this, pLogger, mDatabaseFilePath, mAttributeGroupId);
    int ret = taskManageAttributes.ShowModal();

    if (ret == wxID_OK) {
        pLogger->info("{0} - Manage Attribute dialog returned OK", TAG);
    }
}

void TaskDialog::OnTaskAttributesAdded(wxCommandEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger, "\"tksEVT_TASKDLGATTRIBUTESADDED\" event received");

    Common::TaskAttributeValueClientData* taskAttributeValueClientData =
        reinterpret_cast<Common::TaskAttributeValueClientData*>(event.GetClientObject());

    mTaskAttributeValueModels = taskAttributeValueClientData->TaskAttributeValueModels;

    SPDLOG_LOGGER_TRACE(pLogger,
        "Received \"{0}\" captured task attribute values",
        mTaskAttributeValueModels.size());

    if (taskAttributeValueClientData) {
        delete taskAttributeValueClientData;
    }
}

void TaskDialog::OnClientChoiceSelection(wxCommandEvent& event)
{
    ResetProjectChoiceControl();
    ResetCategoryChoiceControl();

    int clientIndex = event.GetSelection();
    ClientData<std::int64_t>* clientIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pClientChoiceCtrl->GetClientObject(clientIndex));
    std::int64_t clientId = clientIdData->GetValue();

    if (clientId < 1) {
        FetchProjectEntitiesByEmployerOrClient(
            std::make_optional<std::int64_t>(mEmployerId), std::nullopt);
    } else {
        FetchProjectEntitiesByEmployerOrClient(std::make_optional<std::int64_t>(mEmployerId),
            std::make_optional<std::int64_t>(clientId));
    }
}

void TaskDialog::OnProjectChoiceSelection(wxCommandEvent& event)
{
    ResetCategoryChoiceControl();

    int projectIndex = event.GetSelection();
    ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pProjectChoiceCtrl->GetClientObject(projectIndex));
    std::int64_t projectId = projectIdData->GetValue();

    if (projectId < 1) {
        pCategoryChoiceCtrl->Disable();

        return;
    }

    FetchCategoryEntities(std::make_optional<std::int64_t>(projectId));
}

void TaskDialog::OnShowProjectAssociatedCategoriesCheck(wxCommandEvent& event)
{
    ResetCategoryChoiceControl();

    std::optional<std::int64_t> projectId = std::nullopt;

    if (event.IsChecked() && mEmployerId > 0) {
        int projectIndex = pProjectChoiceCtrl->GetSelection();
        ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
            pProjectChoiceCtrl->GetClientObject(projectIndex));

        if (projectIdData->GetValue() < 1) {
            pCategoryChoiceCtrl->Disable();
            return;
        }

        projectId.reset();
        projectId = std::make_optional<std::int64_t>(projectIdData->GetValue());
    } else if (event.IsChecked() && mEmployerId < 1) {
        pCategoryChoiceCtrl->Disable();
        return;
    }

    FetchCategoryEntities(projectId);

    pCfg->ShowProjectAssociatedCategories(event.IsChecked());
    pCfg->Save();
}

void TaskDialog::OnCategoryChoiceSelection(wxCommandEvent& event)
{
    pBillableCheckBoxCtrl->SetValue(false);
    pBillableCheckBoxCtrl->SetToolTip("Indicates if a task is billable");

    int categoryIndex = event.GetSelection();
    ClientData<std::int64_t>* categoryIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pCategoryChoiceCtrl->GetClientObject(categoryIndex));
    std::int64_t categoryId = categoryIdData->GetValue();

    if (categoryId < 1) {
        return;
    }

    Model::CategoryModel model;
    Persistence::CategoriesPersistence categoryPersistence(pLogger, mDatabaseFilePath);
    int ret = 0;

    ret = categoryPersistence.GetById(categoryId, model);
    if (ret == -1) {
        std::string message = "Failed to get category";
        QueueErrorNotificationEvent(message);
    } else {
        if (model.Billable) {
            pBillableCheckBoxCtrl->SetValue(true);
            pBillableCheckBoxCtrl->SetToolTip("Task is billable since category is billable");
        }
    }
}

void TaskDialog::OnIsActiveCheck(wxCommandEvent& event)
{
    if (!event.IsChecked()) {
        pDateContextDatePickerCtrl->Disable();
        pEmployerChoiceCtrl->Disable();
        pClientChoiceCtrl->Disable();
        pProjectChoiceCtrl->Disable();
        pShowProjectAssociatedCategoriesCheckBoxCtrl->Disable();
        pCategoryChoiceCtrl->Disable();
        pBillableCheckBoxCtrl->Disable();
        pUniqueIdentiferTextCtrl->Disable();
        pTimeHoursSpinCtrl->Disable();
        pTimeMinutesSpinCtrl->Disable();
        pAttributeGroupChoiceCtrl->Disable();
        pManageAttributesButton->Disable();
        pTaskDescriptionTextCtrl->Disable();
    } else {
        pDateContextDatePickerCtrl->Enable();
        pEmployerChoiceCtrl->Enable();
        pClientChoiceCtrl->Enable();
        pProjectChoiceCtrl->Enable();
        pShowProjectAssociatedCategoriesCheckBoxCtrl->Enable();
        pCategoryChoiceCtrl->Enable();
        pBillableCheckBoxCtrl->Enable();
        pUniqueIdentiferTextCtrl->Enable();
        pTimeHoursSpinCtrl->Enable();
        pTimeMinutesSpinCtrl->Enable();
        pAttributeGroupChoiceCtrl->Enable();
        if (pAttributeGroupChoiceCtrl->GetSelection() > 0) {
            pManageAttributesButton->Enable();
        }
        pTaskDescriptionTextCtrl->Enable();
    }
}

void TaskDialog::OnOK(wxCommandEvent& event)
{
    if (!Validate()) {
        return;
    }
    pOkButton->Disable();

    TransferDataFromControls();

    int ret = 0;
    std::string message = "";

    Persistence::WorkdaysPersistence workdayPersistence(pLogger, mDatabaseFilePath);
    std::int64_t workdayId = workdayPersistence.GetWorkdayIdByDate(mDate);
    ret = workdayId > 0 ? 0 : -1;

    if (ret == -1) {
        std::string message = "Failed to create/get workday for task";
        QueueErrorNotificationEvent(message);
        return;
    }

    mTaskModel.WorkdayId = workdayId;

    Persistence::TasksPersistence taskPersistence(pLogger, mDatabaseFilePath);
    if (!bIsEdit) {
        std::int64_t taskId = taskPersistence.Create(mTaskModel);
        ret = taskId > 0 ? 0 : -1;
        mTaskId = taskId;

        ret == -1 ? message = "Failed to create task" : message = "Successfully created task";

        if (ret != -1 && mTaskAttributeValueModels.size() > 0) {
            Persistence::TaskAttributeValuesPersistence taskAttributeValuesPersistence(
                pLogger, mDatabaseFilePath);

            ret = taskAttributeValuesPersistence.CreateMany(mTaskAttributeValueModels);
            ret == -1 ? message = "Failed to create task attribute values"
                      : message = "Successfully created task attribute values";
        }
    }

    if (bIsEdit && mTaskModel.IsActive) {
        ret = taskPersistence.Update(mTaskModel);

        ret == -1 ? message = "Failed to update task" : message = "Successfully updated task";
    }

    if (bIsEdit && !mTaskModel.IsActive) {
        ret = taskPersistence.Delete(mTaskId);

        ret == -1 ? message = "Failed to delete task" : message = "Successfully deleted task";
    }

    if (ret == -1) {
        QueueErrorNotificationEvent(message);

        pOkButton->Enable();
    } else {
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Information, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        if (!bIsEdit) {
            wxCommandEvent* taskAddedEvent = new wxCommandEvent(tksEVT_TASKDATEADDED);
            taskAddedEvent->SetString(mDate);
            taskAddedEvent->SetExtraLong(static_cast<long>(mTaskId));

            wxQueueEvent(pParent, taskAddedEvent);
        }

        if (bIsEdit && mTaskModel.IsActive) {
            // FIXME: this is bug prone as mOldDate and mDate are std::string
            // CONT: probably should use date::date types and "escape" to std::string at the last
            // possible moment
            if (mOldDate != mDate) {
                // notify frame control of task date changed TO
                wxCommandEvent* taskDateChangedToEvent =
                    new wxCommandEvent(tksEVT_TASKDATEDCHANGEDTO);

                taskDateChangedToEvent->SetString(mDate);
                taskDateChangedToEvent->SetExtraLong(static_cast<long>(mTaskId));

                wxQueueEvent(pParent, taskDateChangedToEvent);

                // notify frame control of task date changed FROM
                wxCommandEvent* taskDateChangedFromEvent =
                    new wxCommandEvent(tksEVT_TASKDATEDCHANGEDFROM);

                taskDateChangedFromEvent->SetString(mOldDate);
                taskDateChangedFromEvent->SetExtraLong(static_cast<long>(mTaskId));

                wxQueueEvent(pParent, taskDateChangedFromEvent);
            }
        }

        if (bIsEdit && !mTaskModel.IsActive) {
            wxCommandEvent* taskDeletedEvent = new wxCommandEvent(tksEVT_TASKDATEDELETED);
            taskDeletedEvent->SetString(mDate);
            taskDeletedEvent->SetExtraLong(static_cast<long>(mTaskId));

            wxQueueEvent(pParent, taskDeletedEvent);
        }

        EndModal(wxID_OK);
    }
}

void TaskDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool TaskDialog::Validate()
{
    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        auto valMsg = "An employer selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pEmployerChoiceCtrl);
        return false;
    }

    int projectIndex = pProjectChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pProjectChoiceCtrl->GetClientObject(projectIndex));
    if (projectIdData->GetValue() < 1) {
        auto valMsg = "A project selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pProjectChoiceCtrl);
        return false;
    }

    int categoryIndex = pCategoryChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* categoryIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pCategoryChoiceCtrl->GetClientObject(categoryIndex));
    if (categoryIdData->GetValue() < 1) {
        auto valMsg = "A category selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pCategoryChoiceCtrl);
        return false;
    }

    auto description = pTaskDescriptionTextCtrl->GetValue().ToStdString();
    if (description.empty()) {
        auto valMsg = "Description is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pTaskDescriptionTextCtrl);
        return false;
    }

    if (description.length() < MIN_CHARACTER_COUNT ||
        description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS) {
        auto valMsg =
            fmt::format("Description must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pTaskDescriptionTextCtrl);
        return false;
    }

    auto hoursValue = pTimeHoursSpinCtrl->GetValue();
    auto minutesValue = pTimeMinutesSpinCtrl->GetValue();
    if (hoursValue == 0 && minutesValue < 5) {
        auto valMsg = fmt::format("Task duration must have elasped more than \"00:05\"");
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pTimeMinutesSpinCtrl);
        return false;
    }

    auto uniqueIdentifier = pUniqueIdentiferTextCtrl->GetValue().ToStdString();
    if (!uniqueIdentifier.empty() && (uniqueIdentifier.length() < MIN_CHARACTER_COUNT ||
                                         uniqueIdentifier.length() > MAX_CHARACTER_COUNT_NAMES)) {
        auto valMsg =
            fmt::format("Unique identifier must be at minimum {0} or maximum {1} characters long",
                MIN_CHARACTER_COUNT,
                MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pUniqueIdentiferTextCtrl);
        return false;
    }

    return true;
}

void TaskDialog::TransferDataFromControls()
{
    mTaskModel.TaskId = mTaskId;
    mTaskModel.Billable = pBillableCheckBoxCtrl->GetValue();

    auto uniqueIdentifier = pUniqueIdentiferTextCtrl->GetValue().ToStdString();
    mTaskModel.UniqueIdentifier =
        uniqueIdentifier.empty() ? std::nullopt : std::make_optional(uniqueIdentifier);

    mTaskModel.Hours = pTimeHoursSpinCtrl->GetValue();
    mTaskModel.Minutes = pTimeMinutesSpinCtrl->GetValue();

    auto description = pTaskDescriptionTextCtrl->GetValue().ToStdString();
    mTaskModel.Description = description;

    int projectIndex = pProjectChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* projectIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pProjectChoiceCtrl->GetClientObject(projectIndex));
    mTaskModel.ProjectId = projectIdData->GetValue();

    int categoryIndex = pCategoryChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* categoryIdData = reinterpret_cast<ClientData<std::int64_t>*>(
        pCategoryChoiceCtrl->GetClientObject(categoryIndex));
    mTaskModel.CategoryId = categoryIdData->GetValue();
    mTaskModel.IsActive = pIsActiveCheckBoxCtrl->GetValue();
}

void TaskDialog::ResetClientChoiceControl(bool disable)
{
    pClientChoiceCtrl->Clear();
    pClientChoiceCtrl->Append("Select client", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);
    if (disable) {
        pClientChoiceCtrl->Disable();
    }
}

void TaskDialog::ResetProjectChoiceControl(bool disable)
{
    pProjectChoiceCtrl->Clear();
    pProjectChoiceCtrl->Append("Select project", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);
    if (disable) {
        pProjectChoiceCtrl->Disable();
    }
}

void TaskDialog::ResetCategoryChoiceControl(bool disable)
{
    pCategoryChoiceCtrl->Clear();
    pCategoryChoiceCtrl->Append("Select category", new ClientData<std::int64_t>(-1));
    pCategoryChoiceCtrl->SetSelection(0);
    if (disable) {
        pCategoryChoiceCtrl->Disable();
    }
}

void TaskDialog::FetchClientEntitiesByEmployer(const std::int64_t employerId)
{
    std::vector<Model::ClientModel> clients;
    Persistence::ClientsPersistence ClientsPersistence(pLogger, mDatabaseFilePath);

    int rc = ClientsPersistence.FilterByEmployerId(employerId, clients);

    if (rc != 0) {
        std::string message = "Failed to get clients";
        QueueErrorNotificationEvent(message);
    } else {
        if (!clients.empty()) {
            if (!pClientChoiceCtrl->IsEnabled()) {
                pClientChoiceCtrl->Enable();
            }

            for (auto& client : clients) {
                pClientChoiceCtrl->Append(
                    client.Name, new ClientData<std::int64_t>(client.ClientId));
            }
        } else {
            pClientChoiceCtrl->Disable();
        }
    }
}

void TaskDialog::FetchProjectEntitiesByEmployerOrClient(
    const std::optional<std::int64_t> employerId,
    const std::optional<std::int64_t> clientId)
{
    std::vector<Model::ProjectModel> projects;
    Persistence::ProjectsPersistence projectPersistence(pLogger, mDatabaseFilePath);

    int rc = projectPersistence.FilterByEmployerIdOrClientId(employerId, clientId, projects);
    if (rc != 0) {
        std::string message = "Failed to get projects";
        QueueErrorNotificationEvent(message);
        return;
    }

    if (!projects.empty()) {
        if (!pProjectChoiceCtrl->IsEnabled()) {
            pProjectChoiceCtrl->Enable();
        }

        bool hasDefaultProject = false;
        std::int64_t defaultProjectId = -1;

        for (auto& project : projects) {
            pProjectChoiceCtrl->Append(
                project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));

            if (project.IsDefault) {
                hasDefaultProject = true;
                defaultProjectId = project.ProjectId;
                pProjectChoiceCtrl->SetStringSelection(project.DisplayName);
            }
        }

        if (hasDefaultProject && pCfg->ShowProjectAssociatedCategories()) {
            FetchCategoryEntities(std::make_optional<std::int64_t>(defaultProjectId));
        } else if (!hasDefaultProject && pCfg->ShowProjectAssociatedCategories()) {
            pCategoryChoiceCtrl->Disable();
        } else {
            FetchCategoryEntities(std::nullopt);
        }
    } else {
        pProjectChoiceCtrl->Disable();
    }
}

void TaskDialog::FetchCategoryEntities(const std::optional<std::int64_t> projectId)
{
    std::vector<repos::CategoryRepositoryModel> categories;
    repos::CategoryRepository categoryRepo(pLogger, mDatabaseFilePath);
    int rc = 0;

    if (projectId.has_value()) {
        rc = categoryRepo.FilterByProjectId(projectId.value(), categories);
    } else {
        rc = categoryRepo.Filter(categories);
    }

    if (rc != 0) {
        std::string message = "Failed to get categories";
        QueueErrorNotificationEvent(message);
        return;
    }

    if (!categories.empty()) {
        if (!pCategoryChoiceCtrl->IsEnabled()) {
            pCategoryChoiceCtrl->Enable();
        }

        for (auto& category : categories) {
            pCategoryChoiceCtrl->Append(
                category.GetFormattedName(), new ClientData<std::int64_t>(category.CategoryId));
        }
    } else {
        pCategoryChoiceCtrl->Disable();
    }
}

void TaskDialog::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
