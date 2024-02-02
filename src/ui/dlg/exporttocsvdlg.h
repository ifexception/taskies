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

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <spdlog/spdlog.h>

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core
namespace UI::dlg
{
class ExportToCsvDlg : public wxDialog
{
public:
    ExportToCsvDlg() = delete;
    ExportToCsvDlg(const ExportToCsvDlg&) = delete;
    ExportToCsvDlg(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        const wxString& name = "exporttocsvdlg");
    virtual ~ExportToCsvDlg() = default;

    const ExportToCsv& operator=(const ExportToCsvDlg&) = delete;

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    wxWindow* pParent;
    wxTextCtrl* pExportPath;

};
} // namespace UI::dlg
} // namespace tks
