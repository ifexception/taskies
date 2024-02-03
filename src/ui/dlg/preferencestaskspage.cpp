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

#include "preferencestaskspage.h"

#include <wx/richtooltip.h>

#include "../../common/common.h"
#include "../../core/configuration.h"
#include "../clientdata.h"

namespace tks::UI::dlg
{
PreferencesTasksPage::PreferencesTasksPage(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pLogger(logger)
    , pMinutesIncrementChoiceCtrl(nullptr)
    , pShowProjectAssociatedCategoriesCheckBoxCtrl(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesTasksPage::IsValid()
{
    int choiceIndex = pMinutesIncrementChoiceCtrl->GetSelection();
    if (choiceIndex == 0) {
        auto valMsg = "An increment selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pMinutesIncrementChoiceCtrl);
        return false;
    }

    return true;
}

void PreferencesTasksPage::Save()
{
    int choiceIndex = pMinutesIncrementChoiceCtrl->GetSelection();
    ClientData<int>* incrementData =
        reinterpret_cast<ClientData<int>*>(pMinutesIncrementChoiceCtrl->GetClientObject(choiceIndex));

    pCfg->SetMinutesIncrement(incrementData->GetValue());
    pCfg->ShowProjectAssociatedCategories(pShowProjectAssociatedCategoriesCheckBoxCtrl->GetValue());
}

void PreferencesTasksPage::Reset()
{
    pMinutesIncrementChoiceCtrl->SetSelection(pCfg->GetMinutesIncrement());
}

void PreferencesTasksPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Time Increment box */
    auto timeIncrementBox = new wxStaticBox(this, wxID_ANY, "Time Increment");
    auto timeIncrementBoxSizer = new wxStaticBoxSizer(timeIncrementBox, wxHORIZONTAL);

    /* Time Increment label */
    auto timeIncrementLabel = new wxStaticText(timeIncrementBox, wxID_ANY, "Minutes Increment");

    pMinutesIncrementChoiceCtrl = new wxChoice(timeIncrementBox, tksIDC_MINUTES_INCREMENT);
    pMinutesIncrementChoiceCtrl->SetToolTip("Set task minutes incrementer value");

    timeIncrementBoxSizer->Add(timeIncrementLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());
    timeIncrementBoxSizer->AddStretchSpacer(1);
    timeIncrementBoxSizer->Add(
        pMinutesIncrementChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    sizer->Add(timeIncrementBoxSizer, wxSizerFlags().Expand());

    /* Show project associated categories control */
    pShowProjectAssociatedCategoriesCheckBoxCtrl =
        new wxCheckBox(this, tksIDC_ASSOCIATEDCATEGORIES, "Show project associated categories");
    sizer->Add(pShowProjectAssociatedCategoriesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    SetSizerAndFit(sizer);
}

void PreferencesTasksPage::ConfigureEventBindings() {}

void PreferencesTasksPage::FillControls()
{
    pMinutesIncrementChoiceCtrl->Append("Please select");

    pMinutesIncrementChoiceCtrl->Append("1", new ClientData<int>(1));
    pMinutesIncrementChoiceCtrl->Append("5", new ClientData<int>(5));
    pMinutesIncrementChoiceCtrl->Append("15", new ClientData<int>(15));
    pMinutesIncrementChoiceCtrl->Append("30", new ClientData<int>(30));

    pMinutesIncrementChoiceCtrl->SetSelection(0);
}

void PreferencesTasksPage::DataToControls()
{
    pMinutesIncrementChoiceCtrl->SetStringSelection(std::to_string(pCfg->GetMinutesIncrement()));
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetValue(pCfg->ShowProjectAssociatedCategories());
}
} // namespace tks::UI::dlg
