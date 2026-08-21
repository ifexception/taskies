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
#include <cassert>

#include <fmt/format.h>

#include <wx/spinctrl.h>
#include <wx/richtooltip.h>

#include "../../common/clientdata.h"

#include "../../../common/enumclientdata.h"

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
    , pAvailableTasksViewColumnsListBox(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pSelectedTasksViewColumnsListBox(nullptr)
    , pAscSortButton(nullptr)
    , pDescSortButton(nullptr)
    , pSelectedColumnNameReadonlyTextCtrl(nullptr)
    , pSelectedColumnTextAlignmentChoiceCtrl(nullptr)
    , pSelectedColumnTextEllipsisModeChoiceCtrl(nullptr)
    , mCheckedAvailableColumns()
    , mCheckedSelectedColumns()
    , mAllTasksViewColumns()
    , mCfgTasksViewColumns()
    , mDefaultTasksViewColumnSettingProperties()
    , mTasksViewColumnSettingProperties()
{
    mAllTasksViewColumns = Core::Settings::MakeAllTasksViewColumnList();
    mAllTasksViewColumns.pop_back();

    mCfgTasksViewColumns = pCfg->GetTasksViewColumns();
    mCfgTasksViewColumns.pop_back();

    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesTasksViewPage::IsValid()
{
    return true;
}

void PreferencesTasksViewPage::Save(bool* restartRequired)
{
    pCfg->TodayAlwaysExpanded(pTodayAlwaysExpanded->GetValue());
    pCfg->UseProjectDisplayName(pUseProjectDisplayName->GetValue());

    std::vector<Core::Settings::TasksViewColumnSetting> selectedTasksViewColumnsFromCheckListBox;

    for (unsigned int i = 0; i < pSelectedTasksViewColumnsListBox->GetCount(); i++) {
        int clientData =
            Utils::VoidPointerToInt(pSelectedTasksViewColumnsListBox->GetClientData(i));
        auto index = static_cast<TasksViewColumnIdentifier>(clientData);

        auto iterator = std::find_if(mCfgTasksViewColumns.begin(),
            mCfgTasksViewColumns.end(),
            [index](const Core::Settings::TasksViewColumnSetting& setting) {
                return setting.TaskViewColumnId == index;
            });

        if (iterator != mCfgTasksViewColumns.end()) {
            Core::Settings::TasksViewColumnSetting foundSetting = *iterator;
            foundSetting.Selected = true;
            selectedTasksViewColumnsFromCheckListBox.push_back(foundSetting);
        }
    }

    selectedTasksViewColumnsFromCheckListBox.push_back(
        Core::Settings::MakeDescriptionTasksViewColumn());

    SPDLOG_LOGGER_TRACE(
        pLogger, "\"{0}\" columns selected", selectedTasksViewColumnsFromCheckListBox.size());

    for (size_t i = 0; i < selectedTasksViewColumnsFromCheckListBox.size(); i++) {
        selectedTasksViewColumnsFromCheckListBox[i].Order = i;
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Set order index for \"{0}\" selected columns",
        selectedTasksViewColumnsFromCheckListBox.size());

    std::sort(selectedTasksViewColumnsFromCheckListBox.begin(),
        selectedTasksViewColumnsFromCheckListBox.end(),
        [](const Core::Settings::TasksViewColumnSetting& lhs,
            const Core::Settings::TasksViewColumnSetting& rhs) { return lhs.Order < rhs.Order; });

    SPDLOG_LOGGER_TRACE(pLogger,
        "Sorted by \"Order\" for \"{0}\" selected columns",
        selectedTasksViewColumnsFromCheckListBox.size());

    auto tasksViewColumns = pCfg->GetTasksViewColumns();
    for (size_t i = 0; i < tasksViewColumns.size(); i++) {
        auto iterator = std::find_if(selectedTasksViewColumnsFromCheckListBox.begin(),
            selectedTasksViewColumnsFromCheckListBox.end(),
            [&](const Core::Settings::TasksViewColumnSetting& s) {
                return s.TaskViewColumnId == tasksViewColumns[i].TaskViewColumnId;
            });

        if (iterator != selectedTasksViewColumnsFromCheckListBox.end()) {
            Core::Settings::TasksViewColumnSetting setting = *iterator;
            tasksViewColumns[i] = setting;

            *restartRequired = true;
        }
    }

    // clang-format off
    std::sort(
        tasksViewColumns.begin(),
        tasksViewColumns.end(),
        [](
            const Core::Settings::TasksViewColumnSetting& lhs,
            const Core::Settings::TasksViewColumnSetting& rhs
        ) {
            return lhs.Order < rhs.Order;
        }
    );
    // clang-format on

    pCfg->SetTasksViewColumns(tasksViewColumns);
}

void PreferencesTasksViewPage::Reset()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
    pUseProjectDisplayName->SetValue(pCfg->UseProjectDisplayName());

    // TODO: Reset tasks view columns
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
    auto tasksViewColumnStaticBoxSizer =
        new wxStaticBoxSizer(tasksViewColumnsStaticBox, wxVERTICAL);
    sizer->Add(tasksViewColumnStaticBoxSizer, wxSizerFlags().Expand());

    /* Columns selection sizer */
    auto columnsSelectionSizer = new wxBoxSizer(wxHORIZONTAL);
    tasksViewColumnStaticBoxSizer->Add(columnsSelectionSizer, wxSizerFlags().Expand());

    pAvailableTasksViewColumnsListBox =
        new wxCheckListBox(tasksViewColumnsStaticBox, tksIDC_AVAILABLETASKSVIEWCOLUMNS);
    pAvailableTasksViewColumnsListBox->SetToolTip("Available columns to display in the tasks view");
    columnsSelectionSizer->Add(
        pAvailableTasksViewColumnsListBox, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Chevrons (right/left) buttons */
    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    columnsSelectionSizer->Add(chevronButtonSizer, wxSizerFlags().CenterVertical());

    pRightChevronButton = new wxButton(tasksViewColumnsStaticBox,
        tksIDC_RIGHTCHEVRONBUTTON,
        ">",
        wxDefaultPosition,
        wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a column to include in the tasks view display");
    pLeftChevronButton = new wxButton(tasksViewColumnsStaticBox,
        tksIDC_LEFTCHEVRONBUTTON,
        "<",
        wxDefaultPosition,
        wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a column to exclude in the tasks view display");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Tasks view selected columns */
    pSelectedTasksViewColumnsListBox =
        new wxCheckListBox(tasksViewColumnsStaticBox, tksIDC_SELECTEDTASKSVIEWCOLUMNS);
    pSelectedTasksViewColumnsListBox->SetToolTip("Columns selected for display in the tasks view");
    columnsSelectionSizer->Add(
        pSelectedTasksViewColumnsListBox, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Sort (up/down) buttons */
    auto sortButtonSizer = new wxBoxSizer(wxVERTICAL);
    columnsSelectionSizer->Add(sortButtonSizer, wxSizerFlags().CenterVertical());

    pAscSortButton = new wxButton(tasksViewColumnsStaticBox, tksIDC_ASCSORTBUTTON, "Asc");
    pAscSortButton->SetToolTip("Sort a column ascending in the tasks view display");
    pDescSortButton = new wxButton(tasksViewColumnsStaticBox, tksIDC_DESCSORTBUTTON, "Desc");
    pDescSortButton->SetToolTip("Sort a column descending in the tasks view display");

    sortButtonSizer->Add(pAscSortButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    sortButtonSizer->Add(pDescSortButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Tasks View Column Properties */
    auto tasksViewColumnPropertiesStaticBox =
        new wxStaticBox(tasksViewColumnsStaticBox, wxID_ANY, "Selected Column Properties");
    auto tasksViewColumnPropertiesStaticBoxSizer =
        new wxStaticBoxSizer(tasksViewColumnPropertiesStaticBox, wxVERTICAL);
    tasksViewColumnStaticBoxSizer->Add(
        tasksViewColumnPropertiesStaticBoxSizer, wxSizerFlags().Expand());

    /* Selected column name text ctrl */
    auto columnNameLabel = new wxStaticText(tasksViewColumnPropertiesStaticBox, wxID_ANY, "Column");
    pSelectedColumnNameReadonlyTextCtrl = new wxTextCtrl(tasksViewColumnPropertiesStaticBox,
        tksIDC_SELECTEDCOLUMNNAMEREADONLYTEXTCTRL,
        "",
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_READONLY);
    pSelectedColumnNameReadonlyTextCtrl->SetHint("Selected column name");
    pSelectedColumnNameReadonlyTextCtrl->SetToolTip("Name of column currently selected");

    /* Selected column alignment choice ctrl*/
    auto selectedColumnAlignmentLabel =
        new wxStaticText(tasksViewColumnPropertiesStaticBox, wxID_ANY, "Alignment");
    pSelectedColumnTextAlignmentChoiceCtrl =
        new wxChoice(tasksViewColumnPropertiesStaticBox, tksIDC_SELECTEDCOLUMNTEXTALIGNMENTCHOICE);
    pSelectedColumnTextAlignmentChoiceCtrl->SetToolTip("Set the column text alignment");

    /* Selected column ellipsis mode choice ctrl*/
    auto selectedColumnEllipsisModeLabel =
        new wxStaticText(tasksViewColumnPropertiesStaticBox, wxID_ANY, "Ellipsis Mode");
    pSelectedColumnTextEllipsisModeChoiceCtrl = new wxChoice(
        tasksViewColumnPropertiesStaticBox, tksIDC_SELECTEDCOLUMNTEXTELLIPSISMODECHOICECTRL);
    pSelectedColumnTextEllipsisModeChoiceCtrl->SetToolTip(
        "Set the column ellipsis mode when the column text exceeds the tasks view column area");

    /* Apply properties button */
    pApplyButton = new wxButton(tasksViewColumnPropertiesStaticBox, tksIDC_APPLYBUTTON, "Apply");
    pApplyButton->SetToolTip("Apply and save column properties (if any)");

    /* Flex grid sizer for property controls */
    auto tasksViewColumnPropertiesGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    tasksViewColumnPropertiesGridSizer->AddGrowableCol(1, 1);

    tasksViewColumnPropertiesGridSizer->Add(
        columnNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    tasksViewColumnPropertiesGridSizer->Add(
        pSelectedColumnNameReadonlyTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    tasksViewColumnPropertiesGridSizer->Add(
        selectedColumnAlignmentLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    tasksViewColumnPropertiesGridSizer->Add(
        pSelectedColumnTextAlignmentChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    tasksViewColumnPropertiesGridSizer->Add(
        selectedColumnEllipsisModeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    tasksViewColumnPropertiesGridSizer->Add(
        pSelectedColumnTextEllipsisModeChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    tasksViewColumnPropertiesStaticBoxSizer->Add(
        tasksViewColumnPropertiesGridSizer, wxSizerFlags().Expand());

    tasksViewColumnPropertiesStaticBoxSizer->Add(
        pApplyButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesTasksViewPage::ConfigureEventBindings()
{
    pAvailableTasksViewColumnsListBox->Bind(
        wxEVT_CHECKLISTBOX,
        &PreferencesTasksViewPage::OnAvailableColumnCheck,
        this,
        tksIDC_AVAILABLETASKSVIEWCOLUMNS
    );

    pSelectedTasksViewColumnsListBox->Bind(
        wxEVT_CHECKLISTBOX,
        &PreferencesTasksViewPage::OnSelectedColumnCheck,
        this,
        tksIDC_SELECTEDTASKSVIEWCOLUMNS
    );

    pRightChevronButton->Bind(
        wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnRightChevronButtonClick,
        this,
        tksIDC_RIGHTCHEVRONBUTTON
    );

    pLeftChevronButton->Bind(
        wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnLeftChevronButtonClick,
        this,
        tksIDC_LEFTCHEVRONBUTTON
    );

    pAscSortButton->Bind(
        wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnAscButtonClick,
        this,
        tksIDC_ASCSORTBUTTON
    );

    pDescSortButton->Bind(
        wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnDescButtonClick,
        this,
        tksIDC_DESCSORTBUTTON
    );

    pSelectedColumnTextAlignmentChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &PreferencesTasksViewPage::OnTextAlignmentChoice,
        this
    );

    pSelectedColumnTextEllipsisModeChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &PreferencesTasksViewPage::OnEllipsisModeChoice,
        this
    );

    pApplyButton->Bind(
        wxEVT_BUTTON,
        &PreferencesTasksViewPage::OnApplyButtonClick,
        this,
        tksIDC_APPLYBUTTON
    );
}
// clang-format on

void PreferencesTasksViewPage::FillControls()
{
    for (const auto& tasksViewColumn : mAllTasksViewColumns) {
        pAvailableTasksViewColumnsListBox->Append(tasksViewColumn.DisplayName,
            Utils::IntToVoidPointer(static_cast<int>(tasksViewColumn.TaskViewColumnId)));
    }

    pSelectedColumnTextAlignmentChoiceCtrl->AppendString("Please select");
    auto taskAlignments = Common::Static::TasksViewColumnTextAlignmentChoices();
    for (size_t i = 0; i < taskAlignments.size(); i++) {
        pSelectedColumnTextAlignmentChoiceCtrl->Append(taskAlignments[i].Value,
            new ClientData<Common::EnumClientData<TasksViewColumnTextAlignment>>(
                taskAlignments[i]));
    }
    pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(0);
    pSelectedColumnTextAlignmentChoiceCtrl->Disable();

    pSelectedColumnTextEllipsisModeChoiceCtrl->AppendString("Please select");
    auto taskEllipsisModes = Common::Static::TasksViewColumnEllipsizeModeChoices();
    for (size_t i = 0; i < taskEllipsisModes.size(); i++) {
        pSelectedColumnTextEllipsisModeChoiceCtrl->Append(taskEllipsisModes[i].Value,
            new ClientData<Common::EnumClientData<TasksViewColumnEllipsisMode>>(
                taskEllipsisModes[i]));
    }
    pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(0);
    pSelectedColumnTextEllipsisModeChoiceCtrl->Disable();

    pApplyButton->Disable();
}

void PreferencesTasksViewPage::DataToControls()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
    pUseProjectDisplayName->SetValue(pCfg->UseProjectDisplayName());

    std::vector<Core::Settings::TasksViewColumnSetting> cfgTasksViewColumns = mCfgTasksViewColumns;

    cfgTasksViewColumns.erase(
        std::remove_if(cfgTasksViewColumns.begin(),
            cfgTasksViewColumns.end(),
            [&](const Core::Settings::TasksViewColumnSetting& s) { return !s.Selected; }),
        cfgTasksViewColumns.end());

    for (const auto& tasksViewColumn : cfgTasksViewColumns) {
        pSelectedTasksViewColumnsListBox->Append(tasksViewColumn.DisplayName,
            Utils::IntToVoidPointer(static_cast<int>(tasksViewColumn.TaskViewColumnId)));
    }

    for (const auto& column : cfgTasksViewColumns) {
        int itemId = pAvailableTasksViewColumnsListBox->FindString(column.DisplayName);
        if (itemId >= 0) {
            pAvailableTasksViewColumnsListBox->Delete(itemId);
        }
    }
}

void PreferencesTasksViewPage::OnAvailableColumnCheck(wxCommandEvent& event)
{
    TasksViewColumnIdentifier index = TasksViewColumnIdentifier::Unknown;
    int item = event.GetInt();

    if (pAvailableTasksViewColumnsListBox->IsChecked(item)) {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item checked on available list box with ID \"{0}\"", event.GetInt());

        wxCheckListBox* cListBox = wxDynamicCast(event.GetEventObject(), wxCheckListBox);
        if (cListBox == nullptr) {
            return;
        }
        int clientData = Utils::VoidPointerToInt(cListBox->GetClientData(item));
        index = static_cast<TasksViewColumnIdentifier>(clientData);

        mCheckedAvailableColumns.push_back(std::make_pair(item, index));
    } else {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item unchecked from available list box with ID \"{0}\"", event.GetInt());

        // clang-format off
        mCheckedAvailableColumns.erase(
            std::remove_if(
                mCheckedAvailableColumns.begin(),
                mCheckedAvailableColumns.end(),
                [item](const std::pair<int, TasksViewColumnIdentifier>& p) {
                    return p.first == item;
                }),
            mCheckedAvailableColumns.end());
        // clang-format on
    }
}

void PreferencesTasksViewPage::OnSelectedColumnCheck(wxCommandEvent& event)
{
    TasksViewColumnIdentifier index = TasksViewColumnIdentifier::Unknown;
    int item = event.GetInt();

    if (pSelectedTasksViewColumnsListBox->IsChecked(item)) {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item checked on selected list box with ID \"{0}\"", event.GetInt());

        wxCheckListBox* cListBox = wxDynamicCast(event.GetEventObject(), wxCheckListBox);
        if (cListBox != nullptr) {
            int clientData = Utils::VoidPointerToInt(cListBox->GetClientData(item));
            index = static_cast<TasksViewColumnIdentifier>(clientData);

            if (index == TasksViewColumnIdentifier::Description) {
                pSelectedTasksViewColumnsListBox->Check(item, false);
            }
        }

        auto cfgTasksViewColumnIterator = std::find_if(mCfgTasksViewColumns.begin(),
            mCfgTasksViewColumns.end(),
            [&](const Core::Settings::TasksViewColumnSetting& s) {
                return s.TaskViewColumnId == index;
            });

        if (cfgTasksViewColumnIterator != mCfgTasksViewColumns.end()) {
            Core::Settings::TasksViewColumnSetting column = *cfgTasksViewColumnIterator;
            mCheckedSelectedColumns.push_back(std::make_pair(item, column));

            if (mCheckedSelectedColumns.size() == 1) {
                column = mCheckedSelectedColumns[0].second;
                mTasksViewColumnSettingProperties = column;

                pSelectedColumnNameReadonlyTextCtrl->ChangeValue(column.DisplayName);

                pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(
                    static_cast<int>(column.TextAlignment));
                pSelectedColumnTextAlignmentChoiceCtrl->Enable();

                pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(
                    static_cast<int>(column.EllipsisMode));
                pSelectedColumnTextEllipsisModeChoiceCtrl->Enable();

                pApplyButton->Enable();
            } else {
                pSelectedColumnNameReadonlyTextCtrl->ChangeValue("");

                pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(0);
                pSelectedColumnTextAlignmentChoiceCtrl->Disable();

                pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(0);
                pSelectedColumnTextEllipsisModeChoiceCtrl->Disable();

                pApplyButton->Disable();

                mTasksViewColumnSettingProperties = mDefaultTasksViewColumnSettingProperties;
            }
        }
    } else {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Item unchecked from selected list box with ID \"{0}\"", event.GetInt());

        // clang-format off
        mCheckedSelectedColumns.erase(
            std::remove_if(
                mCheckedSelectedColumns.begin(),
                mCheckedSelectedColumns.end(),
                [item](const std::pair<int, Core::Settings::TasksViewColumnSetting>& p) {
                    return p.first == item;
                }),
            mCheckedSelectedColumns.end());
        // clang-format on

        if (mCheckedSelectedColumns.size() == 1) {
            Core::Settings::TasksViewColumnSetting column = mCheckedSelectedColumns[0].second;

            pSelectedColumnNameReadonlyTextCtrl->ChangeValue(column.DisplayName);

            pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(
                static_cast<int>(column.TextAlignment));
            pSelectedColumnTextAlignmentChoiceCtrl->Enable();

            pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(
                static_cast<int>(column.EllipsisMode));
            pSelectedColumnTextEllipsisModeChoiceCtrl->Enable();

            pApplyButton->Enable();
        } else {
            pSelectedColumnNameReadonlyTextCtrl->ChangeValue("");

            pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(0);
            pSelectedColumnTextAlignmentChoiceCtrl->Disable();

            pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(0);
            pSelectedColumnTextEllipsisModeChoiceCtrl->Disable();

            pApplyButton->Disable();

            mTasksViewColumnSettingProperties = mDefaultTasksViewColumnSettingProperties;
        }
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
        [&](std::pair<int, TasksViewColumnIdentifier>& lhs,
            std::pair<int, TasksViewColumnIdentifier>& rhs
            ) {
                return lhs.first > rhs.first;
        }
    );
    // clang-format on

    for (const auto& checkedPair : mCheckedAvailableColumns) {
        pAvailableTasksViewColumnsListBox->Check(checkedPair.first, false);
        pAvailableTasksViewColumnsListBox->Delete(checkedPair.first);
    }

    for (const auto& tasksViewColumn : mCheckedAvailableColumns) {
        auto iter = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [tasksViewColumn](const Core::Settings::TasksViewColumnSetting& column) {
                return tasksViewColumn.second == column.TaskViewColumnId;
            });

        if (iter != mAllTasksViewColumns.end()) {
            auto& foundColumn = *iter;
            pSelectedTasksViewColumnsListBox->Append(foundColumn.DisplayName,
                Utils::IntToVoidPointer(static_cast<int>(foundColumn.TaskViewColumnId)));
        }
    }

    mCheckedAvailableColumns.clear();
}

void PreferencesTasksViewPage::OnLeftChevronButtonClick(wxCommandEvent& event)
{
    if (mCheckedSelectedColumns.size() == 0) {
        return;
    }

    // clang-format off
    std::sort(
        mCheckedSelectedColumns.begin(),
        mCheckedSelectedColumns.end(),
        [&](std::pair<int, Core::Settings::TasksViewColumnSetting>& lhs,
            std::pair<int, Core::Settings::TasksViewColumnSetting>& rhs
            ) {
                return lhs.first > rhs.first;
            }
    );
    // clang-format on

    if (mCheckedSelectedColumns.size() == 1) {
        pSelectedColumnNameReadonlyTextCtrl->ChangeValue("");

        pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(0);
        pSelectedColumnTextAlignmentChoiceCtrl->Disable();

        pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(0);
        pSelectedColumnTextEllipsisModeChoiceCtrl->Disable();

        pApplyButton->Disable();

        mTasksViewColumnSettingProperties = mDefaultTasksViewColumnSettingProperties;
    }

    for (const auto& checkedPair : mCheckedSelectedColumns) {
        pSelectedTasksViewColumnsListBox->Check(checkedPair.first, false);
        pSelectedTasksViewColumnsListBox->Delete(checkedPair.first);
    }

    for (const auto& tasksViewColumn : mCheckedSelectedColumns) {
        auto iter = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [tasksViewColumn](const Core::Settings::TasksViewColumnSetting& column) {
                return tasksViewColumn.second.TaskViewColumnId == column.TaskViewColumnId;
            });

        if (iter != mAllTasksViewColumns.end()) {
            auto& foundColumn = *iter;

            pAvailableTasksViewColumnsListBox->Append(foundColumn.DisplayName,
                Utils::IntToVoidPointer(static_cast<int>(foundColumn.TaskViewColumnId)));
        }
    }

    mCheckedSelectedColumns.clear();
}

void PreferencesTasksViewPage::OnAscButtonClick(wxCommandEvent& event)
{
    // sort asc on selected column
    if (mCheckedSelectedColumns.size() == 0 || mCheckedSelectedColumns.size() >= 2) {
        wxMessageBox("Can only sort one column at a time!",
            Common::GetProgramName(),
            wxICON_INFORMATION | wxOK_DEFAULT);
        return;
    }

    if (mCheckedSelectedColumns.size() == 1) {
        auto& checkedSelectedColumn = mCheckedSelectedColumns[0];
        auto iterator = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [checkedSelectedColumn](const Core::Settings::TasksViewColumnSetting& column) {
                return checkedSelectedColumn.second.TaskViewColumnId == column.TaskViewColumnId;
            });
        if (iterator != mAllTasksViewColumns.end()) {
            Core::Settings::TasksViewColumnSetting match = *iterator;
            int pos = pSelectedTasksViewColumnsListBox->FindString(match.DisplayName);
            int opos = pos;
            --pos;
            if (pos == 0) {
                pSelectedTasksViewColumnsListBox->Check(opos, false);
                mCheckedSelectedColumns.clear();
                return;
            }
            pSelectedTasksViewColumnsListBox->Delete(opos);
            pSelectedTasksViewColumnsListBox->Insert(
                match.Name, pos, Utils::IntToVoidPointer(static_cast<int>(match.TaskViewColumnId)));
            pSelectedTasksViewColumnsListBox->Check(pos);
            mCheckedSelectedColumns[0].first = pos;
        }
    }
}

void PreferencesTasksViewPage::OnDescButtonClick(wxCommandEvent& event)
{
    // sort desc on selected column
    if (mCheckedSelectedColumns.size() == 0 || mCheckedSelectedColumns.size() >= 2) {
        wxMessageBox("Only one column at a time can be sorted",
            Common::GetProgramName(),
            wxICON_INFORMATION | wxOK_DEFAULT);
        return;
    }

    if (mCheckedSelectedColumns.size() == 1) {
        auto& checkedSelectedColumn = mCheckedSelectedColumns[0];
        auto iter = std::find_if(mAllTasksViewColumns.begin(),
            mAllTasksViewColumns.end(),
            [checkedSelectedColumn](const Core::Settings::TasksViewColumnSetting& column) {
                return checkedSelectedColumn.second.TaskViewColumnId == column.TaskViewColumnId;
            });

        if (iter != mAllTasksViewColumns.end()) {
            Core::Settings::TasksViewColumnSetting match = *iter;
            int pos = pSelectedTasksViewColumnsListBox->FindString(match.DisplayName);
            if (pos >= (int) pSelectedTasksViewColumnsListBox->GetCount() - 1) {
                pSelectedTasksViewColumnsListBox->Check(pos, false);
                mCheckedSelectedColumns.clear();
                return;
            }
            pSelectedTasksViewColumnsListBox->Delete(pos);
            // int opos = pos;
            pos++;
            pSelectedTasksViewColumnsListBox->Insert(
                match.Name, pos, Utils::IntToVoidPointer(static_cast<int>(match.TaskViewColumnId)));
            pSelectedTasksViewColumnsListBox->Check(pos);

            mCheckedSelectedColumns[0].first = pos;
        }
    }
}

void PreferencesTasksViewPage::OnTextAlignmentChoice(wxCommandEvent& event)
{
    assert(mTasksViewColumnSettingProperties.IsValid());

    int textAlignmentChoiceIndex = pSelectedColumnTextAlignmentChoiceCtrl->GetSelection();
    ClientData<Common::EnumClientData<TasksViewColumnTextAlignment>>* textAlignmentChoiceData =
        reinterpret_cast<ClientData<Common::EnumClientData<TasksViewColumnTextAlignment>>*>(
            pSelectedColumnTextAlignmentChoiceCtrl->GetClientObject(textAlignmentChoiceIndex));

    mTasksViewColumnSettingProperties.TextAlignment = textAlignmentChoiceData->GetValue().Data;
}

void PreferencesTasksViewPage::OnEllipsisModeChoice(wxCommandEvent& event)
{
    assert(mTasksViewColumnSettingProperties.IsValid());

    int textEllipsisModeChoiceIndex = pSelectedColumnTextEllipsisModeChoiceCtrl->GetSelection();
    ClientData<Common::EnumClientData<TasksViewColumnEllipsisMode>>* textEllipsisModeChoiceData =
        reinterpret_cast<ClientData<Common::EnumClientData<TasksViewColumnEllipsisMode>>*>(
            pSelectedColumnTextEllipsisModeChoiceCtrl->GetClientObject(
                textEllipsisModeChoiceIndex));

    mTasksViewColumnSettingProperties.EllipsisMode = textEllipsisModeChoiceData->GetValue().Data;
}

void PreferencesTasksViewPage::OnApplyButtonClick(wxCommandEvent& event)
{
    assert(mTasksViewColumnSettingProperties.IsValid());

    for (size_t i = 0; i < mCfgTasksViewColumns.size(); i++) {
        if (mTasksViewColumnSettingProperties.TaskViewColumnId ==
            mCfgTasksViewColumns[i].TaskViewColumnId) {
            mCfgTasksViewColumns[i].TextAlignment = mTasksViewColumnSettingProperties.TextAlignment;
            mCfgTasksViewColumns[i].EllipsisMode = mTasksViewColumnSettingProperties.EllipsisMode;

            pSelectedColumnNameReadonlyTextCtrl->ChangeValue("");

            pSelectedColumnTextAlignmentChoiceCtrl->SetSelection(0);
            pSelectedColumnTextAlignmentChoiceCtrl->Disable();

            pSelectedColumnTextEllipsisModeChoiceCtrl->SetSelection(0);
            pSelectedColumnTextEllipsisModeChoiceCtrl->Disable();

            pApplyButton->Disable();

            mTasksViewColumnSettingProperties = mDefaultTasksViewColumnSettingProperties;

            int item = mCheckedSelectedColumns[0].first;
            pSelectedTasksViewColumnsListBox->Check(item, false);

            mCheckedAvailableColumns.clear();

            pSelectedTasksViewColumnsListBox->SetFocus();

            break;
        }
    }
}
} // namespace tks::UI::dlg
