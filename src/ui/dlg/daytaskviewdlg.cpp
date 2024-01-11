// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "daytaskviewdlg.h"

#include <wx/statline.h>

#include <fmt/format.h>

#include "../events.h"
#include "../notificationclientdata.h"

#include "../../common/common.h"

#include "../../repository/taskrepository.h"
#include "../../repository/taskrepositorymodel.h"

namespace tks::UI::dlg
{
DayTaskViewDialog::DayTaskViewDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const std::string& selectedDate,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          wxEmptyString,
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(nullptr)
    , pLogger(logger)
    , pDateCtrl(nullptr)
    , pDataViewCtrl(nullptr)
    , pTaskListModel(nullptr)
    , mDatabaseFilePath(databaseFilePath)
    , mSelectedDate(selectedDate)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    SetTitle(fmt::format("View Daily Tasks for {0}", selectedDate));

    Create();
    //SetMinSize(wxSize(FromDIP(320), FromDIP(240)));

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void DayTaskViewDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

void DayTaskViewDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Top sizer */
    auto topSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(topSizer, wxSizerFlags().Expand());

    /* Date label */
    auto dateStaticLabel = new wxStaticText(this, wxID_ANY, "Date:");

    /* Date picker ctrl */
    pDateCtrl = new wxDatePickerCtrl(this, tksIDC_DATEPICKERCTRL);

    topSizer->Add(dateStaticLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    topSizer->Add(pDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Data view ctrl */
    /* Data View Columns Renderers */
    auto projectNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto categoryNameTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto durationTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    auto descriptionTextRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT);
    descriptionTextRenderer->EnableEllipsize(wxEllipsizeMode::wxELLIPSIZE_END);

    auto idRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);
    pDataViewCtrl = new wxDataViewCtrl(this,
        tksIDC_TASKDATAVIEWCTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES);

    pTaskListModel = new TaskListModel(pLogger);
    pDataViewCtrl->AssociateModel(pTaskListModel.get());

    /* Project Column */
    auto listProjectColumn = new wxDataViewColumn(
        "Project", projectNameTextRenderer, TaskListModel::Col_Project, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    listProjectColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(listProjectColumn);

    /* Category Column */
    auto listCategoryColumn = new wxDataViewColumn(
        "Category", categoryNameTextRenderer, TaskListModel::Col_Category, 80, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
    listCategoryColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    pDataViewCtrl->AppendColumn(listCategoryColumn);

    /* Duration Column */
    auto listDurationColumn =
        new wxDataViewColumn("Duration", durationTextRenderer, TaskListModel::Col_Duration, 80, wxALIGN_CENTER);
    listDurationColumn->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    listDurationColumn->SetResizeable(false);
    pDataViewCtrl->AppendColumn(listDurationColumn);

    /* Description Column */
    auto listDescriptionColumn = new wxDataViewColumn("Description",
        descriptionTextRenderer,
        TaskListModel::Col_Description,
        80,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    pDataViewCtrl->AppendColumn(listDescriptionColumn);

    /* ID Column */
    auto listIdColumn =
        new wxDataViewColumn("ID", idRenderer, TaskListModel::Col_Id, 32, wxALIGN_CENTER, wxDATAVIEW_COL_HIDDEN);
    pDataViewCtrl->AppendColumn(listIdColumn);

    sizer->Add(pDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    auto okButton = new wxButton(this, wxID_OK, "OK");
    auto cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
    okButton->SetDefault();

    buttonsSizer->Add(okButton, wxSizerFlags().Border(wxALL, FromDIP(5)));
    buttonsSizer->Add(cancelButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

void DayTaskViewDialog::FillControls() {}

// clang-format off
void DayTaskViewDialog::ConfigureEventBindings()
{
    Bind(
        wxEVT_DATE_CHANGED,
        &DayTaskViewDialog::OnDateChange,
        this
    );

    Bind(
        wxEVT_CHAR_HOOK,
        &DayTaskViewDialog::OnKeyDown,
        this
    );
}
// clang-format on

void DayTaskViewDialog::DataToControls()
{
    std::vector<repos::TaskRepositoryModel> models;
    repos::TaskRepository taskRepo(pLogger, mDatabaseFilePath);

    int rc = taskRepo.FilterByDate(mSelectedDate, models);
    if (rc != 0) {
        QueueFetchTasksErrorNotificationEvent();
    } else {
        pTaskListModel->AppendMany(models);
    }
}

void DayTaskViewDialog::OnDateChange(wxDateEvent& event) {}

void DayTaskViewDialog::OnKeyDown(wxKeyEvent& event)
{
    event.Skip();
}

void DayTaskViewDialog::QueueFetchTasksErrorNotificationEvent()
{
    std::string message = fmt::format("Failed to fetch tasks for date {0}", mSelectedDate);
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::dlg
