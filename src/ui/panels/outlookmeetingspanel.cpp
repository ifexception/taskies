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

#include <chrono>

#include <wx/artprov.h>
#include <wx/statline.h>

#include "../../common/common.h"
#include "../../common/enums.h"

#include "../../common/messages/persistencemessages.h"

#include "../../services/outlook/outlookclassicservice.h"

namespace tks::UI::Panel
{
OutlookMeetingsPanel::OutlookMeetingsPanel(wxWindow* parent,
    wxWindowID windowPanelId,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, windowPanelId)
    , pLogger(logger)
    , pMeetingStaticBoxSizer(nullptr)
    , pRefreshButton(nullptr)
    , pAccountsChoiceCtrl(nullptr)
    , pFeedbackLabel(nullptr)
    , pScrolledWindow(nullptr)
    , pScrolledWindowSizer(nullptr)
    , pActiveMeetingsPanel(nullptr)
    , mTodaysDate()
    , mSelectedAccount()
    , mMeetingModels()
{
    mTodaysDate = date::floor<date::days>(std::chrono::system_clock::now());

    Create();
}

OutlookMeetingsPanel::~OutlookMeetingsPanel() {}

void OutlookMeetingsPanel::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
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

void OutlookMeetingsPanel::OnRefresh(wxCommandEvent& event) {}

void OutlookMeetingsPanel::OnAccountChoice(wxCommandEvent& event)
{
    wxBusyCursor cursor;

    mMeetingModels.clear();

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
} // namespace tks::UI::Panel
