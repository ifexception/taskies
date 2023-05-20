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

#include "preferencesgeneralpage.h"

#include "../../core/configuration.h"
#include "../clientdata.h"

namespace tks::UI::dlg
{
PreferencesGeneralPage::PreferencesGeneralPage(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pLogger(logger)
    , pUserInterfaceLanguageCtrl(nullptr)
    , pStartWithWindowsCtrl(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesGeneralPage::IsValid()
{
    return false;
}

void PreferencesGeneralPage::Save() {}

void PreferencesGeneralPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* User Interface box */
    auto uiBox = new wxStaticBox(this, wxID_ANY, "User Interface");
    auto uiBoxSizer = new wxStaticBoxSizer(uiBox, wxHORIZONTAL);
    sizer->Add(uiBoxSizer, wxSizerFlags().Expand());
    auto uiGridSizer = new wxFlexGridSizer(2, FromDIP(10), FromDIP(10));
    uiGridSizer->AddGrowableCol(1, 1);

    /* Language label*/
    auto languageLabel = new wxStaticText(uiBox, wxID_ANY, "Language");

    pUserInterfaceLanguageCtrl = new wxChoice(uiBox, IDC_LANG);
    pUserInterfaceLanguageCtrl->SetToolTip("Set the language for the program to use");

    uiGridSizer->Add(languageLabel, wxSizerFlags().CenterVertical());
    uiGridSizer->Add(pUserInterfaceLanguageCtrl, wxSizerFlags().Right().Expand().Proportion(1));

    uiBoxSizer->Add(uiGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Misc options */
    auto miscBox = new wxStaticBox(this, wxID_ANY, "Miscellaneous");
    auto miscBoxSizer = new wxStaticBoxSizer(miscBox, wxVERTICAL);
    sizer->Add(miscBoxSizer, wxSizerFlags().Expand());
    auto miscGridSizer = new wxFlexGridSizer(2, FromDIP(10), FromDIP(10));
    miscGridSizer->AddGrowableCol(1, 1);

    /* Start with Windows */
    pStartWithWindowsCtrl = new wxCheckBox(miscBox, wxID_ANY, "Start with Windows");
    pStartWithWindowsCtrl->SetToolTip("Program gets launched by Windows on start");
    miscGridSizer->Add(pStartWithWindowsCtrl, wxSizerFlags().CenterVertical());
    miscGridSizer->Add(0, 0);

    /* Start Position */
    auto startPositionLabel = new wxStaticText(miscBox, wxID_ANY, "Start Position");

    pWindowStartPositionCtrl = new wxChoice(miscBox, IDC_START_POSITION);
    pWindowStartPositionCtrl->SetToolTip("Select the state of the program launched");
    miscGridSizer->Add(startPositionLabel, wxSizerFlags().CenterVertical());
    miscGridSizer->Add(pWindowStartPositionCtrl, wxSizerFlags().Right().CenterVertical().Proportion(1));
    miscBoxSizer->Add(miscGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

void PreferencesGeneralPage::ConfigureEventBindings() {}

void PreferencesGeneralPage::FillControls()
{
    pUserInterfaceLanguageCtrl->Append("Please Select", new ClientData<int>(-1));
    pWindowStartPositionCtrl->Append("Please Select", new ClientData<int>(-1));

    pUserInterfaceLanguageCtrl->SetSelection(0);
    pWindowStartPositionCtrl->SetSelection(0);
}
void PreferencesGeneralPage::DataToControls()
{
    pUserInterfaceLanguageCtrl->Append("en-US");
    pUserInterfaceLanguageCtrl->SetSelection(1);

    pWindowStartPositionCtrl->Append("Normal", new ClientData<int>(static_cast<int>(WindowState::Normal)));
    pWindowStartPositionCtrl->Append("Minimized", new ClientData<int>(static_cast<int>(WindowState::Minimized)));
    pWindowStartPositionCtrl->Append("Hidden", new ClientData<int>(static_cast<int>(WindowState::Hidden)));
    pWindowStartPositionCtrl->Append("Maximized", new ClientData<int>(static_cast<int>(WindowState::Maximized)));
}
} // namespace tks::UI::dlg
