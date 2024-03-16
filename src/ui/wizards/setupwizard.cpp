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

#include "setupwizard.h"

#include "../../common/common.h"

namespace tks::UI::wizard
{
SetupWizard::SetupWizard(wxFrame* frame, std::shared_ptr<spdlog::logger> logger)
    : wxWizard(frame, wxID_ANY, "Setup/Restore Wizard")
    , pLogger(logger)
    , pWelcomePage(nullptr)
    , pOptionPage(nullptr)
{
    // Set icon in titlebar
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);

    pWelcomePage = new WelcomePage(this);
    GetPageAreaSizer()->Add(pWelcomePage);
}

bool SetupWizard::Run()
{
    bool wizardSuccess = wxWizard::RunWizard(pWelcomePage);

    Destroy();
    return wizardSuccess;
}

WelcomePage::WelcomePage(SetupWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
}

void WelcomePage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Welcome to the Taskies Setup/Restore Wizard";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    std::string introMessage = "This wizard will help you get Taskies setup or restored to your computer";
    auto introLabel = new wxStaticText(this, wxID_ANY, introMessage);

    std::string continueNextMessage = "To continue, click Next";
    auto continueNextLabel = new wxStaticText(this, wxID_ANY, continueNextMessage);

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(introLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(continueNextLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

OptionPage::OptionPage(wxWizardPage* parent, wxWizardPage* prev, wxWizardPage* next)
    : pParent(parent)
    , pPrev(prev)
    , pNext(next)
{
    CreateControls();
    ConfigureEventBindings();
}

void OptionPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto introductionText = "Please select an option below:";
    auto introductionLabel = new wxStaticText(this, wxID_ANY, introductionText);
    introductionLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    auto defaultOptionText = "(not selecting an option will default to the setup wizard)";
    auto defaultOptionLabel = new wxStaticText(this, wxID_ANY, defaultOptionText);
    defaultOptionLabel->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    auto staticBox = new wxStaticBox(this, wxID_ANY, "Options");
    auto staticBoxSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);

    pSetupWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_SETUPWIZARD_CHECKBOX, "Setup program wizard");
    pRestoreWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_RESTOREWIZARD_CHECKBOX, "Restore database wizard");
    pSkipWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_SKIPWIZARD_CHECKBOX, "Skip program wizard");

    staticBoxSizer->Add(pSetupWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));
    staticBoxSizer->Add(pRestoreWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));
    staticBoxSizer->Add(pSkipWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));

    sizer->Add(introductionLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(defaultOptionLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(staticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    SetSizerAndFit(sizer);
}

// clang-format off
void OptionPage::ConfigureEventBindings()
{
    pSetupWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnSetupWizardFlowCheck,
        this
    );

    pRestoreWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnRestoreWizardFlowCheck,
        this
    );

    pSkipWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnSkipWizardFlowCheck,
        this
    );
}
// clang-format on

void OptionPage::OnSetupWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pRestoreWizardFlowCheckBox->SetValue(false);
        pSkipWizardFlowCheckBox->SetValue(false);
    }
}

void OptionPage::OnRestoreWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSetupWizardFlowCheckBox->SetValue(false);
        pSkipWizardFlowCheckBox->SetValue(false);
    }
}

void OptionPage::OnSkipWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSetupWizardFlowCheckBox->SetValue(false);
        pRestoreWizardFlowCheckBox->SetValue(false);
    }
}
} // namespace tks::UI::wizard
