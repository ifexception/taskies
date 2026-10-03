// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "outlookmeetingspanel.h"

#include <algorithm>
#include <chrono>

#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/richmsgdlg.h>
#include <wx/statline.h>

#include "../dlg/taskdlg.h"

#include "../../common/common.h"
#include "../../common/enums.h"

#include "../../common/messages/persistencemessages.h"

#include "../../models/attendedmeetingmodel.h"

#include "../../persistence/attendedmeetingspersistence.h"

#include "../../services/outlook/outlookclassicservice.h"

#include "../../utils/dateutils.h"

namespace tks::UI::Panel
{
OutlookMeetingsPanel::OutlookMeetingsPanel(wxWindow* parent,
    wxWindowID windowPanelId,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : wxPanel(parent, windowPanelId)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pMeetingStaticBoxSizer(nullptr)
    , pRefreshButton(nullptr)
    , pAccountsChoiceCtrl(nullptr)
    , pFeedbackLabel(nullptr)
    , pScrolledWindow(nullptr)
    , pScrolledWindowSizer(nullptr)
    , pActiveMeetingsPanel(nullptr)
    , mSelectedDate()
    , mSelectedAccount()
{
    mSelectedDate = date::floor<date::days>(std::chrono::system_clock::now());

    Create();
}

OutlookMeetingsPanel::~OutlookMeetingsPanel() {}

void OutlookMeetingsPanel::OnDateChanged(date::sys_days newDate)
{
    mSelectedDate = newDate;

    wxBusyCursor cursor;

    if (mSelectedAccount.empty()) {
        ResetFeedbackLabelOnNoData();

        return;
    }

    mOutlookMeetings.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    mOutlookMeetings = FetchOutlookMeetingsByAccountName(mSelectedAccount);
    if (mOutlookMeetings.size() == 0) {
        ResetFeedbackLabelOnNoData("No meetings found");

        return;
    }

    if (pFeedbackLabel && pFeedbackLabel->IsShown()) {
        pFeedbackLabel->Hide();
        pMeetingStaticBoxSizer->Layout();
    }

    auto attendedMeetings = FetchAttendedMeetingsByDate();

    AddMeetingsToPanel(mOutlookMeetings, attendedMeetings);
}

void OutlookMeetingsPanel::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
    DataToControls();
}

void OutlookMeetingsPanel::CreateControls()
{
    /* Outlook Meetings Box Sizer */
    auto meetingStaticBox = new wxStaticBox(this, wxID_ANY, "Outlook");
    pMeetingStaticBoxSizer = new wxStaticBoxSizer(meetingStaticBox, wxVERTICAL);
    SetSizer(pMeetingStaticBoxSizer);

    /* Refresh button */
    auto providedRefreshBitmap = wxArtProvider::GetBitmapBundle(
        wxART_REFRESH, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pRefreshButton =
        new wxBitmapButton(meetingStaticBox, tksIDC_REFRESH_BUTTON, providedRefreshBitmap);
    pRefreshButton->SetToolTip("Refresh meetings of selected account");
    pRefreshButton->Disable();
    pMeetingStaticBoxSizer->Add(pRefreshButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    /* Horizontal Line */
    auto line = new wxStaticLine(meetingStaticBox, wxID_ANY);
    pMeetingStaticBoxSizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* Account label and choice control */
    auto accountLabel = new wxStaticText(meetingStaticBox, wxID_ANY, "Account");

    pAccountsChoiceCtrl = new wxChoice(meetingStaticBox, tksIDC_ACCOUNT_CHOICE_CTRL);
    pAccountsChoiceCtrl->SetToolTip("Select an account to fetch meetings");

    pMeetingStaticBoxSizer->Add(accountLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    pMeetingStaticBoxSizer->Add(
        pAccountsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Feedback label */
    pFeedbackLabel =
        new wxStaticText(meetingStaticBox, tksIDC_FEEDBACKLABEL, "No account selected");
    pMeetingStaticBoxSizer->Add(
        pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal().Top());

    /* Main Scrolled Window */
    pScrolledWindow = new wxScrolledWindow(meetingStaticBox, wxID_ANY);
    pScrolledWindowSizer = new wxBoxSizer(wxVERTICAL);
    pScrolledWindow->SetSizer(pScrolledWindowSizer);
    pScrolledWindow->SetScrollRate(0, 20);
    pScrolledWindowSizer->FitInside(pScrolledWindow);

    pMeetingStaticBoxSizer->Add(pScrolledWindow, wxSizerFlags(1).Expand());
}

void OutlookMeetingsPanel::FillControls()
{
    pAccountsChoiceCtrl->Append("Select account");
    pAccountsChoiceCtrl->SetSelection(0);
}

void OutlookMeetingsPanel::DataToControls()
{
    std::vector<std::string> accountNames;

    Services::Outlook::OutlookClassicService outlookClassicService(pLogger);
    Services::Outlook::OutlookResult result;
    {
        wxBusyCursor cursor;

        result = outlookClassicService.FetchAccountNames(accountNames);
    }

    if (!result.Success) {
        std::string message = "Failed to fetch Outlook accounts";
        pFeedbackLabel->SetLabel(message);

        wxMessageDialog dialog(this,
            message,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(result.Message);

        dialog.ShowModal();

        return;
    }

    for (const std::string& accountName : accountNames) {
        pAccountsChoiceCtrl->Append(accountName);
    }
}

// clang-format off
void OutlookMeetingsPanel::ConfigureEventBindings()
{
    pRefreshButton->Bind(
        wxEVT_BUTTON,
        &OutlookMeetingsPanel::OnRefresh,
        this,
        tksIDC_REFRESH_BUTTON
    );

    pAccountsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &OutlookMeetingsPanel::OnAccountChoice,
        this
    );
}
// clang-format on

void OutlookMeetingsPanel::OnRefresh(wxCommandEvent& event)
{
    wxBusyCursor cursor;

    if (mSelectedAccount.empty()) {
        ResetFeedbackLabelOnNoData();

        return;
    }

    mOutlookMeetings.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    mOutlookMeetings = FetchOutlookMeetingsByAccountName(mSelectedAccount);
    if (mOutlookMeetings.size() == 0) {
        ResetFeedbackLabelOnNoData("No meetings found");

        return;
    }

    if (pFeedbackLabel && pFeedbackLabel->IsShown()) {
        pFeedbackLabel->Hide();
        pMeetingStaticBoxSizer->Layout();
    }

    auto attendedMeetings = FetchAttendedMeetingsByDate();

    AddMeetingsToPanel(mOutlookMeetings, attendedMeetings);
}

void OutlookMeetingsPanel::OnAccountChoice(wxCommandEvent& event)
{
    wxBusyCursor cursor;

    mOutlookMeetings.clear();

    if (pActiveMeetingsPanel != nullptr) {
        RemoveActiveMeetingsPanel();
    }

    int selection = event.GetSelection();
    if (selection == 0) {
        ResetFeedbackLabelOnNoData();
        mSelectedAccount.clear();

        return;
    } else {
        mSelectedAccount = pAccountsChoiceCtrl->GetString(selection).ToStdString();
        if (!pRefreshButton->IsEnabled()) {
            pRefreshButton->Enable();
        }
    }

    mOutlookMeetings = FetchOutlookMeetingsByAccountName(mSelectedAccount);
    if (mOutlookMeetings.size() == 0) {
        ResetFeedbackLabelOnNoData("No meetings found");

        return;
    }

    if (pFeedbackLabel && pFeedbackLabel->IsShown()) {
        pFeedbackLabel->Hide();
        pMeetingStaticBoxSizer->Layout();
    }

    auto attendedMeetings = FetchAttendedMeetingsByDate();

    AddMeetingsToPanel(mOutlookMeetings, attendedMeetings);
}

void OutlookMeetingsPanel::OnAttendedCheckBoxCheck(wxCommandEvent& event)
{
    if (!event.IsChecked()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID \"{0}\" unchecked", event.GetId());
        return;
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Checkbox with ID: \"{0}\" checked", event.GetId());
    wxWindow* windowPtr = dynamic_cast<wxWindow*>(event.GetEventObject());
    wxStringClientData* windowStringClientDataPtr =
        dynamic_cast<wxStringClientData*>(windowPtr->GetClientObject());

    if (!windowStringClientDataPtr) {
        return;
    }

    wxWindowID checkboxId = event.GetId();
    SPDLOG_LOGGER_TRACE(pLogger, "Window ID \"{0}\"", checkboxId);

    auto& stringData = windowStringClientDataPtr->GetData();
    auto eventMeetingEntryId = stringData.ToStdString();
    SPDLOG_LOGGER_TRACE(pLogger,
        "Checkbox with ID: \"{0}\" and ENTRY_ID -> \n{1}",
        checkboxId,
        eventMeetingEntryId);

    const auto& foundMeetingIterator = std::find_if(mOutlookMeetings.begin(),
        mOutlookMeetings.end(),
        [=](const Services::Outlook::OutlookMeetingModel& model) {
            return model.EntryId == eventMeetingEntryId;
        });

    if (foundMeetingIterator == mOutlookMeetings.end()) {
        pLogger->warn("Could not find matching Outlook meeting with entry id from event \"{0}\"",
            eventMeetingEntryId);
        return;
    }

    auto& meetingModel = *foundMeetingIterator;
    SPDLOG_LOGGER_TRACE(pLogger, "Meeting found with detail: \n{0}", meetingModel.DebugPrint());

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

    wxCheckBox* attendedCheckBoxCtrl = dynamic_cast<wxCheckBox*>(windowPtr);
    if (attendedCheckBoxCtrl) {
        if (ret != wxID_OK) {
            attendedCheckBoxCtrl->SetValue(false);
        } else {
            attendedCheckBoxCtrl->Disable();
        }
    } else {
        pLogger->warn("CheckBox control is NULL. Failed to perform dynamic cast to get attended "
                      "check box control");
    }
}

void OutlookMeetingsPanel::RemoveActiveMeetingsPanel()
{
    pScrolledWindowSizer->Detach(pActiveMeetingsPanel);
    bool windowDelete = pActiveMeetingsPanel->Destroy();
    if (!windowDelete) {
        pLogger->warn("Failed to delete active meetings panel and its child controls");
    }
    pActiveMeetingsPanel = nullptr;

    pScrolledWindowSizer->Layout();
    pMeetingStaticBoxSizer->Layout();

    SPDLOG_LOGGER_TRACE(pLogger, "Removed active meetings panel from scrolled window");
}

void OutlookMeetingsPanel::ResetFeedbackLabelOnNoData(const std::string& message)
{
    const std::string feedbackMessage = message.empty() ? "No account selected" : message;

    if (pFeedbackLabel == nullptr) {
        pFeedbackLabel = new wxStaticText(this, tksIDC_FEEDBACKLABEL, feedbackMessage);

        const int FeedbackLabelSizerIndex = 4;
        if (pMeetingStaticBoxSizer->GetItemCount() >= FeedbackLabelSizerIndex) {
            pMeetingStaticBoxSizer->Insert(FeedbackLabelSizerIndex,
                pFeedbackLabel,
                wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal().Top());
        } else {
            pMeetingStaticBoxSizer->Add(
                pFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal().Top());
        }
    } else {
        pFeedbackLabel->SetLabel(feedbackMessage);
        pFeedbackLabel->Show();
    }

    if (pRefreshButton->IsEnabled()) {
        pRefreshButton->Disable();
    }

    pMeetingStaticBoxSizer->Layout();
}

std::vector<Services::Outlook::OutlookMeetingModel>
    OutlookMeetingsPanel::FetchOutlookMeetingsByAccountName(const std::string& accountName)
{
    std::vector<Services::Outlook::OutlookMeetingModel> meetingModels;

    SPDLOG_LOGGER_TRACE(pLogger,
        "Outlook account name selected \"{0}\"",
        accountName.empty() ? "(none)" : accountName);

    Services::Outlook::OutlookClassicService service(pLogger);
    Services::Outlook::OutlookResult result = service.FetchCalendarMeetings(
        accountName, date::format("%F", mSelectedDate), meetingModels);

    if (!result.Success) {
        wxMessageDialog dialog(this,
            "Failed to fetch Outlook meetings for selected account",
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(result.Message);

        dialog.ShowModal();
    }

    return meetingModels;
}

std::vector<Model::AttendedMeetingModel> OutlookMeetingsPanel::FetchAttendedMeetingsByDate()
{
    Persistence::AttendedMeetingsPersistence attendedMeetingsPersistence(
        pLogger, mDatabaseFilePath);

    std::vector<Model::AttendedMeetingModel> attendedMeetingModels;
    auto sqliteResult =
        attendedMeetingsPersistence.GetByDate(Utils::UnixTimestampMidnight(mSelectedDate),
            Utils::UnixTimestampNextDayMidnight(mSelectedDate),
            attendedMeetingModels);

    if (!sqliteResult.Success) {
        wxRichMessageDialog dialog(this,
            Messages::FilterAttendedMeetingsByTodayDateMessage,
            Common::GetProgramName(),
            wxCENTER | wxCANCEL_DEFAULT | wxOK | wxCANCEL | wxICON_ERROR);
        dialog.SetExtendedMessage(sqliteResult.FriendlyErrorMessage);
        dialog.ShowDetailedText(sqliteResult.GetReturnCodeAndMessage());

        dialog.ShowModal();
    }

    return attendedMeetingModels;
}

void OutlookMeetingsPanel::AddMeetingsToPanel(
    const std::vector<Services::Outlook::OutlookMeetingModel>& outlookMeetings,
    const std::vector<Model::AttendedMeetingModel>& attendedMeetings)
{
    /* Panel Sizer */
    auto panelSizer = new wxBoxSizer(wxVERTICAL);

    /* Panel */
    pActiveMeetingsPanel = new wxPanel(pScrolledWindow, wxID_ANY);
    pActiveMeetingsPanel->SetSizer(panelSizer);

    int attendedCheckBoxControlId = tksIDC_ATTENDEDCHECKBOX_BASE;

    for (const auto& outlookMeeting : outlookMeetings) {
        bool outlookMeetingAttended = false;

        auto attendedMeetingFoundIterator = std::find_if(attendedMeetings.begin(),
            attendedMeetings.end(),
            [&](const Model::AttendedMeetingModel& attendedMeeting) {
                return attendedMeeting.EntryId == outlookMeeting.EntryId;
            });

        if (attendedMeetingFoundIterator != attendedMeetings.end()) {
            outlookMeetingAttended = true;
        }

        BuildMeetingControlsToPanel(
            panelSizer, attendedCheckBoxControlId, outlookMeeting, outlookMeetingAttended);

        ++attendedCheckBoxControlId;
    }

    pScrolledWindowSizer->Add(pActiveMeetingsPanel, wxSizerFlags().Expand());
    pScrolledWindowSizer->SetSizeHints(pActiveMeetingsPanel);
    pScrolledWindowSizer->Layout();

    pMeetingStaticBoxSizer->Layout();
}

void OutlookMeetingsPanel::BuildMeetingControlsToPanel(wxBoxSizer* panelSizer,
    int attendedCheckBoxControlId,
    const Services::Outlook::OutlookMeetingModel& meetingModel,
    bool meetingAttended)
{
    auto staticBox = new wxStaticBox(pActiveMeetingsPanel, wxID_ANY, "");
    auto staticBoxSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);
    panelSizer->Add(staticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto flexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    flexGridSizer->AddGrowableCol(1, 1);
    staticBoxSizer->Add(flexGridSizer, wxSizerFlags().Expand().Proportion(1));

    auto subjectLabel = new wxStaticText(staticBox, wxID_ANY, "Subject");
    auto subjectText = new wxTextCtrl(
        staticBox, wxID_ANY, meetingModel.Subject, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    auto durationWithTimeLabel = new wxStaticText(staticBox, wxID_ANY, "Duration");
    auto formattedValue = fmt::format(
        "{0} ({1} -- {2})", meetingModel.Duration, meetingModel.Start, meetingModel.End);
    auto durationWithTimeLabelValue = new wxTextCtrl(
        staticBox, wxID_ANY, formattedValue, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    auto locationLabel = new wxStaticText(staticBox, wxID_ANY, "Location");
    auto locationLabelValue = new wxTextCtrl(staticBox,
        wxID_ANY,
        meetingModel.Location,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_READONLY);

    flexGridSizer->Add(subjectLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(subjectText, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    flexGridSizer->Add(
        durationWithTimeLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(
        durationWithTimeLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    flexGridSizer->Add(locationLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridSizer->Add(locationLabelValue, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Horizontal line */
    auto line2 = new wxStaticLine(staticBox, wxID_ANY);
    staticBoxSizer->Add(line2, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Attended checkbox */
    auto attendedCheckBox = new wxCheckBox(staticBox, attendedCheckBoxControlId, "Attended");

    wxStringClientData* meetingEntryIdData = new wxStringClientData(meetingModel.EntryId);
    attendedCheckBox->SetClientObject(meetingEntryIdData);

    attendedCheckBox->Bind(wxEVT_CHECKBOX,
        &OutlookMeetingsPanel::OnAttendedCheckBoxCheck,
        this,
        attendedCheckBoxControlId);

    staticBoxSizer->Add(attendedCheckBox, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    if (meetingAttended) {
        attendedCheckBox->SetValue(true);
        attendedCheckBox->Disable();
    }
}
} // namespace tks::UI::Panel
