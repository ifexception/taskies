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

#include "preferencestaskspage.h"

#include <wx/richtooltip.h>

#include "../../clientdata.h"

#include "../../../common/common.h"
#include "../../../core/configuration.h"

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
    , pUseLegacyTaskDialogCheckBoxCtrl(nullptr)
    , pUseRemindersCheckBoxCtrl(nullptr)
    , pReminderIntervalChoiceCtrl(nullptr)
    , pOpenTaskDialogOnReminderClickCheckBoxCtrl(nullptr)
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
        auto valMsg = "A selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pMinutesIncrementChoiceCtrl);
        return false;
    }

    if (pUseRemindersCheckBoxCtrl->GetValue()) {
        int reminderIntervalChoiceIndex = pReminderIntervalChoiceCtrl->GetSelection();
        if (reminderIntervalChoiceIndex == 0) {
            auto valMsg = "A reminder selection is required";
            wxRichToolTip tooltip("Validation", valMsg);
            tooltip.SetIcon(wxICON_WARNING);
            tooltip.ShowFor(pReminderIntervalChoiceCtrl);
            return false;
        }
    }

    return true;
}

void PreferencesTasksPage::Save()
{
    int choiceIndex = pMinutesIncrementChoiceCtrl->GetSelection();
    ClientData<int>* incrementData = reinterpret_cast<ClientData<int>*>(
        pMinutesIncrementChoiceCtrl->GetClientObject(choiceIndex));

    pCfg->SetMinutesIncrement(incrementData->GetValue());
    pCfg->ShowProjectAssociatedCategories(pShowProjectAssociatedCategoriesCheckBoxCtrl->GetValue());
    pCfg->UseLegacyTaskDialog(pUseLegacyTaskDialogCheckBoxCtrl->GetValue());

    pCfg->UseReminders(pUseRemindersCheckBoxCtrl->GetValue());

    int intervalIndex = pReminderIntervalChoiceCtrl->GetSelection();
    ClientData<int>* intervalData = reinterpret_cast<ClientData<int>*>(
        pReminderIntervalChoiceCtrl->GetClientObject(intervalIndex));

    if (intervalIndex < 1) {
        pCfg->SetReminderInterval(0);
    } else {
        pCfg->SetReminderInterval(intervalData->GetValue());
    }

    pCfg->OpenTaskDialogOnReminderClick(pOpenTaskDialogOnReminderClickCheckBoxCtrl->GetValue());
}

void PreferencesTasksPage::Reset()
{
    pMinutesIncrementChoiceCtrl->SetSelection(pCfg->GetMinutesIncrement());
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetValue(pCfg->ShowProjectAssociatedCategories());
    pUseLegacyTaskDialogCheckBoxCtrl->SetValue(pCfg->UseLegacyTaskDialog());

    pUseRemindersCheckBoxCtrl->SetValue(pCfg->UseReminders());
    pReminderIntervalChoiceCtrl->SetSelection(0);
    pReminderIntervalChoiceCtrl->Disable();

    pOpenTaskDialogOnReminderClickCheckBoxCtrl->SetValue(pCfg->OpenTaskDialogOnReminderClick());
}

void PreferencesTasksPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Task Increment box */
    auto taskIncrementBox = new wxStaticBox(this, wxID_ANY, "Task Increment");
    auto taskIncrementBoxSizer = new wxStaticBoxSizer(taskIncrementBox, wxHORIZONTAL);
    sizer->Add(taskIncrementBoxSizer, wxSizerFlags().Expand());

    /* Time Increment label */
    auto timeIncrementLabel =
        new wxStaticText(taskIncrementBox, wxID_ANY, "Task Time Increment (in minutes)");

    pMinutesIncrementChoiceCtrl = new wxChoice(taskIncrementBox, tksIDC_MINUTES_INCREMENT);
    pMinutesIncrementChoiceCtrl->SetToolTip("Set task minutes incrementer value");

    taskIncrementBoxSizer->Add(
        timeIncrementLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    taskIncrementBoxSizer->AddStretchSpacer(1);
    taskIncrementBoxSizer->Add(
        pMinutesIncrementChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Show project associated categories control */
    pShowProjectAssociatedCategoriesCheckBoxCtrl =
        new wxCheckBox(this, tksIDC_ASSOCIATEDCATEGORIES, "Show project associated categories");
    sizer->Add(pShowProjectAssociatedCategoriesCheckBoxCtrl,
        wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Use legacy task dialog */
    pUseLegacyTaskDialogCheckBoxCtrl =
        new wxCheckBox(this, tksIDC_USELEGACYTASKDIALOGCHECKBOXCTRL, "Use legacy task dialog");
    sizer->Add(pUseLegacyTaskDialogCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Reminders box */
    auto remindersBox = new wxStaticBox(this, wxID_ANY, "Reminders");
    auto remindersBoxSizer = new wxStaticBoxSizer(remindersBox, wxVERTICAL);
    sizer->Add(remindersBoxSizer, wxSizerFlags().Expand());

    /* Use Reminders control */
    pUseRemindersCheckBoxCtrl =
        new wxCheckBox(remindersBox, tksIDC_USEREMINDERSCHECKBOXCTRL, "Use Reminders");
    pUseRemindersCheckBoxCtrl->SetToolTip("Toogle reminders");

    /* Reminder Interval choice control */
    auto reminderIntervalLabel =
        new wxStaticText(remindersBox, wxID_ANY, "Reminder Interval (in minutes)");
    pReminderIntervalChoiceCtrl = new wxChoice(remindersBox, tksIDC_REMINDERINTERVALCHOICECTRL);
    pReminderIntervalChoiceCtrl->SetToolTip("Set how often a reminder should pop up");
    pReminderIntervalChoiceCtrl->Disable();

    pOpenTaskDialogOnReminderClickCheckBoxCtrl = new wxCheckBox(remindersBox,
        tksIDC_OPENTASKDIALOGONREMINDERCLICKCHECKBOXCTRL,
        "Open task dialog on reminder click");
    pOpenTaskDialogOnReminderClickCheckBoxCtrl->SetToolTip(
        "Opens the task dialog when the reminder window gets clicked");

    remindersBoxSizer->Add(pUseRemindersCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    auto reminderIntervalHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    remindersBoxSizer->Add(reminderIntervalHorizontalSizer,
        wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    reminderIntervalHorizontalSizer->Add(
        reminderIntervalLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(4)).CenterVertical());
    reminderIntervalHorizontalSizer->AddStretchSpacer(1);
    reminderIntervalHorizontalSizer->Add(
        pReminderIntervalChoiceCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(4)).Expand());
    remindersBoxSizer->Add(pOpenTaskDialogOnReminderClickCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesTasksPage::ConfigureEventBindings()
{
    pUseRemindersCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &PreferencesTasksPage::OnUseRemindersCheck,
        this
    );
}
// clang-format on

void PreferencesTasksPage::FillControls()
{
    pMinutesIncrementChoiceCtrl->Append("Please select");
    pMinutesIncrementChoiceCtrl->Append("1", new ClientData<int>(1));
    pMinutesIncrementChoiceCtrl->Append("5", new ClientData<int>(5));
    pMinutesIncrementChoiceCtrl->Append("15", new ClientData<int>(15));
    pMinutesIncrementChoiceCtrl->Append("30", new ClientData<int>(30));

    pMinutesIncrementChoiceCtrl->SetSelection(0);

    pReminderIntervalChoiceCtrl->Append("Please select");

#ifdef TKS_DEBUG
    pReminderIntervalChoiceCtrl->Append("1", new ClientData<int>(1));
#endif // TKS_DEBUG

    pReminderIntervalChoiceCtrl->Append("10", new ClientData<int>(10));
    pReminderIntervalChoiceCtrl->Append("15", new ClientData<int>(15));
    pReminderIntervalChoiceCtrl->Append("30", new ClientData<int>(30));
    pReminderIntervalChoiceCtrl->Append("45", new ClientData<int>(45));
    pReminderIntervalChoiceCtrl->Append("60", new ClientData<int>(60));

    pReminderIntervalChoiceCtrl->SetSelection(0);
}

void PreferencesTasksPage::DataToControls()
{
    pMinutesIncrementChoiceCtrl->SetStringSelection(std::to_string(pCfg->GetMinutesIncrement()));
    pShowProjectAssociatedCategoriesCheckBoxCtrl->SetValue(pCfg->ShowProjectAssociatedCategories());
    pUseLegacyTaskDialogCheckBoxCtrl->SetValue(pCfg->UseLegacyTaskDialog());

    pUseRemindersCheckBoxCtrl->SetValue(pCfg->UseReminders());
    if (pCfg->UseReminders()) {
        if (!pReminderIntervalChoiceCtrl->IsEnabled()) {
            pReminderIntervalChoiceCtrl->Enable();
            pReminderIntervalChoiceCtrl->SetStringSelection(
                std::to_string(pCfg->ReminderInterval()));
        }
    }
}

void PreferencesTasksPage::OnUseRemindersCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pReminderIntervalChoiceCtrl->Enable();
    } else {
        pReminderIntervalChoiceCtrl->Disable();
        pReminderIntervalChoiceCtrl->SetSelection(0);
        pUseRemindersCheckBoxCtrl->SetValue(false);
    }
}
} // namespace tks::UI::dlg
