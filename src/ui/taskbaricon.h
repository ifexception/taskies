// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
} // namespace Core
namespace UI
{
class TaskBarIcon : public wxTaskBarIcon
{
public:
    TaskBarIcon() = delete;
    TaskBarIcon(const TaskBarIcon&) = delete;
    TaskBarIcon(wxFrame* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~TaskBarIcon() = default;

    TaskBarIcon& operator=(const TaskBarIcon&) = delete;

    void SetTaskBarIcon();

private:
    void ConfigureEventBindings();

    wxMenu* CreatePopupMenu() wxOVERRIDE;

    void OnNewTask(wxCommandEvent& event);
    void OnPreferences(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnLeftButtonDown(wxTaskBarIconEvent& event);

    wxFrame* pParent;

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    enum { tksIDC_NEWTASK = wxID_HIGHEST + 1000, tksIDC_PREFERENCES };
};
} // namespace UI
} // namespace tks
