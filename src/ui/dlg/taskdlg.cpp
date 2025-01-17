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

#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/common.h"

#include "../../core/configuration.h"

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
    , mDate()
    , mOldDate()
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
    auto layoutSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(layoutSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    auto leftSizer = new wxBoxSizer(wxVERTICAL);
    layoutSizer->Add(leftSizer, wxSizerFlags().Expand().Proportion(1));

    /* Begin of Left Aligned Controls */

    ///* Defaults Box */
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
    pEmployerChoiceCtrl->SetToolTip("Select an employer to get list of associated projects");

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

    /* Selections box */
    auto selectionsStaticBox = new wxStaticBox(this, wxID_ANY, "Selections");
    auto selectionsBoxSizer = new wxStaticBoxSizer(selectionsStaticBox, wxVERTICAL);
    leftSizer->Add(selectionsBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Client choice control */
    auto clientLabel = new wxStaticText(selectionsStaticBox, wxID_ANY, "Client");
    pClientChoiceCtrl = new wxChoice(selectionsStaticBox, tksIDC_CLIENTCHOICECTRL);
    pClientChoiceCtrl->SetToolTip("Select client to refine list of associated projects");

    /* Project choice control */
    auto projectLabel = new wxStaticText(selectionsStaticBox, wxID_ANY, "Project");
    pProjectChoiceCtrl = new wxChoice(selectionsStaticBox, tksIDC_PROJECTCHOICECTRL);
    pProjectChoiceCtrl->SetToolTip("Task to associate project with");

    /* Associated categories control */
    pShowProjectAssociatedCategoriesCheckBoxCtrl = new wxCheckBox(selectionsStaticBox,
        tksIDC_SHOWPROJECTASSOCIATEDCATEGORIESCHECKBOXCTRL,
        "Only show associated categories");
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetToolTip(
        "Only show categories associated to selected project");

    /* Category choice control*/
    auto categoryLabel = new wxStaticText(selectionsStaticBox, wxID_ANY, "Category");
    pCategoryChoiceCtrl = new wxChoice(selectionsStaticBox, tksIDC_CATEGORYCHOICECTRL);
    pCategoryChoiceCtrl->SetToolTip("Task to associate category with");

    /* Choices flex grid sizer */
    auto choiceFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(18));
    choiceFlexGridSizer->AddGrowableCol(1, 1);

    choiceFlexGridSizer->Add(
        clientLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    choiceFlexGridSizer->Add(pClientChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    choiceFlexGridSizer->Add(
        projectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    choiceFlexGridSizer->Add(pProjectChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    choiceFlexGridSizer->Add(0, 0);
    choiceFlexGridSizer->Add(
        pShowProjectAssociatedCategoriesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    choiceFlexGridSizer->Add(
        categoryLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    choiceFlexGridSizer->Add(
        pCategoryChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    selectionsBoxSizer->Add(choiceFlexGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

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
    pUniqueIdentiferTextCtrl->SetToolTip(
        "Enter a unique identifier, ticket number, or other identifier to associate a task with");

    /* Task Attributes control flex grid sizer */
    auto taskAttributesControlFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(6));
    taskAttributesStaticBoxSizer->Add(
        taskAttributesControlFlexGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    taskAttributesControlFlexGridSizer->Add(0, 0);
    taskAttributesControlFlexGridSizer->Add(
        pBillableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    taskAttributesControlFlexGridSizer->Add(
        uniqueIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CentreVertical());
    taskAttributesControlFlexGridSizer->Add(
        pUniqueIdentiferTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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

    /* Begin Edit Metadata Controls */

    // if (bIsEdit) {
    //     auto metadataLine = new wxStaticLine(this, wxID_ANY);
    //     sizer->Add(metadataLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    auto metadataBox = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto metadataBoxSizer = new wxStaticBoxSizer(metadataBox, wxVERTICAL);
    leftSizer->Add(metadataBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Date Created text control */
    auto dateCreatedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Created");

    pDateCreatedReadonlyTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, "-");
    pDateCreatedReadonlyTextCtrl->Disable();

    /* Date Modified text control */
    auto dateModifiedLabel = new wxStaticText(metadataBox, wxID_ANY, "Date Modified");

    pDateModifiedReadonlyTextCtrl = new wxTextCtrl(metadataBox, wxID_ANY, "-");
    pDateModifiedReadonlyTextCtrl->Disable();

    ///* Is Active checkbox control */
    pIsActiveCheckBoxCtrl = new wxCheckBox(metadataBox, tksIDC_ISACTIVECHECKBOXCTRL, "Is Active");
    pIsActiveCheckBoxCtrl->SetToolTip("Indicates if this task is actively used/still applicable");
    pIsActiveCheckBoxCtrl->Disable();

    /* Metadata flex grid sizer */
    auto metadataFlexGridSizer = new wxFlexGridSizer(2, FromDIP(6), FromDIP(8));
    metadataBoxSizer->Add(metadataFlexGridSizer, wxSizerFlags().Expand());
    metadataFlexGridSizer->AddGrowableCol(1, 1);

    metadataFlexGridSizer->Add(
        dateCreatedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateCreatedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(
        dateModifiedLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    metadataFlexGridSizer->Add(
        pDateModifiedReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    metadataFlexGridSizer->Add(0, 0);
    metadataFlexGridSizer->Add(pIsActiveCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    //}

    /* End of Edit Metadata Controls */

    /* End of Left Aligned Controls */

    /* Begin Center Aligned Controls */

    auto centerVerticalStaticLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    layoutSizer->Add(
        centerVerticalStaticLine, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

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
    pOkButton->Disable();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* End of Button Controls */

    SetSizerAndFit(sizer);
    sizer->SetSizeHints(this);
}

void TaskDialog::ConfigureEventBindings() {}

void TaskDialog::FillControls() {}

void TaskDialog::DataToControls() {}
} // namespace tks::UI::dlg
