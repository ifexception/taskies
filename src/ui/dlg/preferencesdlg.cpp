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

#include "preferencesdlg.h"

#include <wx/persist/toplevel.h>

#include "../../common/common.h"
#include "../../core/configuration.h"
#include "../events.h"
#include "../notificationclientdata.h"

#include "preferencesgeneralpage.h"
#include "preferencesdatabasepage.h"
#include "preferencestaskspage.h"
#include "preferencestasksviewpage.h"
#include "preferencesexportpage.h"

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
    , pParent(parent)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , pListBox(nullptr)
    , pSimpleBook(nullptr)
    , pGeneralPage(nullptr)
    , pDatabasePage(nullptr)
    , pTasksPage(nullptr)
    , pTasksViewPage(nullptr)
    , pExportPage(nullptr)
    , pRestoreDefaultsButton(nullptr)
    , pOkButton(nullptr)
{
    Initialize();

    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(480, 300)));
    }

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
    pListBox->Append("General");
    pListBox->Append("Database");
    pListBox->Append("Tasks");
    pListBox->Append("Tasks View");
    pListBox->Append("Export");
    pListBox->SetSelection(0);

    /* Simple Book*/
    pSimpleBook = new wxSimplebook(this, wxID_ANY);
    pGeneralPage = new PreferencesGeneralPage(pSimpleBook, pCfg, pLogger);
    pDatabasePage = new PreferencesDatabasePage(pSimpleBook, pEnv, pCfg);
    pTasksPage = new PreferencesTasksPage(pSimpleBook, pCfg, pLogger);
    pTasksViewPage = new PreferencesTasksViewPage(pSimpleBook, pCfg, pLogger);
    pExportPage = new PreferencesExportPage(pSimpleBook, pEnv, pCfg, pLogger);

    pSimpleBook->AddPage(pGeneralPage, wxEmptyString, /*bSelect=*/true);
    pSimpleBook->AddPage(pDatabasePage, wxEmptyString, false);
    pSimpleBook->AddPage(pTasksPage, wxEmptyString, false);
    pSimpleBook->AddPage(pTasksViewPage, wxEmptyString, false);
    pSimpleBook->AddPage(pExportPage, wxEmptyString, false);

    mainSizer->Add(pListBox, wxSizerFlags().Border(wxRIGHT, FromDIP(5)).Expand());
    mainSizer->Add(pSimpleBook, wxSizerFlags().Expand().Proportion(1));
    sizer->Add(mainSizer, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(10)).Expand().Proportion(1));

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pRestoreDefaultsButton = new wxButton(this, tksIDC_RESTOREDEFAULTBUTTON, "Restore Defaults");

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
        tksIDC_RESTOREDEFAULTBUTTON
    );

    pOkButton->Bind(
        wxEVT_BUTTON,
        &PreferencesDialog::OnOK,
        this,
        wxID_OK
    );

    Bind(
        wxEVT_CLOSE_WINDOW,
        &PreferencesDialog::OnClose,
        this
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
    pTasksPage->Reset();
    pTasksViewPage->Reset();
    pExportPage->Reset();

    std::string message = "Preferences restored to defaults";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);
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
        return;
    }

    if (!pTasksPage->IsValid()) {
        pListBox->SetSelection(2);
        pSimpleBook->ChangeSelection(pListBox->GetSelection());
        return;
    }

    if (!pTasksViewPage->IsValid()) {
        pListBox->SetSelection(3);
        pSimpleBook->ChangeSelection(pListBox->GetSelection());
        return;
    }

    if (!pExportPage->IsValid()) {
        pListBox->SetSelection(4);
        pSimpleBook->ChangeSelection(pListBox->GetSelection());
        return;
    }

    // Save changes to cfg pointer in memory
    pGeneralPage->Save();
    pDatabasePage->Save();
    pTasksPage->Save();
    pTasksViewPage->Save();
    pExportPage->Save();

    // Save changes to disk
    pCfg->Save();

    // Post success notification event
    std::string message = "Preferences updated";
    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData = new NotificationClientData(NotificationType::Information, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);

    event.Skip();
}

void PreferencesDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(wxID_CANCEL);
}
} // namespace tks::UI::dlg
