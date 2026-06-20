// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "preferencestasksviewpage.h"

#include <algorithm>

#include <fmt/format.h>

#include <wx/richtooltip.h>

#include "../../../utils/utils.h"

#include "../../../core/configuration.h"

namespace tks::UI::dlg
{
PreferencesTasksViewPage::PreferencesTasksViewPage(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pLogger(logger)
    , pTodayAlwaysExpanded(nullptr)
    , pUseProjectDisplayName(nullptr)
    , pAvailableTasksViewColumns(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pSelectedTasksViewColumns(nullptr)
    , mCheckedAvailableColumns()
    , mCheckedSelectedColumns()
    , mAllTasksViewColumns(Common::AvailableTasksViewColumnList())
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesTasksViewPage::IsValid()
{
    return true;
}

void PreferencesTasksViewPage::Save()
{
    pCfg->TodayAlwaysExpanded(pTodayAlwaysExpanded->GetValue());
    pCfg->UseProjectDisplayName(pUseProjectDisplayName->GetValue());

    std::vector<Core::Configuration::TasksViewColumnSetting> selectedColumnsFromListBox;

    for (unsigned int i = 0; i < pSelectedTasksViewColumns->GetCount(); i++) {
        int clientData = Utils::VoidPointerToInt(pSelectedTasksViewColumns->GetClientData(i));
        int index = static_cast<TasksViewColumnModelIndex>(clientData);

        auto iter = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [index](const Core::Configuration::TasksViewColumnSetting& setting) {
                return setting.ColumnModelIndex == index;
            });

        if (iter != mAllTasksViewColumns.end()) {
            Core::Configuration::TasksViewColumnSetting foundSetting = *iter;
            selectedColumnsFromListBox.push_back(foundSetting);
        }
    }

    SPDLOG_LOGGER_TRACE(pLogger, "{0} columns selected", selectedColumnsFromListBox.size());
}

void PreferencesTasksViewPage::Reset()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
    pUseProjectDisplayName->SetValue(pCfg->UseProjectDisplayName());
}

void PreferencesTasksViewPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Tasks view box */
    auto tasksViewBox = new wxStaticBox(this, wxID_ANY, "Tasks View");
    auto tasksViewBoxSizer = new wxStaticBoxSizer(tasksViewBox, wxVERTICAL);
    sizer->Add(tasksViewBoxSizer, wxSizerFlags().Expand());

    /* Today always expanded control */
    pTodayAlwaysExpanded =
        new wxCheckBox(tasksViewBox, tksIDC_TODAYALWAYSEXPANDED, "Today's date always expanded");
    pTodayAlwaysExpanded->SetToolTip("When selecting other dates, keep today's date expanded too");
    tasksViewBoxSizer->Add(pTodayAlwaysExpanded, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Use project display name control */
    pUseProjectDisplayName =
        new wxCheckBox(tasksViewBox, tksIDC_USEPROJECTDISPLAYNAME, "Use project display name");
    pUseProjectDisplayName->SetToolTip(
        "Use the project's display name instead of full name on the tasks view");
    tasksViewBoxSizer->Add(
        pUseProjectDisplayName, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Tasks View Columns group box */
    auto tasksViewColumnsStaticBox =
        new wxStaticBox(this, wxID_ANY, "Tasks View Columns Selection");
    auto tasksViewColumnBoxSizer = new wxStaticBoxSizer(tasksViewColumnsStaticBox, wxHORIZONTAL);
    sizer->Add(tasksViewColumnBoxSizer, wxSizerFlags().Expand().Proportion(1));

    pAvailableTasksViewColumns = new wxCheckListBox(this, tksIDC_AVAILABLETASKSVIEWCOLUMNS);
    pAvailableTasksViewColumns->SetToolTip("Available columns to display in the tasks view");
    tasksViewColumnBoxSizer->Add(
        pAvailableTasksViewColumns, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Chevrons (right/left) buttons */
    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    tasksViewColumnBoxSizer->Add(chevronButtonSizer, wxSizerFlags().CenterVertical());

    pRightChevronButton =
        new wxButton(this, tksIDC_RIGHTCHEVRONBUTTON, ">", wxDefaultPosition, wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a column to include in the tasks view display");
    pLeftChevronButton =
        new wxButton(this, tksIDC_LEFTCHEVRONBUTTON, "<", wxDefaultPosition, wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a header to exclude in the tasks view display");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Tasks view selected columns */
    pSelectedTasksViewColumns = new wxCheckListBox(this, tksIDC_SELECTEDTASKSVIEWCOLUMNS);
    pSelectedTasksViewColumns->SetToolTip(
        "Columns selected for display in the tasks view (\"Date\" column cannot be modified)");
    tasksViewColumnBoxSizer->Add(
        pSelectedTasksViewColumns, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    SetSizerAndFit(sizer);
}

void PreferencesTasksViewPage::ConfigureEventBindings()
{
    pAvailableTasksViewColumns->Bind(wxEVT_CHECKLISTBOX,
        &PreferencesTasksViewPage::OnAvailableColumnCheck,
        this,
        tksIDC_AVAILABLETASKSVIEWCOLUMNS);

    pSelectedTasksViewColumns->Bind(wxEVT_CHECKLISTBOX,
        &PreferencesTasksViewPage::OnSelectedColumnCheck,
        this,
        tksIDC_SELECTEDTASKSVIEWCOLUMNS);

    pRightChevronButton->Bind(wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnRightChevronButtonClick,
        this,
        tksIDC_RIGHTCHEVRONBUTTON);

    pLeftChevronButton->Bind(wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnLeftChevronButtonClick,
        this,
        tksIDC_LEFTCHEVRONBUTTON);
}

void PreferencesTasksViewPage::FillControls()
{
    for (const auto& tasksViewColumn : mAllTasksViewColumns) {
        pAvailableTasksViewColumns->Append(
            tasksViewColumn.Name, Utils::IntToVoidPointer(tasksViewColumn.ColumnModelIndex));
    }
}

void PreferencesTasksViewPage::DataToControls()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
    pUseProjectDisplayName->SetValue(pCfg->UseProjectDisplayName());

    auto cfgTasksViewColumns = pCfg->GetTasksViewColumns();

    // Demarcate the date column is readonly and cannot be removed / modified
    auto& dateTasksViewColumn = cfgTasksViewColumns[0];
    auto dateTasksViewColumnName = fmt::format("{0} (readonly)", dateTasksViewColumn.Name);
    pSelectedTasksViewColumns->Append(
        dateTasksViewColumnName, Utils::IntToVoidPointer(dateTasksViewColumn.ColumnModelIndex));

    cfgTasksViewColumns.erase(cfgTasksViewColumns.begin());

    for (const auto& tasksViewColumn : cfgTasksViewColumns) {
        pSelectedTasksViewColumns->Append(
            tasksViewColumn.Name, Utils::IntToVoidPointer(tasksViewColumn.ColumnModelIndex));
    }

    std::vector<Core::Configuration::TasksViewColumnSetting> availableTasksViewColumnSettings;
    for (const auto& column : mAllTasksViewColumns) {
        Core::Configuration::TasksViewColumnSetting setting(column);
        availableTasksViewColumnSettings.push_back(setting);
    }

    for (const auto& column : pCfg->GetTasksViewColumns()) {
        int itemId = pAvailableTasksViewColumns->FindString(column.Name);
        if (itemId >= 0) {
            pAvailableTasksViewColumns->Delete(itemId);
        }
    }
}

void PreferencesTasksViewPage::OnAvailableColumnCheck(wxCommandEvent& event)
{
    TasksViewColumnModelIndex index = TasksViewColumnModelIndex::Unknown;
    int item = event.GetInt();

    if (pAvailableTasksViewColumns->IsChecked(item)) {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item checked on available list box with ID \"{0}\"", event.GetInt());

        wxCheckListBox* cListBox = wxDynamicCast(event.GetEventObject(), wxCheckListBox);
        if (cListBox != nullptr) {
            int clientData = Utils::VoidPointerToInt(cListBox->GetClientData(item));
            index = static_cast<TasksViewColumnModelIndex>(clientData);
        }

        mCheckedAvailableColumns.push_back(std::make_pair(item, index));
    } else {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item unchecked from available list box with ID \"{0}\"", event.GetInt());

        // clang-format off
        mCheckedAvailableColumns.erase(
            std::remove_if(
                mCheckedAvailableColumns.begin(),
                mCheckedAvailableColumns.end(),
                [item](const std::pair<int, TasksViewColumnModelIndex>& p) {
                    return p.first == item;
                }),
            mCheckedAvailableColumns.end());
        // clang-format on
    }
}

void PreferencesTasksViewPage::OnSelectedColumnCheck(wxCommandEvent& event)
{
    int item = event.GetInt();
    if (pSelectedTasksViewColumns->IsChecked(item)) {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item checked on selected list box with ID \"{0}\"", event.GetInt());

        if (item == 0) {
            SPDLOG_LOGGER_TRACE(pLogger, "Selected special \"Date\" column, deselect it");
            pSelectedTasksViewColumns->Check(item, false);
            return;
        }
        mCheckedSelectedColumns.push_back(item);
    } else {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item unchecked from selected list box with ID \"{0}\"", event.GetInt());
        mCheckedSelectedColumns.erase(
            std::remove(mCheckedSelectedColumns.begin(), mCheckedSelectedColumns.end(), item),
            mCheckedSelectedColumns.end());
    }
}

void PreferencesTasksViewPage::OnRightChevronButtonClick(wxCommandEvent& event)
{
    if (mCheckedAvailableColumns.size() == 0) {
        return;
    }

    // clang-format off
    std::sort(
        mCheckedAvailableColumns.begin(),
        mCheckedAvailableColumns.end(),
        [&](std::pair<int, TasksViewColumnModelIndex>& lhs,
            std::pair<int, TasksViewColumnModelIndex>& rhs
            ) {
                return lhs.first > rhs.first;
        }
    );
    // clang-format on

    for (const auto& checkedPair : mCheckedAvailableColumns) {
        pAvailableTasksViewColumns->Check(checkedPair.first, false);
        pAvailableTasksViewColumns->Delete(checkedPair.first);
    }

    for (const auto& tasksViewColumn : mCheckedAvailableColumns) {
        auto iter = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [tasksViewColumn](const Common::TasksViewColumn& column) {
                return tasksViewColumn.second == column.ColumnModelIndex;
            });

        if (iter != mAllTasksViewColumns.end()) {
            auto& foundColumn = *iter;
            pSelectedTasksViewColumns->Append(
                foundColumn.Name, Utils::IntToVoidPointer(foundColumn.ColumnModelIndex));
        }
    }
}

void PreferencesTasksViewPage::OnLeftChevronButtonClick(wxCommandEvent& event) {}
} // namespace tks::UI::dlg
