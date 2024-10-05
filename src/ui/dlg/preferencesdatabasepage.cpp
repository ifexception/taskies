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

#include "preferencesdatabasepage.h"

#include <wx/filedlg.h>
#include <wx/richtooltip.h>

#include "../../core/environment.h"
#include "../../core/configuration.h"

namespace tks::UI::dlg
{
PreferencesDatabasePage::PreferencesDatabasePage(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg)
    : wxPanel(parent, wxID_ANY)
    , pEnv(env)
    , pCfg(cfg)
    , pDatabasePathTextCtrl(nullptr)
    , pBrowseDatabasePathButton(nullptr)
    , pBackupDatabaseCheckBoxCtrl(nullptr)
    , pBackupPathTextCtrl(nullptr)
    , pBrowseBackupPathButton(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesDatabasePage::IsValid()
{
    auto databasePathSelected = pDatabasePathTextCtrl->GetValue().ToStdString();
    if (databasePathSelected.empty()) {
        auto valMsg = "A database directory is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pDatabasePathTextCtrl);
        return false;
    }

    if (pBackupDatabaseCheckBoxCtrl->IsChecked()) {
        auto exportPathSelected = pBackupPathTextCtrl->GetValue().ToStdString();
        if (exportPathSelected.empty()) {
            auto valMsg = "A backup database directory is required";
            wxRichToolTip tooltip("Validation", valMsg);
            tooltip.SetIcon(wxICON_WARNING);
            tooltip.ShowFor(pBackupPathTextCtrl);
            return false;
        }
    }

    return true;
}

void PreferencesDatabasePage::Save()
{
    pCfg->SetDatabasePath(pDatabasePathTextCtrl->GetValue().ToStdString());
    pCfg->BackupDatabase(pBackupDatabaseCheckBoxCtrl->IsChecked());
    if (pCfg->BackupDatabase()) {
        pCfg->SetBackupPath(pBackupPathTextCtrl->GetValue().ToStdString());
    } else {
        pCfg->SetBackupPath("");
    }
}

void PreferencesDatabasePage::Reset()
{
    pDatabasePathTextCtrl->ChangeValue(pCfg->GetDatabasePath());
    pDatabasePathTextCtrl->SetToolTip(pCfg->GetDatabasePath());
    pBackupDatabaseCheckBoxCtrl->SetValue(pCfg->BackupDatabase());
    if (!pCfg->BackupDatabase()) {
        pBrowseBackupPathButton->Disable();
    }

    pBackupPathTextCtrl->ChangeValue(pCfg->GetBackupPath());
    pBackupPathTextCtrl->SetToolTip(pCfg->GetBackupPath());
}

void PreferencesDatabasePage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Database box */
    auto databaseBox = new wxStaticBox(this, wxID_ANY, "Database");
    auto databaseBoxSizer = new wxStaticBoxSizer(databaseBox, wxVERTICAL);
    sizer->Add(databaseBoxSizer, wxSizerFlags().Expand());

    /* Database path sizer */
    auto dbPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto databasePathLabel = new wxStaticText(databaseBox, wxID_ANY, "Path");
    pDatabasePathTextCtrl = new wxTextCtrl(
        databaseBox, tksIDC_DATABASE_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseDatabasePathButton = new wxButton(databaseBox, tksIDC_DATABASE_PATH_BUTTON, "Browse...");
    pBrowseDatabasePathButton->SetToolTip("Browse and select a directory to store the database");
    dbPathSizer->Add(databasePathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    dbPathSizer->Add(pDatabasePathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    dbPathSizer->Add(pBrowseDatabasePathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    databaseBoxSizer->Add(dbPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Backup box */
    auto backupBox = new wxStaticBox(this, wxID_ANY, "Backup");
    auto backupBoxSizer = new wxStaticBoxSizer(backupBox, wxVERTICAL);
    sizer->Add(backupBoxSizer, wxSizerFlags().Expand());

    /* Enable backups check */
    pBackupDatabaseCheckBoxCtrl = new wxCheckBox(backupBox, tksIDC_BACKUP_DATABASE_CHECK, "Enable database backups");
    pBackupDatabaseCheckBoxCtrl->SetToolTip("Toggles whether database backups occur");
    backupBoxSizer->Add(pBackupDatabaseCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Backup path sizer*/
    auto backupPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto backupPathLabel = new wxStaticText(backupBox, wxID_ANY, "Path");
    pBackupPathTextCtrl = new wxTextCtrl(
        backupBox, tksIDC_BACKUP_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseBackupPathButton = new wxButton(backupBox, tksIDC_BACKUP_PATH_BUTTON, "Browse...");
    pBrowseBackupPathButton->SetToolTip("Browse and select a directory to store the database backups");
    backupPathSizer->Add(backupPathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    backupPathSizer->Add(
        pBackupPathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    backupPathSizer->Add(pBrowseBackupPathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    backupBoxSizer->Add(backupPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesDatabasePage::ConfigureEventBindings()
{
    pBackupDatabaseCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &PreferencesDatabasePage::OnBackupDatabaseCheck,
        this
    );

    pBrowseDatabasePathButton->Bind(
        wxEVT_BUTTON,
        &PreferencesDatabasePage::OnOpenDirectoryForDatabaseLocation,
        this,
        tksIDC_DATABASE_PATH_BUTTON
    );

    pBrowseBackupPathButton->Bind(
        wxEVT_BUTTON,
        &PreferencesDatabasePage::OnOpenDirectoryForBackupLocation,
        this,
        tksIDC_BACKUP_PATH_BUTTON
    );
}
// clang-format on

void PreferencesDatabasePage::FillControls()
{
    pBrowseBackupPathButton->Disable();
}

void PreferencesDatabasePage::DataToControls()
{
    pDatabasePathTextCtrl->ChangeValue(pCfg->GetDatabasePath());
    pDatabasePathTextCtrl->SetToolTip(pCfg->GetDatabasePath());
    pBackupDatabaseCheckBoxCtrl->SetValue(pCfg->BackupDatabase());
    if (pCfg->BackupDatabase()) {
        pBrowseBackupPathButton->Enable();
    }

    pBackupPathTextCtrl->ChangeValue(pCfg->GetBackupPath());
    pBackupPathTextCtrl->SetToolTip(pCfg->GetBackupPath());
}

void PreferencesDatabasePage::OnBackupDatabaseCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pBrowseBackupPathButton->Enable();
    } else {
        pBrowseBackupPathButton->Disable();
        pBackupPathTextCtrl->ChangeValue(wxEmptyString);
    }
}

void PreferencesDatabasePage::OnOpenDirectoryForDatabaseLocation(wxCommandEvent& event)
{
    std::string pathDirectoryToOpenOn;
    if (pCfg->GetDatabasePath().empty()) {
        pathDirectoryToOpenOn = pEnv->GetDatabasePath().string();
    } else {
        pathDirectoryToOpenOn = pCfg->GetDatabasePath();
    }

    auto fullPath = std::filesystem::path(pathDirectoryToOpenOn);
    auto& pathWithoutFileName = fullPath.remove_filename();
    pathDirectoryToOpenOn = pathWithoutFileName.string();

    auto openFileDialog = new wxFileDialog(this,
        "Select a default database location",
        pathDirectoryToOpenOn,
        wxEmptyString,
        "DB files (*.db)|*.db",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = openFileDialog->ShowModal();

    if (ret == wxID_OK) {
        auto selectedPath = openFileDialog->GetPath().ToStdString();
        pDatabasePathTextCtrl->ChangeValue(selectedPath);
        pDatabasePathTextCtrl->SetToolTip(selectedPath);
    }

    openFileDialog->Destroy();
}

void PreferencesDatabasePage::OnOpenDirectoryForBackupLocation(wxCommandEvent& event)
{
    std::string pathDirectoryToOpenOn;
    if (pCfg->GetDatabasePath().empty()) {
        pathDirectoryToOpenOn = pEnv->GetDatabasePath().string();
    } else {
        pathDirectoryToOpenOn = pCfg->GetBackupPath();
    }

    auto openDirDialog = new wxDirDialog(this,
        "Select a backup directory for the database",
        pathDirectoryToOpenOn,
        wxDD_DEFAULT_STYLE,
        wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        auto selectedBackupPath = openDirDialog->GetPath().ToStdString();
        pBackupPathTextCtrl->SetValue(selectedBackupPath);
        pBackupPathTextCtrl->SetToolTip(selectedBackupPath);
    }

    openDirDialog->Destroy();
}
} // namespace tks::UI::dlg
