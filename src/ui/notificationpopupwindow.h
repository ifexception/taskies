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

#include <stack>
#include <string>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/popupwin.h>

#include <spdlog/logger.h>

namespace tks::UI
{
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

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnClose(wxCommandEvent& event);
    void OnMarkAllAsRead(wxCommandEvent& event);
    void OnMarkAsRead(wxCommandEvent& event);

    void AddNotificationMessageWithControls();

    std::shared_ptr<spdlog::logger> pLogger;

    wxSizer* pSizer;
    wxPanel* pNoNotificationsPanel;
    wxBitmapButton* pCloseButton;
    wxButton* pClearAllNotificationsButton;

    struct Notification {
        wxPanel* Panel;
        std::string Message;
        int Order;
        int CloseButtonIndex;
    };

    std::vector<Notification> mNotifications;

    enum { tksIDC_CLOSEBTN = wxID_HIGHEST + 100, tksIDC_CLEARALLNOTIF, tksIDC_MARKASREADBASE };
};
} // namespace tks::UI
