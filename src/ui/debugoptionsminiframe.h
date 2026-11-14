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

#pragma once

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/evtloop.h>
#include <wx/minifram.h>

#include <spdlog/spdlog.h>

namespace tks::UI
{
class DebugOptionsMiniFrame : public wxMiniFrame
{
public:
    DebugOptionsMiniFrame() = delete;
    DebugOptionsMiniFrame(wxWindow* parent, std::shared_ptr<spdlog::logger> logger);
    DebugOptionsMiniFrame(const DebugOptionsMiniFrame&) = delete;
    DebugOptionsMiniFrame& operator=(const DebugOptionsMiniFrame&) = delete;
    DebugOptionsMiniFrame(DebugOptionsMiniFrame&&) noexcept = default;
    DebugOptionsMiniFrame& operator=(DebugOptionsMiniFrame&&) noexcept = default;
    ~DebugOptionsMiniFrame() = default;

    int ShowModal();
    void EndModal(int returnCode);

private:
    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();

    void OnClose(wxCloseEvent& event);
    void OnDebugDefaultCheckBoxCtrl(wxCommandEvent& event);

    wxWindow* pParent;
    std::shared_ptr<spdlog::logger> pLogger;

    wxCheckBox* pDebugDefaultCheckBoxCtrl;

    wxButton* pConfirmButton;
    wxButton* pCancelButton;

    wxEventLoop* pEventLoop;

    int mReturnCode;

    enum {
        tksIDC_CONTROLBASEID = wxID_HIGHEST + 100,
        tksIDC_DEBUGDEFAULTCHECKBOXCTRL,
    };
};
} // namespace tks::UI
