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
#include <wx/spinctrl.h>

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
} // namespace Core
namespace UI::dlg
{
class PreferencesDatabasePage : public wxPanel
{
public:
    PreferencesDatabasePage() = delete;
    PreferencesDatabasePage(const PreferencesDatabasePage&) = delete;
    PreferencesDatabasePage(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg);
    virtual ~PreferencesDatabasePage() = default;

    PreferencesDatabasePage& operator=(const PreferencesDatabasePage&) = delete;

    bool IsValid();
    void Save();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void FillControls();
    void DataToControls();

    void OnOpenDirectoryForDatabaseLocation(wxCommandEvent& event);
    void OnBackupDatabaseCheck(wxCommandEvent& event);
    void OnOpenDirectoryForBackupLocation(wxCommandEvent& event);

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;

    wxTextCtrl* pDatabasePathTextCtrl;
    wxButton* pBrowseDatabasePathButton;
    wxCheckBox* pBackupDatabaseCheckBoxCtrl;
    wxTextCtrl* pBackupPathTextCtrl;
    wxButton* pBrowseBackupPathButton;
    wxSpinCtrl* pBackupsRetentionPeriodSpinCtrl;

    enum {
        IDC_DATABASE_PATH = wxID_HIGHEST + 1,
        IDC_DATABASE_PATH_BUTTON,
        IDC_BACKUP_DATABASE_CHECK,
        IDC_BACKUP_DATABASE,
        IDC_BACKUP_PATH,
        IDC_BACKUP_PATH_BUTTON,
        IDC_BACKUPS_RETENTION_PERIOD
    };
};
} // namespace UI::dlg
} // namespace tks
