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

#include "preferencesexportpage.h"

#include <wx/dirdlg.h>
#include <wx/richtooltip.h>

#include "../../core/environment.h"
#include "../../core/configuration.h"

namespace tks::UI::dlg
{
PreferencesExportPage::PreferencesExportPage(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pEnv(env)
    , pCfg(cfg)
    , pExportPathTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesExportPage::IsValid()
{
    return false;
}

void PreferencesExportPage::Save() {}

void PreferencesExportPage::Reset() {}

void PreferencesExportPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Export box */
    auto exportStaticBox = new wxStaticBox(this, wxID_ANY, "Export");
    auto exportStaticBoxSizer = new wxStaticBoxSizer(exportStaticBox, wxVERTICAL);
    sizer->Add(exportStaticBoxSizer, wxSizerFlags().Expand());

    /* Export path sizer */
    auto exportPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto exportPathLabel = new wxStaticText(exportStaticBox, wxID_ANY, "Export");

    /* Export path controls */
    pExportPathTextCtrl = new wxTextCtrl(exportStaticBox,
        tksIDC_EXPORT_PATH,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT | wxTE_READONLY);
    pBrowseExportPathButton = new wxButton(exportStaticBox, tksIDC_EXPORT_PATH_BUTTON, "Browse...");
    pBrowseExportPathButton->SetToolTip("Browse and select a directory to export data to");
    exportPathSizer->Add(exportPathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    exportPathSizer->Add(
        pExportPathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    exportPathSizer->Add(pBrowseExportPathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    exportStaticBoxSizer->Add(exportPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Presets box */
    auto presetsStaticBox = new wxStaticBox(this, wxID_ANY, "Presets");
    auto presetsStaticBoxSizer = new wxStaticBoxSizer(presetsStaticBox, wxVERTICAL);
    sizer->Add(presetsStaticBoxSizer, wxSizerFlags().Expand());

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesExportPage::ConfigureEventBindings()
{
    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &PreferencesExportPage::OnOpenDirectoryForExportLocation,
        this,
        tksIDC_EXPORT_PATH_BUTTON
    );
}
// clang-format on

void PreferencesExportPage::FillControls() {}

void PreferencesExportPage::DataToControls() {}

void PreferencesExportPage::OnOpenDirectoryForExportLocation(wxCommandEvent& event) {}
} // namespace tks::UI::dlg