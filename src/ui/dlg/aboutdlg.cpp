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
#include <wx/statline.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/version.h"

namespace tks::UI::dlg
{
AboutDialog::AboutDialog(wxWindow* parent, const wxString& name)
    : wxDialog(parent, wxID_ANY, "About", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , pAttributionsListView(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void AboutDialog::Initialize()
{
    CreateControls();
    ConfigureEventBindings();
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
        this, wxID_ANY, wxEmptyString, wxDefaultPosition, FromDIP(wxSize(410, -1)), wxTE_MULTILINE | wxTE_READONLY);
    descriptionCtrl->AppendText(description);
    descriptionSizer->Add(descriptionCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* License */
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
        FromDIP(wxSize(410, 150)),
        wxTE_MULTILINE | wxTE_READONLY);
    licenseTextCtrl->AppendText(Common::GetLicense());
    licenseCollPaneWindowSizer->Add(licenseTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    licenseCollPaneWindow->SetSizer(licenseCollPaneWindowSizer);
    licenseCollPaneWindowSizer->SetSizeHints(licenseCollPaneWindow);
    sizer->Add(licenseCollPane, wxSizerFlags().Expand());

    /* Software */
    /* Software collaspible pane */
    auto softwareCollPane = new wxCollapsiblePane(this, wxID_ANY, "Software");
    auto* softwareCollPaneWindow = softwareCollPane->GetPane();

    /* Software collaspible pane sizer */
    auto softwareCollPaneWindowSizer = new wxBoxSizer(wxVERTICAL);

    /* Software list view */
    auto softwaresListView = new wxListView(softwareCollPaneWindow, wxID_ANY);
    softwaresListView->AppendColumn("Component", wxLIST_FORMAT_LEFT, FromDIP(305));
    softwaresListView->AppendColumn("Version", wxLIST_FORMAT_LEFT, FromDIP(80));

    int listIndex = 0;
    int columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "date");
    softwaresListView->SetItem(listIndex, columnIndex++, "3.0.1#2");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "fmt");
    softwaresListView->SetItem(listIndex, columnIndex++, "9.1.0#1");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "nlohmann_json");
    softwaresListView->SetItem(listIndex, columnIndex++, "3.11.2");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "spdlog");
    softwaresListView->SetItem(listIndex, columnIndex++, "1.11.0");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "SQLite");
    softwaresListView->SetItem(listIndex, columnIndex++, "3.40.1#3");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "toml11");
    softwaresListView->SetItem(listIndex, columnIndex++, "3.7.1");
    columnIndex = 0;

    listIndex = softwaresListView->InsertItem(columnIndex++, "wxWidgets");
    softwaresListView->SetItem(listIndex, columnIndex++, "3.2.2.1#2");

    softwareCollPaneWindowSizer->Add(softwaresListView, wxSizerFlags().Border(wxALL, 5).Expand().Proportion(1));

    softwareCollPaneWindow->SetSizer(softwareCollPaneWindowSizer);
    softwareCollPaneWindowSizer->SetSizeHints(softwareCollPaneWindow);
    sizer->Add(softwareCollPane, wxSizerFlags().Expand());

    /* Attributions */
    /* Attributions collaspible pane */
    auto attributionsCollPane = new wxCollapsiblePane(this, wxID_ANY, "Attributions");
    auto* attributionsCollPaneWindow = attributionsCollPane->GetPane();

    /* Attributions collaspible pane sizer */
    auto attributionsCollPaneWindowSizer = new wxBoxSizer(wxVERTICAL);

    /* Attributions list view */
    pAttributionsListView = new wxListView(attributionsCollPaneWindow, wxID_ANY);
    pAttributionsListView->AppendColumn("Author", wxLIST_FORMAT_LEFT, FromDIP(120));
    pAttributionsListView->AppendColumn("Name", wxLIST_FORMAT_LEFT, FromDIP(70));
    pAttributionsListView->AppendColumn("Link", wxLIST_FORMAT_LEFT, FromDIP(220));

    listIndex = 0;
    columnIndex = 0;

    listIndex = pAttributionsListView->InsertItem(columnIndex++, "Paul J.");
    pAttributionsListView->SetItem(listIndex, columnIndex++, "Paprika");
    pAttributionsListView->SetItem(listIndex, columnIndex++, "https://www.flaticon.com/free-icons/paprika");
    columnIndex = 0;

    listIndex = pAttributionsListView->InsertItem(columnIndex++, "Fathema Khanom");
    pAttributionsListView->SetItem(listIndex, columnIndex++, "Logout");
    pAttributionsListView->SetItem(listIndex, columnIndex++, "https://www.flaticon.com/free-icons/logout");
    columnIndex = 0;

    attributionsCollPaneWindowSizer->Add(pAttributionsListView, wxSizerFlags().Border(wxALL, 5).Expand().Proportion(1));

    attributionsCollPaneWindow->SetSizer(attributionsCollPaneWindowSizer);
    attributionsCollPaneWindowSizer->SetSizeHints(attributionsCollPaneWindow);
    sizer->Add(attributionsCollPane, wxSizerFlags(1).Expand());

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

// clang-format off
void AboutDialog::ConfigureEventBindings()
{
    pAttributionsListView->Bind(
        wxEVT_LIST_ITEM_RIGHT_CLICK,
        &AboutDialog::OnItemRightClick,
        this
    );

    Bind(
        wxEVT_MENU,
        &AboutDialog::OnOpen,
        this,
        wxID_OPEN
    );
}
// clang-format on

void AboutDialog::OnItemRightClick(wxListEvent& event)
{
    auto index = event.GetIndex();
    wxListItem linkListItem;
    linkListItem.m_itemId = index;
    linkListItem.m_col = 2;
    linkListItem.m_mask = wxLIST_MASK_TEXT;
    pAttributionsListView->GetItem(linkListItem);

    mAttrAuthorLink = linkListItem.GetText().ToStdString();

    wxMenu popupMenu;
    popupMenu.Append(wxID_OPEN, "Open");
    PopupMenu(&popupMenu);
}

void AboutDialog::OnOpen(wxCommandEvent& event)
{
    wxLaunchDefaultBrowser(mAttrAuthorLink);
}
} // namespace tks::UI::dlg
