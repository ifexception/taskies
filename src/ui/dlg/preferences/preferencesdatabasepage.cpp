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

#include "preferencesdatabasepage.h"

#include <wx/filedlg.h>
#include <wx/richtooltip.h>

#include "../../../core/environment.h"
#include "../../../core/configuration.h"

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
    , pBackupOnProgramCloseCheckBoxCtrl(nullptr)
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
        pCfg->BackupOnProgramClose(pBackupOnProgramCloseCheckBoxCtrl->GetValue());
        pCfg->ZipBackupFile(pZipBackupFileCheckBoxCtrl->GetValue());
    } else {
        pCfg->SetBackupPath("");
        pCfg->BackupOnProgramClose(false);
        pCfg->ZipBackupFile(false);
    }
}

void PreferencesDatabasePage::Reset()
{
    pDatabasePathTextCtrl->ChangeValue(pCfg->GetDatabasePath());
    pDatabasePathTextCtrl->SetToolTip(pCfg->GetDatabasePath());
    pBackupDatabaseCheckBoxCtrl->SetValue(pCfg->BackupDatabase());
    if (!pCfg->BackupDatabase()) {
        pBrowseBackupPathButton->Disable();
        pBackupOnProgramCloseCheckBoxCtrl->Disable();
        pZipBackupFileCheckBoxCtrl->Disable();
    }

    pBackupPathTextCtrl->ChangeValue("");
    pBackupPathTextCtrl->SetToolTip("");

    pBackupOnProgramCloseCheckBoxCtrl->SetValue(false);
    pZipBackupFileCheckBoxCtrl->SetValue(false);
}

void PreferencesDatabasePage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Database box */
    auto databaseBox = new wxStaticBox(this, wxID_ANY, "Database");
    auto databaseBoxSizer = new wxStaticBoxSizer(databaseBox, wxVERTICAL);
    sizer->Add(databaseBoxSizer, wxSizerFlags().Expand());

    /* Database file name sizer */
    auto databaseNameLabel = new wxStaticText(databaseBox, wxID_ANY, "Name");
    pDatabaseFileNameTextCtrl = new wxTextCtrl(databaseBox,
        tksIDC_DATABASEFILENAMETEXTCTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT | wxTE_READONLY);

    /* Database path sizer */
    auto databasePathLabel = new wxStaticText(databaseBox, wxID_ANY, "Path");
    pDatabasePathTextCtrl = new wxTextCtrl(databaseBox,
        tksIDC_DATABASE_PATH,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT | wxTE_READONLY);

    pBrowseDatabasePathButton = new wxButton(databaseBox, tksIDC_DATABASE_PATH_BUTTON, "Browse...");
    pBrowseDatabasePathButton->SetToolTip("Browse and select a directory to save the database");

    /* Flex Grid Sizer for database controls */
    auto flexGridDatabaseSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    flexGridDatabaseSizer->AddGrowableCol(1, 1);

    flexGridDatabaseSizer->Add(
        databaseNameLabel, wxSizerFlags().Left().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridDatabaseSizer->Add(
        pDatabaseFileNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    flexGridDatabaseSizer->Add(
        databasePathLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridDatabaseSizer->Add(
        pDatabasePathTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    flexGridDatabaseSizer->Add(0, 0);
    flexGridDatabaseSizer->Add(
        pBrowseDatabasePathButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    databaseBoxSizer->Add(flexGridDatabaseSizer, wxSizerFlags().Expand().Proportion(1));

    /* Backup box */
    auto backupBox = new wxStaticBox(this, wxID_ANY, "Backup");
    auto backupBoxSizer = new wxStaticBoxSizer(backupBox, wxVERTICAL);
    sizer->Add(backupBoxSizer, wxSizerFlags().Expand());

    /* Enable backups check */
    pBackupDatabaseCheckBoxCtrl =
        new wxCheckBox(backupBox, tksIDC_BACKUP_DATABASE_CHECK, "Enable database backups");
    pBackupDatabaseCheckBoxCtrl->SetToolTip("Toggles whether database backups occur");

    /* Backup path sizer */
    auto backupPathLabel = new wxStaticText(backupBox, wxID_ANY, "Path");
    pBackupPathTextCtrl = new wxTextCtrl(backupBox,
        tksIDC_BACKUP_PATH,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT | wxTE_READONLY);
    pBrowseBackupPathButton = new wxButton(backupBox, tksIDC_BACKUP_PATH_BUTTON, "Browse...");
    pBrowseBackupPathButton->SetToolTip("Browse and select a directory to backup the database");

    /* Backup on program close ctrl */
    pBackupOnProgramCloseCheckBoxCtrl = new wxCheckBox(
        backupBox, tksIDC_BACKUPONPROGRAMCLOSECHECKBOXCTRL, "Backup database on program close");
    pBackupOnProgramCloseCheckBoxCtrl->SetToolTip(
        "Toggles whether database backups occur when the program is closing");

    /* Zip backup file ctrl */
    pZipBackupFileCheckBoxCtrl = new wxCheckBox(
        backupBox, tksIDC_ZIPBACKUPFILECHECKBOXCTRL, "Compress database backup file (ZIP)");
    pZipBackupFileCheckBoxCtrl->SetToolTip(
        "Create a compressed (ZIP) file of the database backup file");

    /* Flex Grid Sizer for backup controls */
    auto flexGridBackupSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    flexGridBackupSizer->AddGrowableCol(1, 1);

    flexGridBackupSizer->Add(0, 0);
    flexGridBackupSizer->Add(pBackupDatabaseCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    flexGridBackupSizer->Add(
        backupPathLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    flexGridBackupSizer->Add(
        pBackupPathTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    flexGridBackupSizer->Add(0, 0);
    flexGridBackupSizer->Add(
        pBrowseBackupPathButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    flexGridBackupSizer->Add(0, 0);
    flexGridBackupSizer->Add(
        pBackupOnProgramCloseCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    flexGridBackupSizer->Add(0, 0);
    flexGridBackupSizer->Add(pZipBackupFileCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    backupBoxSizer->Add(flexGridBackupSizer, wxSizerFlags().Expand().Proportion(1));

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
    pBackupOnProgramCloseCheckBoxCtrl->Disable();
    pZipBackupFileCheckBoxCtrl->Disable();
}

void PreferencesDatabasePage::DataToControls()
{
    pDatabaseFileNameTextCtrl->ChangeValue(pCfg->GetDatabaseFileName());
    pDatabaseFileNameTextCtrl->SetToolTip(pCfg->GetDatabaseFileName());

    pDatabasePathTextCtrl->ChangeValue(pCfg->GetDatabasePath());
    pDatabasePathTextCtrl->SetToolTip(pCfg->GetDatabasePath());
    pBackupDatabaseCheckBoxCtrl->SetValue(pCfg->BackupDatabase());
    if (pCfg->BackupDatabase()) {
        pBrowseBackupPathButton->Enable();
        pBackupOnProgramCloseCheckBoxCtrl->Enable();
        pZipBackupFileCheckBoxCtrl->Enable();

        pBackupPathTextCtrl->ChangeValue(pCfg->GetBackupPath());
        pBackupPathTextCtrl->SetToolTip(pCfg->GetBackupPath());

        pBackupOnProgramCloseCheckBoxCtrl->SetValue(pCfg->BackupOnProgramClose());
        pZipBackupFileCheckBoxCtrl->SetValue(pCfg->ZipBackupFile());
    }
}

void PreferencesDatabasePage::OnBackupDatabaseCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pBrowseBackupPathButton->Enable();
        pBackupOnProgramCloseCheckBoxCtrl->Enable();
        pZipBackupFileCheckBoxCtrl->Enable();
    } else {
        pBrowseBackupPathButton->Disable();
        pBackupPathTextCtrl->ChangeValue(wxEmptyString);
        pBackupOnProgramCloseCheckBoxCtrl->Disable();
        pBackupOnProgramCloseCheckBoxCtrl->SetValue(false);
        pZipBackupFileCheckBoxCtrl->SetValue(false);
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

    auto openDirDialog = new wxDirDialog(this,
        "Select a directory for the database",
        pathDirectoryToOpenOn,
        wxDD_DEFAULT_STYLE,
        wxDefaultPosition);
    int ret = openDirDialog->ShowModal();

    if (ret == wxID_OK) {
        auto selectedPath = openDirDialog->GetPath().ToStdString();
        pDatabasePathTextCtrl->ChangeValue(selectedPath);
        pDatabasePathTextCtrl->SetToolTip(selectedPath);
    }

    openDirDialog->Destroy();
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
