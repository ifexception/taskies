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

#include "taskbaricon.h"

#include <sqlite3.h>

#include <wx/taskbarbutton.h>

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../core/environment.h"
#include "../core/configuration.h"

#include "../utils/utils.h"

#include "dlg/preferences/preferencesdlg.h"
#include "dlg/exports/quickexporttocsvdlg.h"
#include "dlg/taskdlglegacy.h"

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
    SetIcon(iconBundle.GetIcon(wxDefaultSize), Common::GetProgramName());
}

// clang-format off
void TaskBarIcon::ConfigureEventBindings()
{
    Bind(
        wxEVT_MENU,
        &TaskBarIcon::OnNewTask,
        this,
        tksIDC_MENU_NEWTASK
    );

    Bind(
        wxEVT_MENU,
        &TaskBarIcon::OnQuickExportToCsv,
        this,
        tksIDC_MENU_QUICKEXPORTTOCSV
    );

    Bind(
        wxEVT_MENU,
        &TaskBarIcon::OnPreferences,
        this,
        tksIDC_MENU_PREFERENCES
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
// clang-format on

wxMenu* TaskBarIcon::CreatePopupMenu()
{
    auto menu = new wxMenu();

    auto newTaskMenuBarTitle =
        pCfg->UseLegacyTaskDialog() ? "&New Task (legacy)\tCtrl-N" : "&New Task\tCtrl-N";
    auto newTaskMenuBarDescription =
        pCfg->UseLegacyTaskDialog() ? "Create new task (legacy)" : "Create new task";

    auto newTaskMenuItem =
        menu->Append(tksIDC_MENU_NEWTASK, newTaskMenuBarTitle, newTaskMenuBarDescription);

    wxIconBundle addTaskIconBundle(Common::GetAddTaskIconBundleName(), 0);
    newTaskMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(addTaskIconBundle));

    menu->AppendSeparator();

    menu->Append(tksIDC_MENU_QUICKEXPORTTOCSV,
        "Quick Export to CSV",
        "Export selected data to CSV format using existing presets");

    menu->AppendSeparator();

    auto preferencesMenuItem =
        menu->Append(tksIDC_MENU_PREFERENCES, "Preferences", "View and adjust program options");

    wxIconBundle preferencesIconBundle(Common::GetPreferencesIconBundleName(), 0);
    preferencesMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(preferencesIconBundle));

    menu->AppendSeparator();
    auto exitMenuItem = menu->Append(wxID_EXIT, "Exit", "Exit the program");

    wxIconBundle exitIconBundle(Common::GetExitIconBundleName(), 0);
    exitMenuItem->SetBitmap(wxBitmapBundle::FromIconBundle(exitIconBundle));

    return menu;
}

void TaskBarIcon::OnNewTask(wxCommandEvent& event)
{
    UI::dlg::TaskDialogLegacy newTaskDialog(pParent, pEnv, pCfg, pLogger, mDatabaseFilePath);
    newTaskDialog.ShowModal();
}

void TaskBarIcon::OnQuickExportToCsv(wxCommandEvent& WXUNUSED(event))
{
    UI::dlg::QuickExportToCsvDialog quickExportToCsvDialog(
        pParent, pCfg, pLogger, mDatabaseFilePath);
    quickExportToCsvDialog.ShowModal();
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
        const char* error = sqlite3_errmsg(db);
        pLogger->error(LogMessages::OpenDatabaseTemplate,
            pEnv->GetDatabasePath().string(),
            rc,
            error);

        goto close;
    }

    rc = sqlite3_exec(db, QueryHelper::Optimize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(db);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        goto close;
    }

close:
    sqlite3_close(db);

    pParent->Close(true);
}

void TaskBarIcon::OnLeftButtonDown(wxTaskBarIconEvent& WXUNUSED(event))
{
    pParent->MSWGetTaskBarButton()->Show();
    if (pParent->IsIconized()) {
        pParent->Restore();
    }

    pParent->Raise();
    pParent->Show();
    pParent->SendSizeEvent();
}
} // namespace tks::UI
