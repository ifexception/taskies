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

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/taskbar.h>

#include <spdlog/spdlog.h>

namespace tks::UI
{
class TaskBarIcon : public wxTaskBarIcon
{
public:
    TaskBarIcon() = delete;
    TaskBarIcon(const TaskBarIcon&) = delete;
    TaskBarIcon(wxFrame* parent, std::shared_ptr<spdlog::logger> logger);
    virtual ~TaskBarIcon() = default;

    TaskBarIcon& operator=(const TaskBarIcon&) = delete;

private:
    void ConfigureEventBindings();

    wxMenu* CreatePopupMenu() wxOVERRIDE;

    void OnPreferences(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnLeftButtonDown(wxCommandEvent& event);

    wxFrame* pParent;

    std::shared_ptr<spdlog::logger> pLogger;

    enum { IDC_PREFERENCES = wxID_HIGHEST + 100 };
};
} // namespace tks::UI
