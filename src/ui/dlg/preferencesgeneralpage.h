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
} // namespace Core
namespace UI
{
namespace dlg
{
class PreferencesGeneralPage : public wxPanel
{
public:
    PreferencesGeneralPage() = delete;
    PreferencesGeneralPage(const PreferencesGeneralPage&) = delete;
    PreferencesGeneralPage(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger);
    virtual ~PreferencesGeneralPage() = default;

    PreferencesGeneralPage& operator=(PreferencesGeneralPage&) = delete;

    bool IsValid();
    void Save();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnShowInTrayCheck(wxCommandEvent& event);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxChoice* pUserInterfaceLanguageCtrl;
    wxCheckBox* pStartWithWindowsCtrl;
    wxChoice* pWindowStartPositionCtrl;
    wxCheckBox* pShowInTrayCtrl;
    wxCheckBox* pMinimizeToTrayCtrl;
    wxCheckBox* pCloseToTrayCtrl;

    enum {
        IDC_LANG = wxID_HIGHEST + 1,
        IDC_START_WITH_WINDOWS,
        IDC_START_POSITION,
        IDC_SHOW_IN_TRAY,
        IDC_MINIMIZE_TO_TRAY,
        IDC_CLOSE_TO_TRAY,
    };
};
} // namespace dlg
} // namespace UI
} // namespace tks
