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

#include "preferencesdatabasepage.h"

namespace tks::UI::dlg
{
PreferencesDatabasePage::PreferencesDatabasePage(wxWindow* parent, std::shared_ptr<Core::Configuration> cfg)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pDatabasePathTextCtrl(nullptr)
    , pBrowseDatabasePathButton(nullptr)
    , pBackupDatabaseCheckBoxCtrl(nullptr)
    , pBackupPathTextCtrl(nullptr)
    , pBrowseBackupPathButton(nullptr)
    , pBackupsRetentionPeriodSpinCtrl(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesDatabasePage::IsValid()
{
    return false;
}

void PreferencesDatabasePage::Save() {}

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
        databaseBox, IDC_DATABASE_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseDatabasePathButton = new wxButton(databaseBox, IDC_DATABASE_PATH_BUTTON, "Browse...");
    dbPathSizer->Add(databasePathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    dbPathSizer->Add(pDatabasePathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    dbPathSizer->Add(pBrowseDatabasePathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    databaseBoxSizer->Add(dbPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Backup box */
    auto backupBox = new wxStaticBox(this, wxID_ANY, "Backup");
    auto backupBoxSizer = new wxStaticBoxSizer(backupBox, wxVERTICAL);
    sizer->Add(backupBoxSizer, wxSizerFlags().Expand());

    /* Enable backups check */
    pBackupDatabaseCheckBoxCtrl = new wxCheckBox(backupBox, IDC_BACKUP_DATABASE_CHECK, "Enable database backups");
    backupBoxSizer->Add(pBackupDatabaseCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Backup path sizer*/
    auto backupPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto backupPathLabel = new wxStaticText(backupBox, wxID_ANY, "Path");
    pBackupPathTextCtrl = new wxTextCtrl(
        backupBox, IDC_BACKUP_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseBackupPathButton = new wxButton(backupBox, IDC_BACKUP_PATH_BUTTON, "Browse...");
    backupPathSizer->Add(backupPathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    backupPathSizer->Add(
        pBackupPathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    backupPathSizer->Add(pBrowseBackupPathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    backupBoxSizer->Add(backupPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Backup retention input */
    auto retentionPeriodSizer = new wxBoxSizer(wxHORIZONTAL);
    auto retentionPeriodLabel = new wxStaticText(backupBox, wxID_ANY, "Retention Period");
    pBackupsRetentionPeriodSpinCtrl = new wxSpinCtrl(backupBox,
        IDC_BACKUPS_RETENTION_PERIOD,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxSP_ARROW_KEYS | wxSP_WRAP | wxALIGN_CENTRE_HORIZONTAL,
        0,
        14);
    pBackupsRetentionPeriodSpinCtrl->SetToolTip("Select for how many days to retain the backups for");
    retentionPeriodSizer->Add(retentionPeriodLabel, wxSizerFlags().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    retentionPeriodSizer->AddStretchSpacer();
    retentionPeriodSizer->Add(pBackupsRetentionPeriodSpinCtrl, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    backupBoxSizer->Add(retentionPeriodSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

void PreferencesDatabasePage::ConfigureEventBindings() {}

void PreferencesDatabasePage::FillControls() {}

void PreferencesDatabasePage::DataToControls() {}

void PreferencesDatabasePage::OnOpenDirectoryForDatabaseLocation(wxCommandEvent& event) {}

void PreferencesDatabasePage::OnBackupDatabaseCheck(wxCommandEvent& event) {}

void PreferencesDatabasePage::OnOpenDirectoryForBackupLocation(wxCommandEvent& event) {}
} // namespace tks::UI::dlg
