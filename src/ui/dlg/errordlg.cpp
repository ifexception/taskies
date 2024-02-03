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

#include "errordlg.h"

#include <filesystem>
#include <fstream>
#include <vector>

#include <wx/artprov.h>
#include <wx/collpane.h>
#include <wx/clipbrd.h>
#include <wx/statline.h>

#include "../../common/common.h"

#include "../../core/environment.h"

namespace tks::UI::dlg
{
ErrorDialog::ErrorDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& message,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Taskies Error",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pEnv(env)
    , pLogger(logger)
    , mMessage(message)
    , pErrorIconBitmap(nullptr)
    , pErrorLabel(nullptr)
    , pErrorMessageTextCtrl(nullptr)
    , pCopyButton(nullptr)
    , pOpenIssueLink(nullptr)
    , pOkButton(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void ErrorDialog::Initialize()
{
    CreateControls();
    ConfigureEventBindings();
    DataToControls();
}

void ErrorDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Title and Icon */
    auto titleIconSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(titleIconSizer, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pErrorIconBitmap = new wxStaticBitmap(this, tksIDC_ERRORICON, wxArtProvider::GetBitmap(wxART_ERROR));
    pErrorLabel = new wxStaticText(this, tksIDC_ERRORLABEL, "Taskies encountered an error");

    titleIconSizer->Add(pErrorIconBitmap, wxSizerFlags().Border(wxALL, FromDIP(5)));
    titleIconSizer->Add(pErrorLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

    /* Error message text control */
    auto errMsgSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(errMsgSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    pErrorMessageTextCtrl = new wxTextCtrl(
        this, tksIDC_ERRORMESSAGE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    pErrorMessageTextCtrl->SetHint("Error message");
    pErrorMessageTextCtrl->SetToolTip("User friendly description of the error that occured");
    pErrorMessageTextCtrl->Disable();
    pErrorMessageTextCtrl->SetFont(wxFont(8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    errMsgSizer->Add(pErrorMessageTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Logs */
    /* Logs collaspible pane */
    auto logsCollPane = new wxCollapsiblePane(this, wxID_ANY, "Logs");
    auto* logsCollPaneWindow = logsCollPane->GetPane();

    /* Logs collaspible pane sizer */
    auto logsCollPaneSizer = new wxBoxSizer(wxVERTICAL);

    /* Logs Text Ctrl */
    pLogsTextCtrl = new wxTextCtrl(logsCollPaneWindow,
        tksIDC_LOGSTEXT,
        wxEmptyString,
        wxDefaultPosition,
        FromDIP(wxSize(-1, 156)),
        wxTE_MULTILINE | wxTE_READONLY);
    logsCollPaneSizer->Add(pLogsTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    logsCollPaneWindow->SetSizer(logsCollPaneSizer);
    logsCollPaneSizer->SetSizeHints(logsCollPaneWindow);
    sizer->Add(logsCollPane, wxSizerFlags().Expand());

    /* Include logs checkbox ctrl */
    pIncludeLogsCheckBoxCtrl = new wxCheckBox(this, tksIDC_INCLUDELOGSCHECK, "Include Logs");
    pIncludeLogsCheckBoxCtrl->SetToolTip(
        "By default, taskies will submit an issue without logs. Select this to submit your issue without logs");
    sizer->Add(pIncludeLogsCheckBoxCtrl, wxSizerFlags().Border(wxRIGHT, FromDIP(5)).Right());

    /* Action Details box*/
    auto actionsStaticBox = new wxStaticBox(this, wxID_ANY, "Actions");
    auto actionsStaticBoxSizer = new wxStaticBoxSizer(actionsStaticBox, wxHORIZONTAL);
    sizer->Add(actionsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    pCopyButton = new wxButton(actionsStaticBox, wxID_COPY, "Copy");
    pCopyButton->SetToolTip("Copy the error message to the clipboard");

    std::string link = "https://taskies.org/open-issue";
    pOpenIssueLink = new wxHyperlinkCtrl(actionsStaticBox, tksIDC_OPENISSUELINK, "Open Issue", link);
    pOpenIssueLink->SetToolTip("Open an issue for the developer to fix");

    actionsStaticBoxSizer->AddStretchSpacer();
    actionsStaticBoxSizer->Add(pOpenIssueLink, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());
    actionsStaticBoxSizer->Add(pCopyButton, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

    /* Horizontal Line*/
    auto separationLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    sizer->Add(separationLine, wxSizerFlags().Border(wxALL, FromDIP(1)).Expand());

    /* OK buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

// clang-format off
void ErrorDialog::ConfigureEventBindings()
{
    pCopyButton->Bind(
        wxEVT_BUTTON,
        &ErrorDialog::OnCopy,
        this,
        wxID_COPY
    );

    pOpenIssueLink->Bind(
        wxEVT_HYPERLINK,
        &ErrorDialog::OnOpenIssueLinkClick,
        this,
        tksIDC_OPENISSUELINK
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &ErrorDialog::OnOK,
        this,
        wxID_OK
    );
}
// clang-format on

void ErrorDialog::DataToControls()
{
    if (!mMessage.empty()) {
        pErrorMessageTextCtrl->ChangeValue(mMessage);
    }

    std::vector<std::filesystem::directory_entry> entries;
    auto logsPath = pEnv->ApplicationLogPath();

    for (const auto& entry : std::filesystem::directory_iterator(logsPath)) {
        entries.push_back(entry);
    }

    auto latestLogFileIterator = std::max_element(entries.begin(),
        entries.end(),
        [](const std::filesystem::directory_entry& f1, const std::filesystem::directory_entry& f2) {
            return std::filesystem::last_write_time(f1.path()) < std::filesystem::last_write_time(f2.path());
        });

    if (latestLogFileIterator == entries.end()) {
        pLogger->warn("ErrorDialog - No log files found at {0}", logsPath.string());
        return;
    }

    auto latestLogFile = latestLogFileIterator->path().string();

    std::vector<std::string> logFileContents;
    std::ifstream ifLogFileStream(latestLogFile);

    if (!ifLogFileStream) {
        pLogger->error("ErrorDialog - Failed to open file stream to log file at {0}", latestLogFile);
        return;
    }

    if (ifLogFileStream.is_open()) {
        for (std::string line; std::getline(ifLogFileStream, line);) {
            logFileContents.push_back(line);
        }
    }

    ifLogFileStream.close();

    for (const auto& line : logFileContents) {
        pLogsTextCtrl->AppendText(line);
        pLogsTextCtrl->AppendText('\n');
    }
}

void ErrorDialog::OnOK(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void ErrorDialog::OnCopy(wxCommandEvent& event)
{
    auto canOpen = wxTheClipboard->Open();
    if (canOpen) {
        auto textData = new wxTextDataObject(mMessage);
        wxTheClipboard->SetData(textData);
        wxTheClipboard->Close();
    }
}

void ErrorDialog::OnOpenIssueLinkClick(wxHyperlinkEvent& event)
{
    wxLaunchDefaultBrowser(event.GetURL());
}
} // namespace tks::UI::dlg
