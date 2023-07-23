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

#include <wx/statline.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../dao/employerdao.h"
#include "../../dao/clientdao.h"
#include "../../dao/projectdao.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"

#include "../../utils/utils.h"

namespace tks::UI::dlg
{
TaskDialog::TaskDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Add Task",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pDateContextCtrl(nullptr)
    , pEmployerChoiceCtrl(nullptr)
    , pClientChoiceCtrl(nullptr)
    , pProjectChoiceCtrl(nullptr)
    , pCategoryChoiceCtrl(nullptr)
    , pDurationHoursCtrl(nullptr)
    , pDurationMinutesCtrl(nullptr)
    , pTaskDescriptionTextCtrl(nullptr)
    , pTaskUniqueIdentiferTextCtrl(nullptr)
    , pGenerateUniqueIdentifierCtrl(nullptr)
    , pDateCreatedTextCtrl(nullptr)
    , pDateModifiedTextCtrl(nullptr)
    , pIsActiveCtrl(nullptr)
    , pOkButton(nullptr)
    , pCancelButton(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void TaskDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
}

void TaskDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Date Box */
    auto dateBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto dateBoxSizer = new wxStaticBoxSizer(dateBox, wxHORIZONTAL);

    /* Date Label */
    auto dateLabel = new wxStaticText(dateBox, wxID_ANY, "Date");

    /* Date Control */
    pDateContextCtrl = new wxDatePickerCtrl(dateBox, tksIDC_DATECONTEXT);

    dateBoxSizer->Add(dateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateBoxSizer->AddSpacer(8);
    dateBoxSizer->Add(pDateContextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(dateBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

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

    auto categoryLabel = new wxStaticText(this, wxID_ANY, "Category");
    pCategoryChoiceCtrl = new wxChoice(this, tksIDC_CATEGORYCHOICE);
    pCategoryChoiceCtrl->SetToolTip("Task to associate category with");

    leftSizer->Add(employerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pEmployerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->Add(clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->Add(projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    leftSizer->Add(categoryLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    leftSizer->Add(pCategoryChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Right Sizer */
    /* Task Details Box */
    auto taskDetailsBox = new wxStaticBox(this, wxID_ANY, "Task Details");
    auto taskDetailsBoxSizer = new wxStaticBoxSizer(taskDetailsBox, wxVERTICAL);

    /* Billable Check Box Control */
    pBillableCheckBoxCtrl = new wxCheckBox(taskDetailsBox, tksIDC_BILLABLE, "Billable");

    /* Unique ID Sizer */
    auto uniqueIdSizer = new wxBoxSizer(wxHORIZONTAL);

    /* Unique Identifier Text Control */
    auto uniqueIdLabel = new wxStaticText(taskDetailsBox, wxID_ANY, "Unique ID");
    pTaskUniqueIdentiferTextCtrl = new wxTextCtrl(taskDetailsBox, tksIDC_UNIQUEIDENTIFIER);
    pTaskUniqueIdentiferTextCtrl->SetToolTip(
        "Enter a unique identifier, ticket number, work order or other identifier to associate task with");

    /* Generate Unique Identifer Check Box Control */
    pGenerateUniqueIdentifierCtrl =
        new wxCheckBox(taskDetailsBox, tksIDC_GENERATEUNIQUEIDENTIFER, "Generate Unique ID");
    pGenerateUniqueIdentifierCtrl->SetToolTip("Generate a unique identifier to associate task with");

    /* Time Controls */
    auto durationLabel = new wxStaticText(taskDetailsBox, wxID_STATIC, "Duration");

    pDurationHoursCtrl = new wxSpinCtrl(taskDetailsBox,
        tksIDC_DURATIONHOURS,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        16);
    pDurationHoursCtrl->SetToolTip("Number of hours the task took");

    pDurationMinutesCtrl = new wxSpinCtrl(taskDetailsBox,
        tksIDC_DURATIONMINUTES,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        59);
    pDurationMinutesCtrl->SetToolTip("Number of minutes the task took");
    pDurationMinutesCtrl->SetValue(15);

    taskDetailsBoxSizer->Add(pBillableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    uniqueIdSizer->Add(uniqueIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    uniqueIdSizer->Add(pTaskUniqueIdentiferTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    taskDetailsBoxSizer->Add(uniqueIdSizer, wxSizerFlags().Expand());
    taskDetailsBoxSizer->Add(pGenerateUniqueIdentifierCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    auto durationSizer = new wxBoxSizer(wxHORIZONTAL);
    durationSizer->Add(durationLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    durationSizer->AddStretchSpacer(4);
    durationSizer->Add(pDurationHoursCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    durationSizer->Add(pDurationMinutesCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    taskDetailsBoxSizer->Add(durationSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

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

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    SetSizerAndFit(sizer);
}

void TaskDialog::FillControls() {}

// clang-format off
void TaskDialog::ConfigureEventBindings()
{

}
// clang-format on
} // namespace tks::UI::dlg
