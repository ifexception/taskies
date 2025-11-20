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
#include <wx/statline.h>

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
    , pScrolledWindowSizer(nullptr)
    , pActiveMeetingsPanel(nullptr)
    , pCancelButton(nullptr)
    , mSelectedAccount()
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
    FillControls();
    DataToControls();
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

    /* Horizontal Line */
    auto line0 = new wxStaticLine(this, wxID_ANY);
    pMainSizer->Add(line0, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

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

    pScrolledWindowSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(pScrolledWindowSizer);

    SetSizerAndFit(pMainSizer);
}

// clang-format off
void OutlookMeetingsViewDialog::ConfigureEventBindings()
{
    pAccountsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &OutlookMeetingsViewDialog::OnAccountChoice,
        this
    );
}
// clang-format on

void OutlookMeetingsViewDialog::FillControls()
{
    pAccountsChoiceCtrl->Append("Select account");
    pAccountsChoiceCtrl->SetSelection(0);
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

        SetFeedbackLabelOnEvent(message);

        return;
    }

    for (const std::string& accountName : accountNames) {
        pAccountsChoiceCtrl->Append(accountName);
    }

    Fit();
}

void OutlookMeetingsViewDialog::OnRefresh(wxCommandEvent& event) {}

void OutlookMeetingsViewDialog::OnAccountChoice(wxCommandEvent& event)
{
    if (pActiveMeetingsPanel != nullptr) {
        pScrolledWindowSizer->Detach(pActiveMeetingsPanel);
        pActiveMeetingsPanel->Destroy();

        pScrolledWindowSizer->Layout();
        pMainSizer->Layout();
    }

    int selection = event.GetSelection();
    if (selection == 0) {
        mSelectedAccount = "";

        pFeedbackLabel = new wxStaticText(this, tksIDC_FEEDBACKLABEL, "No meetings found");
        pMainSizer->Add(
            pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

        pMainSizer->Layout();
        Fit();

        return;
    } else {
        mSelectedAccount = pAccountsChoiceCtrl->GetString(selection).ToStdString();
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Outlook account name selected \"{0}\"",
        mSelectedAccount.empty() ? "(none)" : mSelectedAccount);

    std::vector<Services::Integrations::OutlookMeetingModel> meetingModels;

    Services::Integrations::OutlookIntegratorService service(pLogger);
    Services::Integrations::OutlookResult result;
    {
        wxBusyCursor cursor;

        result = service.FetchCalendarMeetings(mSelectedAccount, meetingModels);
    }

    if (!result.Success) {
        std::string message = "Failed to fetch Outlook meetings";

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        SetFeedbackLabelOnEvent(message);

        return;
    } else if (result.Success && !result.Message.empty()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Retrieved \"{0}\" meetings", meetingModels.size());

        if (meetingModels.size() == 0) {
            SetFeedbackLabelOnEvent("No meetings found");
            return;
        }
    }

    if (pFeedbackLabel != nullptr) {
        pMainSizer->Detach(pFeedbackLabel);
        pFeedbackLabel->Destroy();
        pFeedbackLabel = nullptr;
        pMainSizer->Layout();
        SPDLOG_LOGGER_TRACE(pLogger, "Removed feedback static text from main sizer");
    }

    /* Panel Sizer */
    auto panelSizer = new wxBoxSizer(wxVERTICAL);

    /* Panel */
    pActiveMeetingsPanel = new wxPanel(pScrolledWindow, wxID_ANY);
    pActiveMeetingsPanel->SetSizer(panelSizer);

    for (const auto& meetingModel : meetingModels) {
        // static box for meeting controls
        auto staticBox = new wxStaticBox(pActiveMeetingsPanel, wxID_ANY, "");
        auto staticBoxSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);
        panelSizer->Add(staticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        auto flexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
        flexGridSizer->AddGrowableCol(1, 1);
        staticBoxSizer->Add(flexGridSizer, wxSizerFlags().Expand().Proportion(1));

        auto meetingIdLabel = new wxStaticText(staticBox, wxID_ANY, "Entry ID");
        auto providedInfoBitmap = wxArtProvider::GetBitmapBundle(
            wxART_INFORMATION, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
        auto meetingIdLabelValue = new wxStaticBitmap(staticBox, wxID_ANY, providedInfoBitmap);
        meetingIdLabelValue->SetToolTip(meetingModel.EntryId);

        auto subjectLabel = new wxStaticText(staticBox, wxID_ANY, "Subject");
        auto subjectText = new wxTextCtrl(staticBox,
            wxID_ANY,
            meetingModel.Subject,
            wxDefaultPosition,
            wxDefaultSize,
            wxTE_READONLY);

        auto durationWithTimeLabel = new wxStaticText(staticBox, wxID_ANY, "Duration");
        auto formattedValue = fmt::format(
            "{0} ({1} - {2})", meetingModel.Duration, meetingModel.Start, meetingModel.End);
        auto durationWithTimeLabelValue = new wxStaticText(staticBox, wxID_ANY, formattedValue);

        auto locationLabel = new wxStaticText(staticBox, wxID_ANY, "Location");
        auto locationLabelValue = new wxStaticText(staticBox, wxID_ANY, meetingModel.Location);

        flexGridSizer->Add(
            meetingIdLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
        flexGridSizer->Add(meetingIdLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Left());

        flexGridSizer->Add(subjectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
        flexGridSizer->Add(
            subjectText, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

        flexGridSizer->Add(
            durationWithTimeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
        flexGridSizer->Add(
            durationWithTimeLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        flexGridSizer->Add(
            locationLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
        flexGridSizer->Add(locationLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());
    }

    pScrolledWindowSizer->Add(pActiveMeetingsPanel, wxSizerFlags().Expand());
    pScrolledWindowSizer->Layout();

    pMainSizer->Layout();
    Fit();
}

void OutlookMeetingsViewDialog::OnCancel(wxCommandEvent& WXUNUSED(event)) {}

void OutlookMeetingsViewDialog::SetFeedbackLabelOnEvent(const std::string& message)
{
    pFeedbackLabel->SetLabel(message);
}

void OutlookMeetingsViewDialog::QueueErrorNotificationEvent(const std::string& message) {}
} // namespace tks::UI::dlg
