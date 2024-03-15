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

namespace tks::UI::wizard
{
SetupWizard::SetupWizard(wxFrame* frame, std::shared_ptr<spdlog::logger> logger)
    : wxWizard(frame, wxID_ANY, "Setup/Restore Wizard")
    , pLogger(logger)
    , pPage1(nullptr)
{
    pPage1 = new WelcomePage(this);
    GetPageAreaSizer()->Add(pPage1);
}

bool SetupWizard::Run()
{
    bool wizardSuccess = wxWizard::RunWizard(pPage1);

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
} // namespace tks::UI::wizard
