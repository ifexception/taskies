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

#include "preferencestasksviewpage.h"

#include <wx/richtooltip.h>

#include "../../common/clientdata.h"

#include "../../../common/common.h"
#include "../../../core/configuration.h"

static std::vector<std::string> MakeTaskViewColumns()
{
    return std::vector<std::string>{ "Employer",
        "Client",
        "Project",
        "Display Name",
        "Category",
        "Duration",
        "Billable",
        "Unique ID",
        "Description" };
}

namespace tks::UI::dlg
{
PreferencesTasksViewPage::PreferencesTasksViewPage(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pLogger(logger)
    , pTodayAlwaysExpanded(nullptr)
    , pAvailableColumnsListView(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pDisplayColumnsListView(nullptr)
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
}

void PreferencesTasksViewPage::Reset()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
}

void PreferencesTasksViewPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Tasks view options box */
    auto optionsBox = new wxStaticBox(this, wxID_ANY, "Options");
    auto optionsBoxSizer = new wxStaticBoxSizer(optionsBox, wxHORIZONTAL);
    sizer->Add(optionsBoxSizer, wxSizerFlags().Expand());

    /* Today always expanded control */
    pTodayAlwaysExpanded =
        new wxCheckBox(this, tksIDC_TODAYALWAYSEXPANDED, "Today's date always expanded");
    pTodayAlwaysExpanded->SetToolTip("When selecting other dates, keep today's date expanded too");
    optionsBoxSizer->Add(pTodayAlwaysExpanded, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Columns box */
    auto columnsBox = new wxStaticBox(this, wxID_ANY, "Columns");
    auto columnsBoxSizer = new wxStaticBoxSizer(columnsBox, wxHORIZONTAL);
    sizer->Add(columnsBoxSizer, wxSizerFlags().Expand().Proportion(1));

    /* Available column list */
    pAvailableColumnsListView = new wxListView(columnsBox,
        tksIDC_AVAILABLECOLUMNSLISTVIEW,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pAvailableColumnsListView->EnableCheckBoxes();
    pAvailableColumnsListView->SetToolTip("Select columns to display in the task view");
    columnsBoxSizer->Add(
        pAvailableColumnsListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    int availableColumnIndex = 0;

    wxListItem availableColumn;
    availableColumn.SetId(availableColumnIndex);
    availableColumn.SetText("Available Columns");
    availableColumn.SetWidth(180);
    pAvailableColumnsListView->InsertColumn(availableColumnIndex++, availableColumn);

    /* Chevrons buttons */
    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    columnsBoxSizer->Add(chevronButtonSizer, wxSizerFlags());

    pRightChevronButton =
        new wxButton(columnsBox, tksIDC_RIGHTCHEVRONBUTTON, ">", wxDefaultPosition, wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a column to include in the task view display");
    pLeftChevronButton =
        new wxButton(columnsBox, tksIDC_LEFTCHEVRONBUTTON, "<", wxDefaultPosition, wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a column to exclude from the task view display");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    pDisplayColumnsListView = new wxListView(columnsBox,
        tksIDC_DISPLAYCOLUMNSLISTVIEW,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pDisplayColumnsListView->EnableCheckBoxes();
    pDisplayColumnsListView->SetToolTip("Columns to be displayed in the task view");
    columnsBoxSizer->Add(
        pDisplayColumnsListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    int displayColumnIndex = 0;

    wxListItem displayColumnName;
    displayColumnName.SetId(displayColumnIndex);
    displayColumnName.SetText("Display Column");
    displayColumnName.SetWidth(180);
    pDisplayColumnsListView->InsertColumn(displayColumnIndex++, displayColumnName);

    wxListItem displayColumnOrder;
    displayColumnOrder.SetId(displayColumnIndex);
    displayColumnOrder.SetText("Order");
    displayColumnOrder.SetWidth(wxLIST_AUTOSIZE);
    pDisplayColumnsListView->InsertColumn(displayColumnIndex++, displayColumnOrder);

    SetSizerAndFit(sizer);
}

void PreferencesTasksViewPage::ConfigureEventBindings() {}

void PreferencesTasksViewPage::FillControls()
{
    const auto& availableColumns = MakeTaskViewColumns();
    for (auto& availableColumn : availableColumns) {
        pAvailableColumnsListView->InsertItem(0, availableColumn);
    }
}

void PreferencesTasksViewPage::DataToControls()
{
    pTodayAlwaysExpanded->SetValue(pCfg->TodayAlwaysExpanded());
}
} // namespace tks::UI::dlg
