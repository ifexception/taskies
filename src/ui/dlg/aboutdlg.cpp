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

#include "aboutdlg.h"

#include <wx/collpane.h>
#include <wx/statline.h>

#include "../../common/common.h"

namespace tks::UI::dlg
{
AboutDialog::AboutDialog(wxWindow* parent, const wxString& name)
    : wxDialog(parent, wxID_ANY, "About", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void AboutDialog::Initialize()
{
    CreateControls();
    FillControls();
}

void AboutDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Logo Bitmap */
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    wxIcon icon = iconBundle.GetIcon(wxSize(128, 128));
    auto staticBmp = new wxStaticBitmap(this, wxID_ANY, icon);
    sizer->Add(staticBmp, wxSizerFlags().Border(wxLEFT | wxRIGHT, FromDIP(8)).Center());
    sizer->AddSpacer(FromDIP(5));

    /* Description */
    auto descriptionSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(descriptionSizer, wxSizerFlags().Expand());
    auto description = "Taskies is a time tracking productivity tool built with wxWidgets, SQLite, toml11, date, fmt, "
                       "nlohmann_json and spdlog";
    auto descriptionCtrl =
        new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    descriptionCtrl->AppendText(description);
    descriptionSizer->Add(descriptionCtrl, wxSizerFlags(1).Border(wxALL, FromDIP(5)));

    /* License collaspible pane */
    auto licenseCollPane = new wxCollapsiblePane(this, wxID_ANY, "License");
    auto* licenseCollPaneWindow = licenseCollPane->GetPane();

    /* License collaspible pane sizer */
    auto licenseCollPaneWindowSizer = new wxBoxSizer(wxVERTICAL);

    /* License text ctrl */
    auto licenseTextCtrl = new wxTextCtrl(licenseCollPaneWindow,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY);
    licenseTextCtrl->AppendText(Common::GetLicense());
    licenseCollPaneWindowSizer->Add(licenseTextCtrl, wxSizerFlags(1).Border(wxALL, FromDIP(5)).Expand());

    licenseCollPaneWindow->SetSizer(licenseCollPaneWindowSizer);
    licenseCollPaneWindowSizer->SetSizeHints(licenseCollPaneWindow);
    sizer->Add(licenseCollPane, wxSizerFlags(1).Expand());

    /* Footer */
    auto footerLine = new wxStaticLine(this, wxID_ANY);
    sizer->Add(footerLine, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    SetSizerAndFit(sizer);
}

void AboutDialog::FillControls() {}
} // namespace tks::UI::dlg
