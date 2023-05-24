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
#include <wx/bookctrl.h>
#include <wx/hyperlink.h>
#include <wx/listbook.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/simplebook.h>

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
class PreferencesGeneralPage;
class PreferencesDatabasePage;

class PreferencesDialog : public wxDialog
{
public:
    PreferencesDialog() = delete;
    PreferencesDialog(const PreferencesDialog&) = delete;
    PreferencesDialog(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const wxString& name = "preferencesdlg");
    virtual ~PreferencesDialog() = default;

    PreferencesDialog& operator=(const PreferencesDialog&) = delete;

private:
    void Initialize();

    void CreateControls();
    void ConfigureEventBindings();

    void OnListBoxSelection(wxCommandEvent& event);
    void OnOK(wxCommandEvent& event);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    wxListBox* pListBox;
    wxSimplebook* pSimpleBook;
    PreferencesGeneralPage* pGeneralPage;
    PreferencesDatabasePage* pDatabasePage;
    wxButton* pOkButton;
    wxButton* pCancelButton;
};
} // namespace dlg
} // namespace UI
} // namespace tks
