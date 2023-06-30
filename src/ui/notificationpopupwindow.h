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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/popupwin.h>

namespace tks::UI
{
class NotificationPopupWindow final : public wxPopupTransientWindow
{
public:
    NotificationPopupWindow() = delete;
    NotificationPopupWindow(const NotificationPopupWindow&) = delete;
    NotificationPopupWindow(wxWindow* parent);
    virtual ~NotificationPopupWindow() = default;

    NotificationPopupWindow& operator=(const NotificationPopupWindow&) = delete;

    void Popup(wxWindow* focus) override;
    bool Show(bool show = true) override;

    void OnDismiss() override;

private:
    void CreateControls();
    void ConfigureEventBindings();

    wxButton* pClearAllNotificationsButton;
    std::vector<wxButton*> pMarkAsReadButtons;

    enum {
        tksIDC_CLEARALLNOTIF = wxID_HIGHEST + 100
        tksIDC_MARKASREADBASE
    };
};
}
