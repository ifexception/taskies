// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include "outlookmeetingsviewdlg.h"

#include <wx/artprov.h>

#include "../../events.h"
#include "../../common/clientdata.h"
#include "../../common/notificationclientdata.h"

#include "../../../common/common.h"

#include "../../../services/integrations/outlookintegratorservice.h"

namespace tks::UI::dlg
{
OutlookMeetingsViewDialog::OutlookMeetingsViewDialog(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Outlook Meetings",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pMainSizer(nullptr)
    , pRefreshButton(nullptr)
    , pAccountsChoiceCtrl(nullptr)
    , pFeedbackLabel(nullptr)
    , pScrolledWindow(nullptr)
    , pCancelButton(nullptr)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void OutlookMeetingsViewDialog::Create()
{
    CreateControls();
    ConfigureEventBindings();
}

void OutlookMeetingsViewDialog::CreateControls()
{
    /* Main dialog sizer for controls */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Refresh button sizer */
    auto refreshButtonHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    pMainSizer->Add(refreshButtonHorizontalSizer, wxSizerFlags().Expand());

    /* Refresh button */
    auto providedRefreshBitmap = wxArtProvider::GetBitmapBundle(
        wxART_REFRESH, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pRefreshButton = new wxBitmapButton(this, tksIDC_REFRESH_BUTTON, providedRefreshBitmap);
    pRefreshButton->SetToolTip("Refresh meetings of selected account");
    refreshButtonHorizontalSizer->AddStretchSpacer();
    refreshButtonHorizontalSizer->Add(pRefreshButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Account label and choice control */
    auto accountLabel = new wxStaticText(this, wxID_ANY, "Account");

    pAccountsChoiceCtrl = new wxChoice(this, tksIDC_ACCOUNT_CHOICE_CTRL);
    pAccountsChoiceCtrl->SetToolTip("Select an account to display meetings for");

    pMainSizer->Add(accountLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(pAccountsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Feedback label */
    pFeedbackLabel = new wxStaticText(this, tksIDC_FEEDBACKLABEL, "No meetings found");
    pMainSizer->Add(pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

    /* Main Scrolled Window */
    pScrolledWindow = new wxScrolledWindow(this, wxID_ANY);
    pMainSizer->Add(pScrolledWindow, wxSizerFlags(1).Expand());

    auto scrolledSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(scrolledSizer);

    SetSizerAndFit(pMainSizer);
}

void OutlookMeetingsViewDialog::ConfigureEventBindings() {}

void OutlookMeetingsViewDialog::FillControls()
{
    pAccountsChoiceCtrl->Append("Select account");
}

void OutlookMeetingsViewDialog::DataToControls()
{
    std::vector<std::string> accountNames;

    Services::Integrations::OutlookIntegratorService service(pLogger);
    Services::Integrations::OutlookResult result;
    {
        wxBusyCursor cursor;

        result = service.FetchAccountNames(accountNames);
    }

    if (!result.Success) {
        std::string message = "Failed to fetch Outlook accounts";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        wxMessageBox(result.Message, Common::GetProgramName(), wxICON_ERROR | wxOK_DEFAULT);

        return;
    }
}

void OutlookMeetingsViewDialog::OnRefresh(wxCommandEvent& event) {}

void OutlookMeetingsViewDialog::SetFeedbackLabelOnEvent(const std::string& message) {}

void OutlookMeetingsViewDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {}

void OutlookMeetingsViewDialog::QueueErrorNotificationEvent(const std::string& message) {}
} // namespace tks::UI::dlg
