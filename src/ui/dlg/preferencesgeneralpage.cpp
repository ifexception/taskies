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
    sizer->Add(uiBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));
    auto uiGridSizer = new wxFlexGridSizer(2, FromDIP(10), FromDIP(10));
    uiGridSizer->AddGrowableCol(1, 1);

    /* Language label*/
    auto languageLabel = new wxStaticText(uiBox, wxID_ANY, "Language");

    pUserInterfaceLanguageCtrl = new wxComboBox(uiBox, IDC_LANG, wxEmptyString);
    pUserInterfaceLanguageCtrl->SetToolTip("Set the language for the program to use");

    uiGridSizer->Add(languageLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    uiGridSizer->Add(pUserInterfaceLanguageCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    uiBoxSizer->Add(uiGridSizer, wxSizerFlags().Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

void PreferencesGeneralPage::ConfigureEventBindings() {}

void PreferencesGeneralPage::FillControls() {}
} // namespace tks::UI::dlg
