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

#include "../common/common.h"

namespace tks::UI
{
DebugOptionsMiniFrame::DebugOptionsMiniFrame(wxWindow* parent,
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Core::Environment> env)
    : wxMiniFrame(parent,
          wxID_ANY,
          "Debug Options",
          wxDefaultPosition,
          wxSize(400, 290),
          wxCAPTION | wxCLOSE_BOX | wxSTAY_ON_TOP)
    , pParent(parent)
    , pLogger(logger)
    , pEnv(env)
    , pDebugDefaultCheckBoxCtrl(nullptr)
    , pDebugReleasePortableCheckBoxCtrl(nullptr)
    , pSetupOptionsRadioBoxCtrl(nullptr)
    , pConfirmButton(nullptr)
    , pCancelButton(nullptr)
    , pEventLoop(nullptr)
    , mReturnCode(wxID_CANCEL)
    , mRadioBoxSetupOptions()
{
    std::string title = fmt::format("{0} Debug Options", Common::GetProgramName());
    SetTitle(title);

    mRadioBoxSetupOptions.Add("Default program setup");
    mRadioBoxSetupOptions.Add("Portable program setup");
    mRadioBoxSetupOptions.Add("New install setup");

    CreateControls();
    FillControls();
    ConfigureEventBindings();
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

    /* Release options static box */
    wxStaticBox* releaseOptionsStaticBox =
        new wxStaticBox(panel, wxID_ANY, "(Debug) Release Options");
    wxStaticBoxSizer* releaseOptionsStaticBoxSizer =
        new wxStaticBoxSizer(releaseOptionsStaticBox, wxVERTICAL);
    sizer->Add(releaseOptionsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Debug release program files */
    pDebugReleaseProgramFilesCheckBoxCtrl = new wxCheckBox(releaseOptionsStaticBox,
        tksIDC_DEBUGRELEASEPROGRAMFILESCHECKBOXCTRL,
        "Release configuration (Program Files)");
    releaseOptionsStaticBoxSizer->Add(
        pDebugReleaseProgramFilesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Debug release portable */
    pDebugReleasePortableCheckBoxCtrl = new wxCheckBox(releaseOptionsStaticBox,
        tksIDC_DEBUGRELEASEPORTABLECHECKBOXCTRL,
        "Release configuration (Portable)");
    releaseOptionsStaticBoxSizer->Add(
        pDebugReleasePortableCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Setup options Radio Box Ctrl */
    pSetupOptionsRadioBoxCtrl = new wxRadioBox(panel,
        tksIDC_SETUPOPTIONSRADIOBOXCTRL,
        "(Debug) Setup Options",
        wxDefaultPosition,
        wxDefaultSize,
        mRadioBoxSetupOptions,
        0,
        wxRA_SPECIFY_ROWS);
    sizer->Add(pSetupOptionsRadioBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    sizer->AddStretchSpacer();

    /* Confirm|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pConfirmButton = new wxButton(panel, wxID_OK, "OK");
    pConfirmButton->SetDefault();

    pCancelButton = new wxButton(panel, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pConfirmButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
}

void DebugOptionsMiniFrame::FillControls()
{
    pDebugDefaultCheckBoxCtrl->SetValue(true);
}

void DebugOptionsMiniFrame::ConfigureEventBindings()
{
    Bind(wxEVT_CLOSE_WINDOW, &DebugOptionsMiniFrame::OnClose, this);

    pDebugDefaultCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX, &DebugOptionsMiniFrame::OnDebugDefaultCheckBoxCtrl, this);

    pConfirmButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { EndModal(wxID_OK); });

    pCancelButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) { EndModal(wxID_CANCEL); });

    pSetupOptionsRadioBoxCtrl->Bind(
        wxEVT_RADIOBOX, &DebugOptionsMiniFrame::OnSetupOptionsRadioBoxSelection, this);
}

void DebugOptionsMiniFrame::OnDebugDefaultCheckBoxCtrl(wxCommandEvent& event) {}

void DebugOptionsMiniFrame::OnSetupOptionsRadioBoxSelection(wxCommandEvent& event) {}

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
