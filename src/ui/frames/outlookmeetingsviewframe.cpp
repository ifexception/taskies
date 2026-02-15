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

#include "outlookmeetingsviewframe.h"

#include <algorithm>

#include <wx/artprov.h>
#include <wx/statline.h>

#include "../events.h"
#include "../common/clientdata.h"
#include "../common/notificationclientdata.h"
#include "../dlg/taskdlg.h"

#include "../../common/common.h"
#include "../../core/configuration.h"
#include "../../persistence/attendedmeetingspersistence.h"
#include "../../services/integrations/outlookintegratorservice.h"
#include "../../utils/utils.h"

namespace tks::UI::frames
{
OutlookMeetingsViewFrame::OutlookMeetingsViewFrame(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    const wxString& name)
    : wxFrame(parent,
          wxID_ANY,
          "Outlook Meetings",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pThisPanel(nullptr)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pMainSizer(nullptr)
    , pRefreshButton(nullptr)
    , pAccountsChoiceCtrl(nullptr)
    , pFeedbackLabel(nullptr)
    , pScrolledWindow(nullptr)
    , pScrolledWindowSizer(nullptr)
    , pActiveMeetingsPanel(nullptr)
    , pAttendedCheckBoxes()
    , mSelectedAccount()
    , mMeetingModels()
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    Create();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void OutlookMeetingsViewFrame::Create()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();

    wxSize parentWindowSize = pParent->GetSize();
    /* SPDLOG_LOGGER_TRACE(pLogger,
        "PARENT SIZE ({0},{1})",
        parentWindowSize.GetHeight(),
        parentWindowSize.GetWidth());
    wxSize dialogMaxSize;
    dialogMaxSize.SetHeight(parentWindowSize.GetHeight());
    dialogMaxSize.SetWidth(-1);
    SetMaxSize(dialogMaxSize);*/

    wxPoint screenPos = pParent->GetScreenPosition();
    int screenX = screenPos.x + parentWindowSize.x;
    int screenY = screenPos.y;
    SPDLOG_LOGGER_TRACE(pLogger, "PARENT POSITION ({0},{1})", screenX, screenY);
    wxPoint topRightScreen(screenX, screenY);
    SetPosition(topRightScreen);

    SetMinSize(wxSize(240, 180));
    SetSize(wxSize(240, 180));
}

void OutlookMeetingsViewFrame::CreateControls()
{
    /* Main dialog sizer for controls */
    pMainSizer = new wxBoxSizer(wxVERTICAL);

    /* Main panel for the frame */
    pThisPanel = new wxPanel(this, wxID_ANY);
    pThisPanel->SetSizer(pMainSizer);

    /* Refresh button sizer */
    auto refreshButtonHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    pMainSizer->Add(refreshButtonHorizontalSizer, wxSizerFlags().Expand());

    /* Refresh button */
    auto providedRefreshBitmap = wxArtProvider::GetBitmapBundle(
        wxART_REFRESH, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pRefreshButton = new wxBitmapButton(pThisPanel, tksIDC_REFRESH_BUTTON, providedRefreshBitmap);
    pRefreshButton->SetToolTip("Refresh meetings of selected account");
    pRefreshButton->Disable();
    refreshButtonHorizontalSizer->AddStretchSpacer();
    refreshButtonHorizontalSizer->Add(pRefreshButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line0 = new wxStaticLine(pThisPanel, wxID_ANY);
    pMainSizer->Add(line0, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Account label and choice control */
    auto accountLabel = new wxStaticText(pThisPanel, wxID_ANY, "Account");

    pAccountsChoiceCtrl = new wxChoice(pThisPanel, tksIDC_ACCOUNT_CHOICE_CTRL);
    pAccountsChoiceCtrl->SetToolTip("Select an account to display meetings for");

    pMainSizer->Add(accountLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMainSizer->Add(pAccountsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Feedback label */
    pFeedbackLabel = new wxStaticText(pThisPanel, tksIDC_FEEDBACKLABEL, "No account selected");
    pMainSizer->Add(pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

    /* Main Scrolled Window */
    pScrolledWindow = new wxScrolledWindow(pThisPanel, wxID_ANY);
    pScrolledWindowSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(pScrolledWindowSizer);
    pScrolledWindow->SetScrollRate(0, 20);
    pScrolledWindowSizer->FitInside(pScrolledWindow);

    pMainSizer->Add(pScrolledWindow, wxSizerFlags(1).Expand());
}

// clang-format off
void OutlookMeetingsViewFrame::ConfigureEventBindings()
{
    pAccountsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &OutlookMeetingsViewFrame::OnAccountChoice,
        this
    );
}
// clang-format on

void OutlookMeetingsViewFrame::FillControls()
{
    pAccountsChoiceCtrl->Append("Select account");
    pAccountsChoiceCtrl->SetSelection(0);
}

void OutlookMeetingsViewFrame::DataToControls()
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

        pFeedbackLabel->SetLabel(message);

        return;
    }

    for (const std::string& accountName : accountNames) {
        pAccountsChoiceCtrl->Append(accountName);
    }
}

void OutlookMeetingsViewFrame::OnParentFrameMove()
{
    wxSize parentWindowSize = pParent->GetSize();
    wxPoint screenPos = pParent->GetScreenPosition();
    int screenX = screenPos.x + parentWindowSize.x;
    int screenY = screenPos.y;
    SPDLOG_LOGGER_TRACE(pLogger, "PARENT POSITION CHANGED!\nNew position => ({0},{1})", screenX, screenY);
    wxPoint topRightScreen(screenX, screenY);
    SetPosition(topRightScreen);
}

void OutlookMeetingsViewFrame::OnRefresh(wxCommandEvent& event) {}

void OutlookMeetingsViewFrame::OnAccountChoice(wxCommandEvent& event)
{
    mMeetingModels.clear();

    if (pActiveMeetingsPanel != nullptr) {
        pScrolledWindowSizer->Detach(pActiveMeetingsPanel);
        bool windowDelete = pActiveMeetingsPanel->Destroy();
        if (!windowDelete) {
            pLogger->warn("Failed to delete active meetings panel and its child controls");
        }

        pScrolledWindowSizer->Layout();
        pMainSizer->Layout();

        SPDLOG_LOGGER_TRACE(pLogger, "Removed active meetings panel from scrolled window");
    }

    int selection = event.GetSelection();
    if (selection == 0) {
        mSelectedAccount = "";

        if (pFeedbackLabel == nullptr) {
            pFeedbackLabel =
                new wxStaticText(pThisPanel, tksIDC_FEEDBACKLABEL, "No meetings found");
            pMainSizer->Add(
                pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

            pMainSizer->Layout();
        }

        if (pRefreshButton->IsEnabled()) {
            pRefreshButton->Disable();
        }

        return;
    } else {
        mSelectedAccount = pAccountsChoiceCtrl->GetString(selection).ToStdString();
        if (!pRefreshButton->IsEnabled()) {
            pRefreshButton->Enable();
        }
    }

    SPDLOG_LOGGER_TRACE(pLogger,
        "Outlook account name selected \"{0}\"",
        mSelectedAccount.empty() ? "(none)" : mSelectedAccount);

    Services::Integrations::OutlookIntegratorService service(pLogger);
    Services::Integrations::OutlookResult result;
    {
        wxBusyCursor cursor;

        result = service.FetchCalendarMeetings(mSelectedAccount, mMeetingModels);
    }

    if (!result.Success) {
        std::string message = "Failed to fetch Outlook meetings";

        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        pFeedbackLabel->SetLabel(message);

        return;
    } else if (result.Success && !result.Message.empty()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Retrieved \"{0}\" meetings", mMeetingModels.size());

        if (mMeetingModels.size() == 0) {
            if (pFeedbackLabel == nullptr) {
                pFeedbackLabel =
                    new wxStaticText(pThisPanel, tksIDC_FEEDBACKLABEL, "No meetings found");
                pMainSizer->Add(
                    pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

                pMainSizer->Layout();
                Fit();
            }

            if (pRefreshButton->IsEnabled()) {
                pRefreshButton->Disable();
            }
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

    Persistence::AttendedMeetingsPersistence attendedMeetingsPersistence(
        pLogger, mDatabaseFilePath);

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels;
    int ret = attendedMeetingsPersistence.GetByTodaysDate(Utils::UnixTimestampTodayMidnight(),
        Utils::UnixTimestampTomorrowMidnight(),
        attendedMeetingModels);

    if (ret != 0) {
        std::string message = "Failed to get employer";
        QueueErrorNotificationEvent(message);
    }

    /* Panel Sizer */
    auto panelSizer = new wxBoxSizer(wxVERTICAL);

    /* Panel */
    pActiveMeetingsPanel = new wxPanel(pScrolledWindow, wxID_ANY);
    pActiveMeetingsPanel->SetSizer(panelSizer);

    int attendedCheckBoxControlId = tksIDC_ATTENDEDCHECKBOX_BASE;

    for (const auto& meetingModel : mMeetingModels) {
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

        /* Horizontal line */
        auto line = new wxStaticLine(staticBox, wxID_ANY);
        staticBoxSizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

        /* Attended checkbox */
        auto attendedCheckBox = new wxCheckBox(staticBox, attendedCheckBoxControlId, "Attended");
        wxStringClientData* meetingEntryIdData = new wxStringClientData(meetingModel.EntryId);
        attendedCheckBox->SetClientObject(meetingEntryIdData);

        attendedCheckBox->Bind(wxEVT_CHECKBOX,
            &OutlookMeetingsViewFrame::OnAttendedCheckBoxCheck,
            this,
            attendedCheckBoxControlId);

        attendedCheckBoxControlId++;
        staticBoxSizer->Add(attendedCheckBox, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

        auto attendedMeetingFoundIterator = std::find_if(attendedMeetingModels.begin(),
            attendedMeetingModels.end(),
            [&](const Model::AttendedMeetingModel& model) {
                return model.EntryId == meetingModel.EntryId;
            });

        if (attendedMeetingFoundIterator != attendedMeetingModels.end()) {
            attendedCheckBox->SetValue(true);
            attendedCheckBox->Disable();
        }
    }

    pScrolledWindowSizer->Add(pActiveMeetingsPanel, wxSizerFlags().Expand());
    pScrolledWindowSizer->SetSizeHints(pActiveMeetingsPanel);
    pScrolledWindowSizer->Layout();

    pMainSizer->Layout();
    pMainSizer->SetSizeHints(pThisPanel);
    Fit();

    wxSize parentWindowSize = pParent->GetSize();
    SPDLOG_LOGGER_TRACE(pLogger,
        "Parent size ({0},{1})",
        parentWindowSize.GetHeight(),
        parentWindowSize.GetWidth());

    auto currentDialogHeight = GetSize().GetHeight();
    wxSize dialogMaxSize;
    dialogMaxSize.SetHeight(parentWindowSize.GetHeight());
    dialogMaxSize.SetWidth(-1);
    SetSize(dialogMaxSize);
}

void OutlookMeetingsViewFrame::OnAttendedCheckBoxCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID \"{0}\" checked", event.GetId());
        wxWindow* wnd = dynamic_cast<wxWindow*>(event.GetEventObject());
        wxStringClientData* scd = dynamic_cast<wxStringClientData*>(wnd->GetClientObject());
        if (scd) {
            auto& s = scd->GetData();
            auto eventEntryId = s.ToStdString();
            SPDLOG_LOGGER_TRACE(
                pLogger, "Checkbox with ID \"{0}\" ENTRY_ID -> \n{1}", event.GetId(), eventEntryId);

            const auto& foundMeetingIterator = std::find_if(mMeetingModels.begin(),
                mMeetingModels.end(),
                [=](const Services::Integrations::OutlookMeetingModel& model) {
                    return model.EntryId == eventEntryId;
                });

            if (foundMeetingIterator != mMeetingModels.end()) {
                auto& meetingModel = *foundMeetingIterator;
                SPDLOG_LOGGER_TRACE(
                    pLogger, "Meeting found with detail: \n{0}", meetingModel.DebugPrint());

                auto* attendedCheckBoxCtrl = reinterpret_cast<wxCheckBox*>(wnd);

                dlg::TaskDialog meetingTaskDialog(pParent, pCfg, pLogger, mDatabaseFilePath);
                meetingTaskDialog.SetAttendedMeetingData(
                    meetingModel.TrimmedSubject(), meetingModel.Duration, meetingModel.Location);
                meetingTaskDialog.SetAttendedMeetingDataEx(meetingModel.EntryId,
                    meetingModel.TrimmedSubject(),
                    meetingModel.Start,
                    meetingModel.End,
                    meetingModel.Duration,
                    meetingModel.Location);

                int ret = meetingTaskDialog.ShowModal();

                if (attendedCheckBoxCtrl) {
                    if (ret != wxID_OK) {
                        attendedCheckBoxCtrl->SetValue(false);
                    } else {
                        attendedCheckBoxCtrl->Disable();
                    }
                }
            }
        }
    } else {
        SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID \"{0}\" unchecked", event.GetId());
    }
}

void OutlookMeetingsViewFrame::OnCancel(wxCommandEvent& WXUNUSED(event)) {}

void OutlookMeetingsViewFrame::QueueErrorNotificationEvent(const std::string& message)
{
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Error, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
}
} // namespace tks::UI::frames
