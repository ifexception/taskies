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

#include <wx/artprov.h>

namespace tks::UI
{
NotificationPopupWindow::NotificationPopupWindow(wxWindow* parent, std::shared_ptr<spdlog::logger> logger)
    : wxPopupTransientWindow(parent, wxBORDER_SIMPLE)
    , pLogger(logger)
    , pSizer(nullptr)
    , pCloseButton(nullptr)
    , pClearAllNotificationsButton(nullptr)
    , mIndex(0)
{
    CreateControls();
    ConfigureEventBindings();
}

void NotificationPopupWindow::Popup(wxWindow* WXUNUSED(focus))
{
    wxPopupTransientWindow::Popup();
}

bool NotificationPopupWindow::Show(bool show)
{
    return wxPopupTransientWindow::Show(show);
}

void NotificationPopupWindow::OnDismiss()
{
    wxPopupTransientWindow::OnDismiss();
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
    notifcationsLabel->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    /* Close button */
    auto providedCloseBitmap =
        wxArtProvider::GetBitmapBundle(wxART_CLOSE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pCloseButton = new wxBitmapButton(this, tksIDC_CLOSEBTN, providedCloseBitmap);

    titleButtonSizer->Add(notifcationsLabel, wxSizerFlags().Border(wxALL, FromDIP(4)));
    titleButtonSizer->AddStretchSpacer();
    titleButtonSizer->Add(pCloseButton, wxSizerFlags().Border(wxALL, FromDIP(2)));

    /* Clear All notifications button */
    auto clearAllSizer = new wxBoxSizer(wxHORIZONTAL);
    pSizer->Add(clearAllSizer, wxSizerFlags().Expand());

    clearAllSizer->AddStretchSpacer();
    pClearAllNotificationsButton = new wxButton(this, tksIDC_CLEARALLNOTIF, "Clear All");
    clearAllSizer->Add(pClearAllNotificationsButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Noifications */

    SetSizerAndFit(pSizer);
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
}
// clang-format on

void NotificationPopupWindow::OnClose(wxCommandEvent& event)
{
    wxPopupTransientWindow::Dismiss();
}

void NotificationPopupWindow::OnMarkAllAsRead(wxCommandEvent& event) {}

void NotificationPopupWindow::OnMarkAsRead(wxCommandEvent& event) {}
} // namespace tks::UI
