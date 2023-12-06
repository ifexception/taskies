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

#include "taskdialog.h"

#include <algorithm>

#include <date/date.h>

#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"
#include "../../core/configuration.h"

#include "../../dao/employerdao.h"
#include "../../dao/clientdao.h"
#include "../../dao/projectdao.h"
#include "../../dao/categorydao.h"
#include "../../dao/workdaydao.h"
#include "../../dao/taskdao.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"

#include "../../repository/categoryrepositorymodel.h"
#include "../../repository/categoryrepository.h"

#include "../../utils/utils.h"

#include "../clientdata.h"
#include "../events.h"
#include "../notificationclientdata.h"

namespace tks::UI::dlg
{
TaskDialog::TaskDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isEdit,
    const std::int64_t taskId,
    const std::string& selectedDate,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          isEdit ? "Edit Task" : "Add Task",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pDateContextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pClientChoiceCtrl(nullptr)
    , pProjectChoiceCtrl(nullptr)
    , pShowProjectAssociatedCategoriesCheckBoxCtrl(nullptr)
    , pCategoryChoiceCtrl(nullptr)
    , pTimeHoursCtrl(nullptr)
    , pTimeMinutesCtrl(nullptr)
    , pTaskDescriptionTextCtrl(nullptr)
    , pUniqueIdentiferTextCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
    , pIsActiveCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
    , bIsEdit(isEdit)
    , mTaskModel()
    , mTaskId(taskId)
    , mDate("")
    , mOldDate("")
    , mEmployerIndex(-1)
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
        if (bIsEdit) {
            SetSize(FromDIP(wxSize(420, 440)));
        } else {
            SetSize(FromDIP(wxSize(420, 320)));
        }
    }

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
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
    auto baseLRSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(baseLRSizer, wxSizerFlags().Expand());

    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    auto rightSizer = new wxBoxSizer(wxVERTICAL);

    baseLRSizer->Add(leftSizer, wxSizerFlags().Expand().Proportion(1));
    baseLRSizer->Add(rightSizer, wxSizerFlags().Expand().Proportion(1));

    /* Left Sizer */
    /* Choice Controls */
    auto employerLabel = new wxStaticText(this, wxID_ANY, "Employer");
    pEmployerChoiceCtrl = new wxChoice(this, tksIDC_EMPLOYERCHOICE);
    pEmployerChoiceCtrl->SetToolTip("Select employer to get list of associated projects");

    auto clientLabel = new wxStaticText(this, wxID_ANY, "Client");
    pClientChoiceCtrl = new wxChoice(this, tksIDC_CLIENTCHOICE);
    pClientChoiceCtrl->SetToolTip("Select client to refine list of associated projects");

    auto projectLabel = new wxStaticText(this, wxID_ANY, "Project");
    pProjectChoiceCtrl = new wxChoice(this, tksIDC_PROJECTCHOICE);
    pProjectChoiceCtrl->SetToolTip("Task to associate project with");

    pShowProjectAssociatedCategoriesCheckBoxCtrl =
        new wxCheckBox(this, tksIDC_SHOWASSOCIATEDCATEGORIES, "Only show associated categories");
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetToolTip("Only show categories associated to selected project");

    auto categoryLabel = new wxStaticText(this, wxID_ANY, "Category");
    pCategoryChoiceCtrl = new wxChoice(this, tksIDC_CATEGORYCHOICE);
    pCategoryChoiceCtrl->SetToolTip("Task to associate category with");

    auto choiceFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(18));
    choiceFlexGridSizer->AddGrowableCol(1, 1);

    choiceFlexGridSizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    choiceFlexGridSizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    choiceFlexGridSizer->Add(clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    choiceFlexGridSizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    choiceFlexGridSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    choiceFlexGridSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    choiceFlexGridSizer->Add(0, 0);
    choiceFlexGridSizer->Add(pShowProjectAssociatedCategoriesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    choiceFlexGridSizer->Add(categoryLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    choiceFlexGridSizer->Add(pCategoryChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->AddSpacer(FromDIP(4));
    leftSizer->Add(choiceFlexGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Right Sizer */
    /* Task Details Box */
    auto taskDetailsBox = new wxStaticBox(this, wxID_ANY, "Task Details");
    auto taskDetailsBoxSizer = new wxStaticBoxSizer(taskDetailsBox, wxVERTICAL);

    /* Date Sizer */
    auto dateSizer = new wxBoxSizer(wxHORIZONTAL);

    /* Date Label */
    auto dateLabel = new wxStaticText(taskDetailsBox, wxID_ANY, "Date");

    /* Date Control */
    pDateContextCtrl = new wxDatePickerCtrl(taskDetailsBox, tksIDC_DATECONTEXT);

    /* Billable Check Box Control */
    pBillableCheckBoxCtrl = new wxCheckBox(taskDetailsBox, tksIDC_BILLABLE, "Billable");
    pBillableCheckBoxCtrl->SetToolTip("Indicates if a task is billable");

    /* Unique ID Sizer */
    auto uniqueIdSizer = new wxBoxSizer(wxHORIZONTAL);

    /* Unique Identifier Text Control */
    auto uniqueIdLabel = new wxStaticText(taskDetailsBox, wxID_ANY, "Unique ID");
    pUniqueIdentiferTextCtrl = new wxTextCtrl(taskDetailsBox, tksIDC_UNIQUEIDENTIFIER);
    pUniqueIdentiferTextCtrl->SetHint("Unique identifier");
    pUniqueIdentiferTextCtrl->SetToolTip(
        "Enter a unique identifier, ticket number, work order or other identifier to associate task with");

    /* Time Controls */
    auto timeLabel = new wxStaticText(taskDetailsBox, wxID_STATIC, "Time");

    pTimeHoursCtrl = new wxSpinCtrl(taskDetailsBox,
        tksIDC_DURATIONHOURS,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        16);
    pTimeHoursCtrl->SetToolTip("Number of hours the task took");

    pTimeMinutesCtrl = new wxSpinCtrl(taskDetailsBox,
        tksIDC_DURATIONMINUTES,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        59);
    pTimeMinutesCtrl->SetToolTip("Number of minutes the task took");
    pTimeMinutesCtrl->SetValue(pCfg->GetMinutesIncrement());
    pTimeMinutesCtrl->SetIncrement(pCfg->GetMinutesIncrement());

    dateSizer->Add(dateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateSizer->AddStretchSpacer(1);
    dateSizer->Add(pDateContextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    taskDetailsBoxSizer->Add(dateSizer, wxSizerFlags().Expand());

    taskDetailsBoxSizer->Add(pBillableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    uniqueIdSizer->Add(uniqueIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    uniqueIdSizer->Add(pUniqueIdentiferTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    taskDetailsBoxSizer->Add(uniqueIdSizer, wxSizerFlags().Expand());

    auto timeSizer = new wxBoxSizer(wxHORIZONTAL);
    timeSizer->Add(timeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    timeSizer->AddStretchSpacer(1);
    timeSizer->Add(pTimeHoursCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    timeSizer->Add(pTimeMinutesCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    taskDetailsBoxSizer->Add(timeSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    rightSizer->Add(taskDetailsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Task Description Text Control */
    auto descriptionBox = new wxStaticBox(this, wxID_ANY, "Task Description");
    auto descriptionBoxSizer = new wxStaticBoxSizer(descriptionBox, wxVERTICAL);

    pTaskDescriptionTextCtrl = new wxTextCtrl(
        descriptionBox, tksIDC_DESCRIPTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    pTaskDescriptionTextCtrl->SetHint("Task description");
    pTaskDescriptionTextCtrl->SetToolTip("Enter the description of the task");

    descriptionBoxSizer->Add(pTaskDescriptionTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    sizer->Add(descriptionBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    if (bIsEdit) {
        auto metadataLine = new wxStaticLine(this, wxID_ANY);
        sizer->Add(metadataLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

        auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
        auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
        sizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        /* FlexGrid sizer */
        auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
        metadataBoxSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand().Proportion(1));
        metadataFlexGridSizer->AddGrowableCol(1, 1);

        /* Date Created */
        auto dateCreatedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Created");
        metadataFlexGridSizer->Add(dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());

        pDateCreatedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
        pDateCreatedTextCtrl->Disable();
        metadataFlexGridSizer->Add(pDateCreatedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        /* Date Modified */
        auto dateModifiedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Modified");
        metadataFlexGridSizer->Add(dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());

        pDateModifiedTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, wxEmptyString);
        pDateModifiedTextCtrl->Disable();
        metadataFlexGridSizer->Add(pDateModifiedTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        /* Is Active checkbox control */
        metadataFlexGridSizer->Add(0, 0);

        pIsActiveCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVE, "Is Active");
        pIsActiveCtrl->SetToolTip("Indicates if this task is being used/still applicable");
        metadataFlexGridSizer->Add(pIsActiveCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    }

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();
    pOkButton->Disable();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    SetSizerAndFit(sizer);
    sizer->SetSizeHints(this);
}

void TaskDialog::FillControls()
{
    auto bottomRangeDate = wxDateTime::GetCurrentYear() - 1;
    auto& bottomDateContext = wxDateTime::Now().SetYear(bottomRangeDate);
    pDateContextCtrl->SetRange(bottomDateContext, wxDateTime::Now());

    wxDateTime dateTaskContext = wxDefaultDateTime;
    bool success = dateTaskContext.ParseDate(mDate);
    if (success) {
        pDateContextCtrl->SetValue(dateTaskContext);
    } else {
        pLogger->error(
            "TaskDialog::FillControls - wxDateTime failed to parse date \"{0}\". Revert to default date", mDate);
    }

    pEmployerChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pEmployerChoiceCtrl->SetSelection(0);

    pClientChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);
    pClientChoiceCtrl->Disable();

    pProjectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);
    pProjectChoiceCtrl->Disable();

    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetValue(pCfg->GetShowProjectAssociatedCategories());

    pCategoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pCategoryChoiceCtrl->SetSelection(0);

    std::string defaultSearhTerm = "";

    std::vector<Model::EmployerModel> employers;
    DAO::EmployerDao employerDao(pLogger, mDatabaseFilePath);

    int rc = employerDao.Filter(defaultSearhTerm, employers);
    if (rc != 0) {
        std::string message = "Failed to get employers";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        for (auto& employer : employers) {
            pEmployerChoiceCtrl->Append(employer.Name, new ClientData<std::int64_t>(employer.EmployerId));
        }
    }

    if (!pCfg->GetShowProjectAssociatedCategories()) {
        std::vector<Model::CategoryModel> categories;
        DAO::CategoryDao categoryDao(pLogger, mDatabaseFilePath);

        rc = categoryDao.Filter(defaultSearhTerm, categories);
        if (rc == -1) {
            std::string message = "Failed to get categories";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);
        } else {
            for (const auto& category : categories) {
                pCategoryChoiceCtrl->Append(category.Name, new ClientData<std::int64_t>(category.CategoryId));
            }
        }
    } else {
        pCategoryChoiceCtrl->Disable();
    }

    pOkButton->Enable();
}

// clang-format off
void TaskDialog::ConfigureEventBindings()
{
    pEmployerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &TaskDialog::OnEmployerChoiceSelection,
        this
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

    pDateContextCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &TaskDialog::OnDateChange,
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
    Model::TaskModel task;
    // FIXME: look into using task repo class to fetch all data in one go
    DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);
    bool isSuccess = false;

    int rc = taskDao.GetById(mTaskId, task);
    if (rc != 0) {
        std::string message = "Failed to get task";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        pBillableCheckBoxCtrl->SetValue(task.Billable);
        pUniqueIdentiferTextCtrl->ChangeValue(task.UniqueIdentifier.has_value() ? task.UniqueIdentifier.value() : "");
        pTimeHoursCtrl->SetValue(task.Hours);
        pTimeMinutesCtrl->SetValue(task.Minutes);
        pTaskDescriptionTextCtrl->ChangeValue(task.Description);
        pIsActiveCtrl->SetValue(task.IsActive);
        pDateCreatedTextCtrl->SetValue(task.GetDateCreatedString());
        pDateModifiedTextCtrl->SetValue(task.GetDateModifiedString());
        isSuccess = true;
    }

    // load project
    Model::ProjectModel project;
    DAO::ProjectDao projectDao(pLogger, mDatabaseFilePath);
    isSuccess = false;

    rc = projectDao.GetById(task.ProjectId, project);
    if (rc != 0) {
        std::string message = "Failed to get project";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
        isSuccess = false;
        return;
    } else {
        // load projects
        std::vector<Model::ProjectModel> projects;

        rc = projectDao.FilterByEmployerIdOrClientId(std::make_optional(project.EmployerId),
            project.ClientId.has_value() ? project.ClientId : std::nullopt,
            projects);
        if (rc != 0) {
            std::string message = "Failed to get projects";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);
            isSuccess = false;
            return;
        } else {
            if (!projects.empty()) {
                if (!pProjectChoiceCtrl->IsEnabled()) {
                    pProjectChoiceCtrl->Enable();
                }

                for (auto& project : projects) {
                    pProjectChoiceCtrl->Append(project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
                }
            } else {
                return;
            }
        }

        pProjectChoiceCtrl->SetStringSelection(project.DisplayName);
        isSuccess = true;

        // load employer
        Model::EmployerModel employer;
        DAO::EmployerDao employerDao(pLogger, mDatabaseFilePath);

        rc = employerDao.GetById(project.EmployerId, employer);
        if (rc == -1) {
            std::string message = "Failed to get employer";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);

            isSuccess = false;
            return;
        } else {
            pEmployerChoiceCtrl->SetStringSelection(employer.Name);
            isSuccess = true;
        }

        // load clients
        std::vector<Model::ClientModel> clients;
        DAO::ClientDao clientDao(pLogger, mDatabaseFilePath);
        std::string defaultSearchTerm = "";

        rc = clientDao.FilterByEmployerId(project.EmployerId, clients);
        if (rc == -1) {
            std::string message = "Failed to get clients";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);

            isSuccess = false;
            return;
        } else {
            // load client
            if (!clients.empty()) {
                for (const auto& client : clients) {
                    pClientChoiceCtrl->Append(client.Name, new ClientData<std::int64_t>(client.ClientId));
                }

                if (project.ClientId.has_value()) {
                    Model::ClientModel client;
                    rc = clientDao.GetById(project.ClientId.value(), client);
                    if (rc == -1) {
                        std::string message = "Failed to get client";
                        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
                        NotificationClientData* clientData =
                            new NotificationClientData(NotificationType::Error, message);
                        addNotificationEvent->SetClientObject(clientData);
                        wxQueueEvent(pParent, addNotificationEvent);

                        isSuccess = false;
                    } else {
                        pClientChoiceCtrl->SetStringSelection(client.Name);
                        isSuccess = true;
                    }
                }

                pClientChoiceCtrl->Enable();
            }
        }

        Model::CategoryModel category;
        DAO::CategoryDao categoryDao(pLogger, mDatabaseFilePath);

        rc = categoryDao.GetById(task.CategoryId, category);
        if (rc != 0) {
            std::string message = "Failed to get category";
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);
            isSuccess = false;
        } else {
            pCategoryChoiceCtrl->SetStringSelection(category.Name);
            isSuccess = true;
        }
    }

    if (isSuccess) {
        pOkButton->Enable();
        pOkButton->SetFocus();
        pOkButton->SetDefault();
    }
}

void TaskDialog::OnEmployerChoiceSelection(wxCommandEvent& event)
{
    pOkButton->Disable();

    pClientChoiceCtrl->Clear();
    pClientChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pClientChoiceCtrl->SetSelection(0);

    pProjectChoiceCtrl->Clear();
    pProjectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);

    int employerIndex = event.GetSelection();
    ClientData<std::int64_t>* employerIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        pClientChoiceCtrl->Disable();
        pProjectChoiceCtrl->Disable();
        mEmployerIndex = -1;

        return;
    }

    auto employerId = employerIdData->GetValue();
    mEmployerIndex = employerIndex;
    std::vector<Model::ClientModel> clients;
    DAO::ClientDao clientDao(pLogger, mDatabaseFilePath);

    int rc = clientDao.FilterByEmployerId(employerId, clients);

    if (rc != 0) {
        std::string message = "Failed to get clients";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (clients.empty()) {
            pClientChoiceCtrl->Disable();
        }

        for (auto& client : clients) {
            pClientChoiceCtrl->Append(client.Name, new ClientData<std::int64_t>(client.ClientId));
        }

        if (!pClientChoiceCtrl->IsEnabled() && !clients.empty()) {
            pClientChoiceCtrl->Enable();
        }
    }

    std::vector<Model::ProjectModel> projects;
    DAO::ProjectDao projectDao(pLogger, mDatabaseFilePath);

    rc = projectDao.FilterByEmployerIdOrClientId(std::make_optional(employerId), std::nullopt, projects);
    if (rc != 0) {
        std::string message = "Failed to get projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (!projects.empty()) {
            if (!pProjectChoiceCtrl->IsEnabled()) {
                pProjectChoiceCtrl->Enable();
            }

            for (auto& project : projects) {
                pProjectChoiceCtrl->Append(project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
            }

            auto hasProjectDefaultIterator = std::find_if(projects.begin(),
                projects.end(),
                [](Model::ProjectModel project) { return project.IsDefault == true; });
            if (hasProjectDefaultIterator != projects.end()) {
                auto& defaultProject = *hasProjectDefaultIterator;
                pProjectChoiceCtrl->SetStringSelection(defaultProject.DisplayName);
            }
        } else {
            pProjectChoiceCtrl->Disable();
        }
    }

    pOkButton->Enable();
}

void TaskDialog::OnClientChoiceSelection(wxCommandEvent& event)
{
    pOkButton->Disable();

    int employerIndex = mEmployerIndex;
    ClientData<std::int64_t>* employerIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pEmployerChoiceCtrl->GetClientObject(employerIndex));
    auto employerId = employerIdData->GetValue();

    int clientIndex = event.GetSelection();
    ClientData<std::int64_t>* clientIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pClientChoiceCtrl->GetClientObject(clientIndex));
    auto clientId = clientIdData->GetValue();

    pProjectChoiceCtrl->Clear();
    pProjectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pProjectChoiceCtrl->SetSelection(0);

    if (clientIdData->GetValue() < 1) {
        pProjectChoiceCtrl->Disable();

        return;
    }

    std::vector<Model::ProjectModel> projects;
    DAO::ProjectDao projectDao(pLogger, mDatabaseFilePath);

    int rc =
        projectDao.FilterByEmployerIdOrClientId(std::make_optional(employerId), std::make_optional(clientId), projects);
    if (rc != 0) {
        std::string message = "Failed to get projects";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);

        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (!projects.empty()) {
            if (!pProjectChoiceCtrl->IsEnabled()) {
                pProjectChoiceCtrl->Enable();
            }

            for (auto& project : projects) {
                pProjectChoiceCtrl->Append(project.DisplayName, new ClientData<std::int64_t>(project.ProjectId));
            }

            auto hasProjectDefaultIterator = std::find_if(projects.begin(),
                projects.end(),
                [](Model::ProjectModel project) { return project.IsDefault == true; });
            if (hasProjectDefaultIterator != projects.end()) {
                auto& defaultProject = *hasProjectDefaultIterator;
                pProjectChoiceCtrl->SetStringSelection(defaultProject.DisplayName);
            }
        } else {
            pProjectChoiceCtrl->Clear();
            pProjectChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
            pProjectChoiceCtrl->SetSelection(0);
            pProjectChoiceCtrl->Disable();
        }
    }

    pOkButton->Enable();
}

void TaskDialog::OnProjectChoiceSelection(wxCommandEvent& event)
{
    if (!pCfg->GetShowProjectAssociatedCategories()) {
        return;
    }

    pCategoryChoiceCtrl->Clear();
    pCategoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
    pCategoryChoiceCtrl->SetSelection(0);

    int projectIndex = event.GetSelection();
    ClientData<std::int64_t>* projectIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pProjectChoiceCtrl->GetClientObject(projectIndex));
    if (projectIdData->GetValue() < 1) {
        pCategoryChoiceCtrl->Disable();
        mEmployerIndex = -1;

        return;
    }

    auto projectId = projectIdData->GetValue();

    std::vector<repos::CategoryRepositoryModel> categories;
    repos::CategoryRepository categoryRepo(pLogger, mDatabaseFilePath);

    int rc = categoryRepo.FilterByProjectId(projectId, categories);
    if (rc == -1) {
        std::string message = "Failed to get categories";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    } else {
        if (!categories.empty()) {
            if (!pCategoryChoiceCtrl->IsEnabled()) {
                pCategoryChoiceCtrl->Enable();
            }

            for (auto& category : categories) {
                pCategoryChoiceCtrl->Append(
                    category.GetFormattedName(), new ClientData<std::int64_t>(category.CategoryId));
            }
        } else {
            pCategoryChoiceCtrl->Clear();
            pCategoryChoiceCtrl->Append("Please select", new ClientData<std::int64_t>(-1));
            pCategoryChoiceCtrl->SetSelection(0);
            pCategoryChoiceCtrl->Disable();
        }
    }
}

void TaskDialog::OnShowProjectAssociatedCategoriesCheck(wxCommandEvent& event) {}

void TaskDialog::OnCategoryChoiceSelection(wxCommandEvent& event)
{
    pBillableCheckBoxCtrl->SetValue(false);
    pBillableCheckBoxCtrl->SetToolTip("Indicates if a task is billable");

    int categoryIndex = event.GetSelection();
    ClientData<std::int64_t>* categoryIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pCategoryChoiceCtrl->GetClientObject(categoryIndex));

    if (categoryIdData->GetValue() < 1) {
        return;
    }

    Model::CategoryModel model;
    DAO::CategoryDao categoryDao(pLogger, mDatabaseFilePath);
    int rc = 0;

    rc = categoryDao.GetById(categoryIdData->GetValue(), model);
    if (rc == -1) {
        std::string message = "Failed to get category";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent->GetParent(), addNotificationEvent);
    } else {
        if (model.Billable) {
            pBillableCheckBoxCtrl->SetValue(true);
            pBillableCheckBoxCtrl->SetToolTip("Category selected is billable, thus task inherits billable attribute");
        }
    }
}

void TaskDialog::OnDateChange(wxDateEvent& event)
{
    pLogger->info(
        "TaskDialog::OnDateChange - Received date from event \"{0}\"", event.GetDate().FormatISODate().ToStdString());

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

void TaskDialog::OnOK(wxCommandEvent& event)
{
    pOkButton->Disable();

    if (TransferDataAndValidate()) {
        int ret = 0;
        std::string message = "";

        DAO::WorkdayDao workdayDao(pLogger, mDatabaseFilePath);
        std::int64_t workdayId = workdayDao.GetWorkdayIdByDate(mDate);
        ret = workdayId > 0 ? 0 : -1;

        if (ret == -1) {
            wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
            NotificationClientData* clientData =
                new NotificationClientData(NotificationType::Error, "Failed to get workday for task");
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);
            return;
        }

        mTaskModel.WorkdayId = workdayId;

        DAO::TaskDao taskDao(pLogger, mDatabaseFilePath);
        if (!bIsEdit) {
            std::int64_t taskId = taskDao.Create(mTaskModel);
            ret = taskId > 0 ? 0 : -1;
            mTaskId = taskId;

            ret == -1 ? message = "Failed to create task" : message = "Successfully created task";
        }
        if (bIsEdit && pIsActiveCtrl->IsChecked()) {
            ret = taskDao.Update(mTaskModel);

            ret == -1 ? message = "Failed to update task" : message = "Successfully updated task";
        }
        if (bIsEdit && !pIsActiveCtrl->IsChecked()) {
            ret = taskDao.Delete(mTaskId);

            ret == -1 ? message = "Failed to delete task" : message = "Successfully deleted task";
        }

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        if (ret == -1) {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);

            pOkButton->Enable();
        } else {
            NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
            addNotificationEvent->SetClientObject(clientData);

            wxQueueEvent(pParent, addNotificationEvent);

            if (!bIsEdit) {
                wxCommandEvent* taskAddedEvent = new wxCommandEvent(tksEVT_TASKDATEADDED);
                taskAddedEvent->SetString(mDate);
                taskAddedEvent->SetExtraLong(static_cast<long>(mTaskId));

                wxQueueEvent(pParent, taskAddedEvent);
            }
            if (bIsEdit && pIsActiveCtrl->IsChecked()) {
                // FIXME: this is could be bug prone as mOldDate and mDate are std::string
                if (mOldDate != mDate) {
                    // notify frame control of task date changed TO
                    wxCommandEvent* taskDateChangedToEvent = new wxCommandEvent(tksEVT_TASKDATEDCHANGEDTO);

                    taskDateChangedToEvent->SetString(mDate);
                    taskDateChangedToEvent->SetExtraLong(static_cast<long>(mTaskId));

                    wxQueueEvent(pParent, taskDateChangedToEvent);

                    // notify frame control of task date changed FROM
                    wxCommandEvent* taskDateChangedFromEvent = new wxCommandEvent(tksEVT_TASKDATEDCHANGEDFROM);

                    taskDateChangedFromEvent->SetString(mOldDate);
                    taskDateChangedFromEvent->SetExtraLong(static_cast<long>(mTaskId));

                    wxQueueEvent(pParent, taskDateChangedFromEvent);
                }
            }
            if (bIsEdit && !pIsActiveCtrl->IsChecked()) {
                wxCommandEvent* taskDeletedEvent = new wxCommandEvent(tksEVT_TASKDATEDELETED);
                taskDeletedEvent->SetString(mDate);
                taskDeletedEvent->SetExtraLong(static_cast<long>(mTaskId));

                wxQueueEvent(pParent, taskDeletedEvent);
            }

            EndModal(wxID_OK);
        }
    }
    pOkButton->Enable();
}

void TaskDialog::OnCancel(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

bool TaskDialog::TransferDataAndValidate()
{
    int employerIndex = pEmployerChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* employerIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pEmployerChoiceCtrl->GetClientObject(employerIndex));
    if (employerIdData->GetValue() < 1) {
        auto valMsg = "An employer selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pEmployerChoiceCtrl);
        return false;
    }

    auto uniqueIdentifier = pUniqueIdentiferTextCtrl->GetValue().ToStdString();
    if (!uniqueIdentifier.empty() &&
        (uniqueIdentifier.length() < MIN_CHARACTER_COUNT || uniqueIdentifier.length() > MAX_CHARACTER_COUNT_NAMES)) {
        auto valMsg = fmt::format("Unique identifier must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pUniqueIdentiferTextCtrl);
        return false;
    }

    int projectIndex = pProjectChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* projectIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pProjectChoiceCtrl->GetClientObject(projectIndex));
    if (projectIdData->GetValue() < 1) {
        auto valMsg = "A project selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pProjectChoiceCtrl);
        return false;
    }

    int categoryIndex = pCategoryChoiceCtrl->GetSelection();
    ClientData<std::int64_t>* categoryIdData =
        reinterpret_cast<ClientData<std::int64_t>*>(pCategoryChoiceCtrl->GetClientObject(categoryIndex));
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

    if (description.length() < MIN_CHARACTER_COUNT || description.length() > MAX_CHARACTER_COUNT_DESCRIPTIONS) {
        auto valMsg = fmt::format("Description must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pTaskDescriptionTextCtrl);
        return false;
    }

    auto hoursValue = pTimeHoursCtrl->GetValue();
    auto minutesValue = pTimeMinutesCtrl->GetValue();
    if (hoursValue == 0 && minutesValue < 5) {
        auto valMsg = fmt::format("Task duration must have elasped more than \"00:05\"");
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pTimeMinutesCtrl);
        return false;
    }

    mTaskModel.TaskId = mTaskId;
    mTaskModel.Billable = pBillableCheckBoxCtrl->GetValue();
    mTaskModel.UniqueIdentifier = uniqueIdentifier.empty() ? std::nullopt : std::make_optional(uniqueIdentifier);
    mTaskModel.Hours = pTimeHoursCtrl->GetValue();
    mTaskModel.Minutes = pTimeMinutesCtrl->GetValue();
    mTaskModel.Description = description;
    mTaskModel.ProjectId = projectIdData->GetValue();
    mTaskModel.CategoryId = categoryIdData->GetValue();

    return true;
}
} // namespace tks::UI::dlg
