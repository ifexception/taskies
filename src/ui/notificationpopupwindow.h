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

#pragma once

#include <string>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/popupwin.h>

#include <spdlog/logger.h>

#include "../common/enums.h"

namespace tks::UI
{
#define NOTIFICATION_WINDOW_X_SCALE_FACTOR 0.25
#define NOTIFICATION_WINDOW_Y_SCALE_OFFSET 86
#define NOTIFICATION_MESSAGE_WRAP_WIDTH_OFFSET 40

class NotificationPopupWindow final : public wxPopupTransientWindow
{
public:
    NotificationPopupWindow() = delete;
    NotificationPopupWindow(const NotificationPopupWindow&) = delete;
    NotificationPopupWindow(wxWindow* parent, std::shared_ptr<spdlog::logger> logger);
    virtual ~NotificationPopupWindow() = default;

    NotificationPopupWindow& operator=(const NotificationPopupWindow&) = delete;

    void Popup(wxWindow* focus = nullptr) override;
    bool Show(bool show = true) override;

    void OnDismiss() override;
    void OnResize();

    void AddNotification(const std::string& message, NotificationType status);

private:
    struct Notification {
        std::string Message;
        wxStaticText* ControlMessage;
        wxPanel* Panel;
        int Order;
        int CloseButtonIndex;
    };

    void CreateControls();
    void ConfigureEventBindings();

    void OnClose(wxCommandEvent& event);
    void OnMarkAllAsRead(wxCommandEvent& event);
    void OnMarkAsRead(wxCommandEvent& event);

    void AddNotificationMessageWithControls(Notification& notification, NotificationType status);

    std::shared_ptr<spdlog::logger> pLogger;

    wxWindow* pParent;
    wxSizer* pSizer;
    wxPanel* pNoNotificationsPanel;
    wxScrolledWindow* pNotificationsScrolledWindow;
    wxSizer* pNotificationsScrolledWindowSizer;
    wxBitmapButton* pCloseButton;
    wxButton* pClearAllNotificationsButton;
    std::vector<Notification> mNotifications;
    int mNotificationCounter;

    enum { tksIDC_MARKASREADBASE = wxID_HIGHEST + 101, tksIDC_CLOSEBTN, tksIDC_CLEARALLNOTIFICATIONS };
};
} // namespace tks::UI
