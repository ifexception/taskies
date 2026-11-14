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

#include "debugoptionsminiframe.h"

namespace tks::UI
{
DebugOptionsMiniFrame::DebugOptionsMiniFrame(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger)
    : wxMiniFrame(parent,
          wxID_ANY,
          "Debug Options",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP)
    , pParent(parent)
    , pLogger(logger)
    , pDebugDefaultCheckBoxCtrl(nullptr)
    , pConfirmButton(nullptr)
    , pCancelButton(nullptr)
    , pEventLoop(nullptr)
    , mReturnCode(wxID_CANCEL)
{
    CreateControls();
    ConfigureEventBindings();
}

void DebugOptionsMiniFrame::ConfigureEventBindings()
{
    Bind(wxEVT_CLOSE_WINDOW, &DebugOptionsMiniFrame::OnClose, this);

    pDebugDefaultCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX, &DebugOptionsMiniFrame::OnDebugDefaultCheckBoxCtrl, this);

    pConfirmButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { EndModal(wxID_OK); });

    pCancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { EndModal(wxID_CANCEL); });
}

void DebugOptionsMiniFrame::OnClose(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_EXIT);
}

void DebugOptionsMiniFrame::CreateControls()
{
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxPanel* panel = new wxPanel(this);
    panel->SetSizer(sizer);

    pDebugDefaultCheckBoxCtrl =
        new wxCheckBox(panel, tksIDC_DEBUGDEFAULTCHECKBOXCTRL, "Default configuration");
    sizer->Add(pDebugDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    sizer->AddStretchSpacer();

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pConfirmButton = new wxButton(panel, wxID_OK, "OK");
    pConfirmButton->SetDefault();

    pCancelButton = new wxButton(panel, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pConfirmButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    panel->Fit();
}

void DebugOptionsMiniFrame::FillControls()
{
    pDebugDefaultCheckBoxCtrl->SetValue(true);
}

void DebugOptionsMiniFrame::OnDebugDefaultCheckBoxCtrl(wxCommandEvent& event) {}

int DebugOptionsMiniFrame::ShowModal()
{
    Show(true);
    Raise();

    pEventLoop = new wxEventLoop();
    pEventLoop->Run();

    delete pEventLoop;
    pEventLoop = nullptr;

    return mReturnCode;
}

void DebugOptionsMiniFrame::EndModal(int returnCode)
{
    mReturnCode = returnCode;
    Hide();

    if (pEventLoop && pEventLoop->IsRunning()) {
        pEventLoop->Exit();
    }
}
} // namespace tks::UI
