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

#include "notificationpopupwindow.h"

#include <algorithm>

#include <wx/artprov.h>
#include <wx/statline.h>

namespace tks::UI
{
NotificationPopupWindow::NotificationPopupWindow(wxWindow* parent, std::shared_ptr<spdlog::logger> logger)
    : wxPopupTransientWindow(parent, wxBORDER_SIMPLE)
    , pParent(parent)
    , pLogger(logger)
    , pSizer(nullptr)
    , pNoNotificationsPanel(nullptr)
    , pNotificationsScrolledWindow(nullptr)
    , pNotificationsScrolledWindowSizer(nullptr)
    , pCloseButton(nullptr)
    , pClearAllNotificationsButton(nullptr)
    , mNotifications()
    , mNotificationCounter(0)
{
    CreateControls();
    ConfigureEventBindings();
}

void NotificationPopupWindow::Popup(wxWindow* WXUNUSED(focus))
{
    pLogger->info("NotificationPopupWindow - Popup notification window");
    wxPopupTransientWindow::Popup();
}

bool NotificationPopupWindow::Show(bool show)
{
    pLogger->info("NotificationPopupWindow - Show notification window");
    return wxPopupTransientWindow::Show(show);
}

void NotificationPopupWindow::OnDismiss()
{
    pLogger->info("NotificationPopupWindow - Dismiss notification window");
    wxPopupTransientWindow::OnDismiss();
}

void NotificationPopupWindow::OnResize()
{
    pLogger->info("NotificationPopupWindow - Resize event received from parent window");
    wxSize notificationWindowSize;
    if (pParent->GetClientSize().GetWidth() < 800) {
        pLogger->info("NotificationPopupWindow - Parent window has gone below 800 pixels in width");
        notificationWindowSize.SetWidth(FromDIP(200));
    } else {
        auto result = static_cast<int>(pParent->GetClientSize().GetWidth() * NOTIFICATION_WINDOW_X_SCALE_FACTOR);
        notificationWindowSize.SetWidth(FromDIP(result));
    }
    notificationWindowSize.SetHeight(FromDIP(pParent->GetClientSize().GetY() - NOTIFICATION_WINDOW_Y_SCALE_OFFSET));
    SetSize(notificationWindowSize);

    for (auto& notification : mNotifications) {
        notification.ControlMessage->SetLabel("");
        notification.ControlMessage->SetLabel(notification.Message);

        auto wrapThreshold = GetClientSize().GetWidth() - NOTIFICATION_MESSAGE_WRAP_WIDTH_OFFSET;
        notification.ControlMessage->Wrap(wrapThreshold);
    }

    pNotificationsScrolledWindowSizer->Layout();
    pSizer->Layout();

    pLogger->info("NotificationPopupWindow - Resized to new dimensions: [ \"x\": \"{0}\", \"y\": \"{1}\" ]",
        notificationWindowSize.GetX(),
        notificationWindowSize.GetY());
}

void NotificationPopupWindow::AddNotification(const std::string& message, NotificationType type)
{
    pLogger->info("NotificationPopupWindow - Add notification with message: \"{0}\" and type: \"{1}\"",
        message,
        NotificationTypeToString(type));
    Notification n;
    n.Message = message;
    n.Order = ++mNotificationCounter;
    n.CloseButtonIndex = tksIDC_MARKASREADBASE + mNotificationCounter;
    n.Panel = nullptr;

    if (pNoNotificationsPanel->IsEnabled()) {
        pNoNotificationsPanel->HideWithEffect(wxShowEffect::wxSHOW_EFFECT_ROLL_TO_BOTTOM);
        pNoNotificationsPanel->Disable();

        pSizer->Layout();
    }

    pLogger->info(
        "NotificationPopupWindow - Create notification with attributes - Order: \"{0}\" | CloseButtonIndex: \"{1}\"",
        n.Order,
        n.CloseButtonIndex);
    AddNotificationMessageWithControls(n, type);
}

void NotificationPopupWindow::CreateControls()
{
    /* Sizer */
    pSizer = new wxBoxSizer(wxVERTICAL);

    /* Close window and title sizer */
    auto titleButtonSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(titleButtonSizer, wxSizerFlags().Expand());

    /* Notifications title static text */
    auto notifcationsLabel = new wxStaticText(this, wxID_ANY, "Notifications");
    notifcationsLabel->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    /* Close button */
    auto providedCloseBitmap =
        wxArtProvider::GetBitmapBundle(wxART_CLOSE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pCloseButton = new wxBitmapButton(this, tksIDC_CLOSEBTN, providedCloseBitmap);
    pCloseButton->SetToolTip("Close notifications window");

    titleButtonSizer->Add(notifcationsLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    titleButtonSizer->AddStretchSpacer();
    titleButtonSizer->Add(pCloseButton, wxSizerFlags().Border(wxALL, FromDIP(2)));

    /* Clear All notifications button */
    auto clearAllSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(clearAllSizer, wxSizerFlags().Expand());

    clearAllSizer->AddStretchSpacer();
    pClearAllNotificationsButton = new wxButton(this, tksIDC_CLEARALLNOTIFICATIONS, "Clear All");
    pClearAllNotificationsButton->SetToolTip("Mark all as read");

    clearAllSizer->Add(pClearAllNotificationsButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Static Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    pSizer->Add(line, wxSizerFlags().Expand());

    /* No Noifications Panel */
    pNoNotificationsPanel = new wxPanel(this, wxID_ANY);

    auto noNotificationsPanelSizer = new wxBoxSizer(wxVERTICAL);
    pNoNotificationsPanel->SetSizer(noNotificationsPanelSizer);

    auto noNotificationsText = new wxStaticText(pNoNotificationsPanel, wxID_ANY, "No Notifications");
    noNotificationsText->SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    noNotificationsPanelSizer->Add(noNotificationsText, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterHorizontal());

    pSizer->Add(pNoNotificationsPanel, wxSizerFlags().Expand());

    /* Scrolled notifications panel */
    pNotificationsScrolledWindow = new wxScrolledWindow(this, wxID_ANY);
    pNotificationsScrolledWindowSizer = new wxBoxSizer(wxVERTICAL);
    pNotificationsScrolledWindow->SetSizer(pNotificationsScrolledWindowSizer);
    pNotificationsScrolledWindow->SetScrollRate(0, 20);
    pNotificationsScrolledWindowSizer->FitInside(pNotificationsScrolledWindow);

    pSizer->Add(pNotificationsScrolledWindow, wxSizerFlags().Expand().Proportion(1));

    pNotificationsScrolledWindow->Disable();
    pNotificationsScrolledWindow->Hide();

    SetSizer(pSizer);

    wxSize notificationWindowSize;
    if (pParent->GetClientSize().GetWidth() < 100) {
        notificationWindowSize.SetWidth(FromDIP(100));
    } else {
        auto result = static_cast<int>(pParent->GetClientSize().GetWidth() * NOTIFICATION_WINDOW_X_SCALE_FACTOR);
        notificationWindowSize.SetWidth(FromDIP(result));
    }
    notificationWindowSize.SetHeight(FromDIP(pParent->GetClientSize().GetY() - NOTIFICATION_WINDOW_Y_SCALE_OFFSET));
    SetSize(notificationWindowSize);
}

// clang-format off
void NotificationPopupWindow::ConfigureEventBindings()
{
    pCloseButton->Bind(
        wxEVT_BUTTON,
        &NotificationPopupWindow::OnClose,
        this,
        tksIDC_CLOSEBTN
    );

    pClearAllNotificationsButton->Bind(
        wxEVT_BUTTON,
        &NotificationPopupWindow::OnMarkAllAsRead,
        this,
        tksIDC_CLEARALLNOTIFICATIONS
    );
}
// clang-format on

void NotificationPopupWindow::OnClose(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("NotificationPopupWindow - Dismiss notification window");
    wxPopupTransientWindow::Dismiss();
}

void NotificationPopupWindow::OnMarkAllAsRead(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("NotificationPopupWindow - Removing all notifications. Count: \"{0}\"", mNotifications.size());
    for (auto& notification : mNotifications) {
        bool ret = notification.Panel->HideWithEffect(wxShowEffect::wxSHOW_EFFECT_SLIDE_TO_BOTTOM);
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to hide panel");
            return;
        }
        ret = pNotificationsScrolledWindowSizer->Detach(notification.Panel);
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to detach panel from main sizer");
            return;
        }
        ret = notification.Panel->Destroy();
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to destroy panel");
            return;
        }
    }

    pLogger->info("NotificationPopupWindow - Removed all notifications");

    pSizer->Layout();
    mNotifications.clear();

    pNotificationsScrolledWindow->Disable();
    pNotificationsScrolledWindow->Hide();

    pNoNotificationsPanel->ShowWithEffect(wxShowEffect::wxSHOW_EFFECT_SLIDE_TO_BOTTOM);
    pNoNotificationsPanel->Enable();
    pSizer->Layout();
}

void NotificationPopupWindow::OnMarkAsRead(wxCommandEvent& event)
{
    pLogger->info("NotificationPopupWindow - Mark as read on notification with ID: \"{0}\"", event.GetId());

    auto buttonId = event.GetId();
    auto it = std::find_if(mNotifications.begin(), mNotifications.end(), [&](const Notification& notification) {
        return notification.CloseButtonIndex == buttonId;
    });

    if (it != std::end(mNotifications)) {
        auto& notification = *it;
        bool ret = notification.Panel->HideWithEffect(wxShowEffect::wxSHOW_EFFECT_SLIDE_TO_BOTTOM);
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to hide panel");
            return;
        }
        ret = pNotificationsScrolledWindowSizer->Detach(notification.Panel);
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to detach panel from main sizer");
            return;
        }
        ret = notification.Panel->Destroy();
        if (!ret) {
            pLogger->error("NotificationPopupWindow - Failed to destroy panel");
            return;
        }

        pNotificationsScrolledWindowSizer->Layout();
        pSizer->Layout();
        mNotifications.erase(it);
        pLogger->info("NotificationPopupWindow - Removed notification with ID \"{0}\"", buttonId);
    }

    if (mNotifications.empty()) {
        pNotificationsScrolledWindow->Disable();
        pNotificationsScrolledWindow->Hide();

        pNoNotificationsPanel->ShowWithEffect(wxShowEffect::wxSHOW_EFFECT_SLIDE_TO_BOTTOM);
        pNoNotificationsPanel->Enable();
        pSizer->Layout();
    }
}

void NotificationPopupWindow::AddNotificationMessageWithControls(Notification& notification, NotificationType type)
{
    if (!pNotificationsScrolledWindow->IsEnabled()) {
        pNotificationsScrolledWindow->Enable();
        pNotificationsScrolledWindow->Show();
    }

    /* Panel Sizer */
    auto panelSizer = new wxBoxSizer(wxVERTICAL);

    /* Panel */
    auto panel = new wxPanel(pNotificationsScrolledWindow, wxID_ANY);
    panel->SetSizer(panelSizer);

    auto notificationBox = new wxStaticBox(panel, wxID_ANY, wxEmptyString);
    auto notificationBoxSizer = new wxStaticBoxSizer(notificationBox, wxVERTICAL);
    panelSizer->Add(notificationBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Panel Header */
    auto headerSizer = new wxBoxSizer(wxHORIZONTAL);

    /* Close button */
    auto providedCloseBitmap =
        wxArtProvider::GetBitmapBundle(wxART_CLOSE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    auto closeNotificationButton =
        new wxBitmapButton(notificationBox, notification.CloseButtonIndex, providedCloseBitmap);
    closeNotificationButton->SetToolTip("Mark as read");
    Connect(notification.CloseButtonIndex,
        wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(NotificationPopupWindow::OnMarkAsRead));

    headerSizer->AddStretchSpacer();
    headerSizer->Add(closeNotificationButton, wxSizerFlags().Border(wxALL, FromDIP(2)));

    notificationBoxSizer->Add(headerSizer, wxSizerFlags().Expand());

    /* Static line */
    auto line = new wxStaticLine(notificationBox, wxID_ANY);
    notificationBoxSizer->Add(line, wxSizerFlags().Expand());

    /* Panel Body */
    auto bodySizer = new wxBoxSizer(wxHORIZONTAL);

    /* Notification Type Bitmap */
    wxBitmapBundle providedBitmap;
    switch (type) {
    case NotificationType::Information:
        providedBitmap =
            wxArtProvider::GetBitmapBundle(wxART_INFORMATION, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
        break;
    case NotificationType::Error:
        providedBitmap = wxArtProvider::GetBitmapBundle(wxART_ERROR, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
        break;
    }
    auto typeBitmap = new wxStaticBitmap(notificationBox, wxID_ANY, providedBitmap);

    /* Message Text */
    auto typeMessageText = new wxStaticText(notificationBox, wxID_ANY, notification.Message);
    auto wrapThreshold = GetClientSize().GetWidth() - NOTIFICATION_MESSAGE_WRAP_WIDTH_OFFSET;
    typeMessageText->Wrap(wrapThreshold);

    bodySizer->Add(typeBitmap, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    bodySizer->Add(typeMessageText, wxSizerFlags().Border(wxALL, FromDIP(2)).CenterVertical().Proportion(1));

    notificationBoxSizer->Add(bodySizer, wxSizerFlags().Expand());

    pNotificationsScrolledWindowSizer->Add(panel, wxSizerFlags().Expand());

    pNotificationsScrolledWindowSizer->Layout();
    pSizer->Layout();

    notification.ControlMessage = typeMessageText;
    notification.Panel = panel;
    mNotifications.push_back(notification);
}
} // namespace tks::UI
