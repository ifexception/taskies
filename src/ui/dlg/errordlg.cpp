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

#include "errordlg.h"

#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/statline.h>

namespace tks::UI::dlg
{
ErrorDialog::ErrorDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& message,
    const wxString& name)
    : wxDialog(parent, wxID_ANY, "Error", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX, name)
    , pParent(parent)
    , pLogger(logger)
    , mMessage(message)
    , pErrorIconBitmap(nullptr)
    , pErrorLabel(nullptr)
    , pErrorMessageTextCtrl(nullptr)
    , pCopyButton(nullptr)
    , pOpenIssueLink(nullptr)
    , pOkButton(nullptr)
{
    Create();

    wxIconBundle iconBundle("TASKIES_ICO", 0);
    SetIcons(iconBundle);
}

void ErrorDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
    DataToControls();

    SetSize(FromDIP(wxSize(340, 280)));
    Centre();
}

void ErrorDialog::CreateControls()
{
    /* Main Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Title and Icon */
    auto titleIconSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(titleIconSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Align(wxEXPAND).Proportion(1));

    pErrorIconBitmap = new wxStaticBitmap(this, IDC_ERRORICON, wxArtProvider::GetBitmap(wxART_ERROR));
    titleIconSizer->Add(pErrorIconBitmap, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pErrorLabel = new wxStaticText(this, IDC_ERRORLABEL, "An unexpected error occured during the operation of the program");
    titleIconSizer->Add(pErrorLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    /* Error message text control */
    auto errMsgSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(errMsgSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    pErrorMessageTextCtrl =
        new wxTextCtrl(this, IDC_ERRORMESSAGE, wxEmptyString, wxDefaultPosition, wxSize(-1, 56), wxTE_MULTILINE | wxTE_READONLY);
    pErrorMessageTextCtrl->SetHint("Error message");
    pErrorMessageTextCtrl->SetToolTip("Description of the error that occured");
    pErrorMessageTextCtrl->Disable();
    pErrorMessageTextCtrl->SetFont(wxFont(8, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    errMsgSizer->Add(pErrorMessageTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Action Details box*/
    auto actionsStaticBox = new wxStaticBox(this, wxID_ANY, "Actions");
    auto actionsStaticBoxSizer = new wxStaticBoxSizer(actionsStaticBox, wxHORIZONTAL);
    sizer->Add(actionsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Align(wxEXPAND).Proportion(1));

    pCopyButton = new wxButton(actionsStaticBox, wxID_COPY, "Copy");
    pCopyButton->SetToolTip("Copy the error message to the clipboard");
    actionsStaticBoxSizer->Add(pCopyButton, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

    actionsStaticBoxSizer->AddStretchSpacer();

    std::string link = "https://github.com/ifexception/taskies/issues"; ///new?title=BUG%3A&template=ISSUE_TEMPLATE.md";
    pOpenIssueLink = new wxHyperlinkCtrl(actionsStaticBox, IDC_OPENISSUELINK, "Open Issue", link);
    pOpenIssueLink->SetToolTip("Open an issue on GitHub to help the developer fix the issue");
    actionsStaticBoxSizer->Add(pOpenIssueLink, wxSizerFlags().Border(wxALL, FromDIP(5)).CenterVertical());

     /* Horizontal Line*/
    auto separationLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    sizer->Add(separationLine, wxSizerFlags().Border(wxALL, FromDIP(1)).Expand());

    /* OK|Cancel buttons */
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
        IDC_OPENISSUELINK
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
} // namespace tks::UI