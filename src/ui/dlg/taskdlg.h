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

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/spinctrl.h>

#include <spdlog/logger.h>

namespace tks
{
namespace Core
{
class Configuration;
} // namespace Core
namespace UI::dlg
{
class TaskDialog final : public wxDialog
{
public:
    TaskDialog() = delete;
    TaskDialog(const TaskDialog&) = delete;
    TaskDialog(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isEdit = false,
        std::int64_t taskId = -1,
        const std::string& selectedDate = "",
        const wxString& name = "taskdlg");
    virtual ~TaskDialog() = default;

    TaskDialog& operator=(const TaskDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();
    void DataToControls();

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace UI::dlg
} // namespace tks
