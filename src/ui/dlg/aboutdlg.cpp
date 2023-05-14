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
#include <wx/hyperlink.h>
#include <wx/listctrl.h>
#include <wx/statline.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/version.h"

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

    /* Taskies version */
    auto version = fmt::format("Taskies v{0}.{1}.{2}", TASKIES_MAJOR, TASKIES_MINOR, TASKIES_PATCH);
    auto versionLabel = new wxStaticText(this, wxID_ANY, version);
    sizer->Add(versionLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).Center());

    sizer->AddSpacer(FromDIP(5));

    /* Description */
    auto descriptionSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(descriptionSizer, wxSizerFlags().Expand());
    auto description = "Taskies is a time tracking productivity tool built with date, fmt, nlohmann_json, spdlog, "
                       "SQLite, and wxWidgets";
    auto descriptionCtrl = new wxTextCtrl(
        this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(340, -1), wxTE_MULTILINE | wxTE_READONLY);
    descriptionCtrl->AppendText(description);
    descriptionSizer->Add(descriptionCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

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
    licenseCollPaneWindowSizer->Add(licenseTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    licenseCollPaneWindow->SetSizer(licenseCollPaneWindowSizer);
    licenseCollPaneWindowSizer->SetSizeHints(licenseCollPaneWindow);
    sizer->Add(licenseCollPane, wxSizerFlags().Expand());

    /* Software collaspible pane */
    auto softwareCollPane = new wxCollapsiblePane(this, wxID_ANY, "Software");
    auto* softwareCollPaneWindow = softwareCollPane->GetPane();

    /* Software collaspible pane sizer */
    auto softwareCollPaneWindowSizer = new wxBoxSizer(wxVERTICAL);

    /* Software list view */
    auto listView = new wxListView(softwareCollPaneWindow, wxID_ANY);
    listView->AppendColumn("Component", wxLIST_FORMAT_LEFT, FromDIP(200));
    listView->AppendColumn("Version", wxLIST_FORMAT_LEFT, FromDIP(80));

    int listIndex = 0;
    int columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "date");
    listView->SetItem(listIndex, columnIndex++, "3.0.1#2");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "fmt");
    listView->SetItem(listIndex, columnIndex++, "9.1.0#1");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "nlohmann_json");
    listView->SetItem(listIndex, columnIndex++, "3.11.2");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "spdlog");
    listView->SetItem(listIndex, columnIndex++, "1.11.0");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "SQLite");
    listView->SetItem(listIndex, columnIndex++, "3.40.1#3");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "toml11");
    listView->SetItem(listIndex, columnIndex++, "3.7.1");
    columnIndex = 0;

    listIndex = listView->InsertItem(columnIndex++, "wxWidgets");
    listView->SetItem(listIndex, columnIndex++, "3.2.2.1#2");

    softwareCollPaneWindowSizer->Add(listView, wxSizerFlags().Border(wxALL, 5).Expand().Proportion(1));

    softwareCollPaneWindow->SetSizer(softwareCollPaneWindowSizer);
    softwareCollPaneWindowSizer->SetSizeHints(softwareCollPaneWindow);
    sizer->Add(softwareCollPane, wxSizerFlags(1).Expand());

    /* Footer */
    auto footerLine = new wxStaticLine(this, wxID_ANY);
    sizer->Add(footerLine, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* Footer sizer */
    auto footerSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(footerSizer, wxSizerFlags().Expand());

    /* Copyright text */
    auto copyrightText = new wxStaticText(this, wxID_ANY, "(C) 2023");
    footerSizer->Add(copyrightText, wxSizerFlags().Border(wxALL, FromDIP(4)));
    footerSizer->AddStretchSpacer();

    /* Link */
    auto link = new wxHyperlinkCtrl(this, wxID_ANY, "https://taskies.org", "https://taskies.org");
    footerSizer->Add(link, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void AboutDialog::FillControls() {}
} // namespace tks::UI::dlg
