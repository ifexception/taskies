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

#include "preferencesdlg.h"

#include "../../common/common.h"
#include "../../core/configuration.h"
#include "preferencesgeneralpage.h"
#include "preferencesdatabasepage.h"

namespace tks::UI::dlg
{
PreferencesDialog::PreferencesDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Preferences",
          wxDefaultPosition,
          wxDefaultSize,
          wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
          name)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , pListBox(nullptr)
    , pSimpleBook(nullptr)
    , pGeneralPage(nullptr)
    , pDatabasePage(nullptr)
    , pRestoreDefaultsButton(nullptr)
    , pOkButton(nullptr)
{
    Initialize();

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void PreferencesDialog::Initialize()
{
    CreateControls();
    ConfigureEventBindings();
}

void PreferencesDialog::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Main Sizer */
    auto mainSizer = new wxBoxSizer(wxHORIZONTAL);

    /* List box */
    pListBox = new wxListBox(this, wxID_ANY);
    // move to fill controls method
    pListBox->Append("General");
    pListBox->Append("Database");
    pListBox->SetSelection(0);

    /* Simple Book*/
    pSimpleBook = new wxSimplebook(this, wxID_ANY);
    pGeneralPage = new PreferencesGeneralPage(pSimpleBook, pCfg, pLogger);
    pDatabasePage = new PreferencesDatabasePage(pSimpleBook, pEnv, pCfg);

    pSimpleBook->AddPage(pGeneralPage, wxEmptyString, true);
    pSimpleBook->AddPage(pDatabasePage, wxEmptyString, false);

    mainSizer->Add(pListBox, wxSizerFlags().Border(wxRIGHT, FromDIP(5)).Expand());
    mainSizer->Add(pSimpleBook, wxSizerFlags().Expand().Proportion(1));
    sizer->Add(mainSizer, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(10)).Expand().Proportion(1));

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pRestoreDefaultsButton = new wxButton(this, IDC_RESTOREDEFAULTBUTTON, "Restore Defaults");

    pOkButton = new wxButton(this, wxID_OK, "OK");
    pOkButton->SetDefault();

    buttonsSizer->Add(pRestoreDefaultsButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pOkButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesDialog::ConfigureEventBindings()
{
    Bind(
        wxEVT_LISTBOX,
        &PreferencesDialog::OnListBoxSelection,
        this
    );

    pRestoreDefaultsButton->Bind(
        wxEVT_BUTTON,
        &PreferencesDialog::OnRestoreDefaults,
        this,
        IDC_RESTOREDEFAULTBUTTON
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &PreferencesDialog::OnOK,
        this,
        wxID_OK
    );
}
// clang-format on

void PreferencesDialog::OnListBoxSelection(wxCommandEvent& event)
{
    pSimpleBook->ChangeSelection(pListBox->GetSelection());
}

void PreferencesDialog::OnRestoreDefaults(wxCommandEvent& event)
{
    bool success = pCfg->RestoreDefaults();
    if (!success) {
        wxMessageBox("Failed to restore default configuration", Common::GetProgramName(), wxICON_ERROR | wxOK_DEFAULT);
        return;
    }

    pGeneralPage->Reset();
    pDatabasePage->Reset();
}

void PreferencesDialog::OnOK(wxCommandEvent& event)
{
    if (!pGeneralPage->IsValid()) {
        pListBox->SetSelection(0);
        pSimpleBook->ChangeSelection(pListBox->GetSelection());
        return;
    }

    if (!pDatabasePage->IsValid()) {
        pListBox->SetSelection(1);
        pSimpleBook->ChangeSelection(pListBox->GetSelection());
    }

    // Save changes to cfg pointer in memory
    pGeneralPage->Save();
    pDatabasePage->Save();

    // Save changes to disk
    pCfg->Save();

    event.Skip();
}
} // namespace tks::UI::dlg
