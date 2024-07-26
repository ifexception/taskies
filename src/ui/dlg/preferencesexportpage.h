// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <spdlog/logger.h>

namespace tks
{
namespace Core
{
class Configuration;
class Environment;
} // namespace Core
namespace UI::dlg
{
class PreferencesExportPage : public wxPanel
{
public:
    PreferencesExportPage() = delete;
    PreferencesExportPage(const PreferencesExportPage&) = delete;
    explicit PreferencesExportPage(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger);
    virtual ~PreferencesExportPage() = default;

    PreferencesExportPage& operator=(const PreferencesExportPage&) = delete;

    bool IsValid();
    void Save();
    void Reset();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnOpenDirectoryForExportLocation(wxCommandEvent& event);

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;

    wxTextCtrl* pExportPathTextCtrl;
    wxButton* pBrowseExportPathButton;

    enum {
        tksIDC_EXPORT_PATH = wxID_HIGHEST + 100,
        tksIDC_EXPORT_PATH_BUTTON,
    };
};
} // namespace UI::dlg
} // namespace tks