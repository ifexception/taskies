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

#include "taskbaricon.h"

#include <wx/taskbarbutton.h>
#include <sqlite3.h>

#include "../common/common.h"
#include "../common/constants.h"
#include "../core/environment.h"
#include "../core/configuration.h"
#include "../utils/utils.h"
#include "dlg/preferencesdlg.h"

namespace tks::UI
{
TaskBarIcon::TaskBarIcon(wxFrame* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pParent(parent)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
{
    ConfigureEventBindings();
}

void TaskBarIcon::SetTaskBarIcon()
{
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcon(iconBundle.GetIcon(wxDefaultSize), "Taskies is running from the system tray");
}

// clang-format off
void TaskBarIcon::ConfigureEventBindings()
{
    Bind(
        wxEVT_MENU,
        &TaskBarIcon::OnPreferences,
        this,
        tksIDC_PREFERENCES
    );

    Bind(
        wxEVT_MENU,
        &TaskBarIcon::OnExit,
        this,
        wxID_EXIT
    );

    Bind(
        wxEVT_TASKBAR_LEFT_DOWN,
        &TaskBarIcon::OnLeftButtonDown,
        this
    );
}
//clang-format on

wxMenu* TaskBarIcon::CreatePopupMenu()
{
    auto menu = new wxMenu();
    auto preferencesMenuItem = menu->Append(tksIDC_PREFERENCES, wxT("Preferences"));
    // preferencesMenuItem->SetBitmap(rc::GetSettingsIcon());

    menu->AppendSeparator();
    auto exitMenuItem = menu->Append(wxID_EXIT, wxT("Exit"));
    wxIconBundle exitIconBundle(Common::GetExitIconBundleName(), 0);
    exitMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(exitIconBundle));

    return menu;
}

void TaskBarIcon::OnPreferences(wxCommandEvent& WXUNUSED(event))
{
    dlg::PreferencesDialog preferencesDlg(pParent, pEnv, pCfg, pLogger);
    preferencesDlg.ShowModal();
}

void TaskBarIcon::OnExit(wxCommandEvent& WXUNUSED(event))
{
    sqlite3* db = nullptr;

    int rc = sqlite3_open(mDatabaseFilePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "TaskBarIcon",
            pEnv->GetDatabaseName(),
            pEnv->GetDatabasePath().string(),
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(db, Utils::sqlite::pragmas::Optimize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        pLogger->error(LogMessage::ExecQueryTemplate, "TaskBarIcon", Utils::sqlite::pragmas::Optimize, rc, err);
    }

    sqlite3_close(db);

    pParent->Close();
}

void TaskBarIcon::OnLeftButtonDown(wxTaskBarIconEvent& WXUNUSED(event))
{
    pParent->MSWGetTaskBarButton()->Show();
    if (pParent->IsIconized())
    {
        pParent->Restore();
    }
    pParent->Raise();
    pParent->Show();
    pParent->SendSizeEvent();
}
} // namespace tks::UI
