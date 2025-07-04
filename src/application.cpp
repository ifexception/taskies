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

#include "application.h"

#include <wx/ipc.h>
#include <wx/taskbarbutton.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>

#include "common/common.h"
#include "common/enums.h"

#include "core/environment.h"
#include "core/configuration.h"
#include "core/database_migration.h"

#include "ui/wizards/setupwizard.h"

#include "ui/persistencemanager.h"
#include "ui/translator.h"
#include "ui/mainframe.h"

#define LOGGER_NAME "TaskiesLogger"

namespace tks
{
Application::Application()
    : pInstanceChecker(std::make_unique<wxSingleInstanceChecker>())
    , pLogger(nullptr)
    , pEnv(nullptr)
    , pCfg(nullptr)
    , pPersistenceManager(nullptr)
{
}

bool Application::OnInit()
{
    if (!wxApp::OnInit()) {
        return false;
    }

    if (pInstanceChecker->IsAnotherRunning()) {
        ActivateOtherInstance();
        return false;
    }

    pEnv = std::make_shared<Core::Environment>();

    InitializeLogger();

    auto env = pEnv->GetBuildConfiguration();
    pLogger->info("Application - Running in \"{0}\" environment", BuildConfigurationToString(env));

    pCfg = std::make_shared<Core::Configuration>(pEnv, pLogger);
    if (!InitializeConfiguration()) {
        wxMessageBox("An error occured when initializing configuration",
            Common::GetProgramName(),
            wxICON_ERROR | wxOK_DEFAULT);
        return false;
    }

    if (pCfg->GetDatabasePath().empty()) {
        pCfg->SetDatabasePath(pEnv->GetDatabasePath().string());
        pCfg->Save();
    }

    if (pCfg->GetExportPath().empty()) {
        pCfg->SetExportPath(pEnv->GetExportPath().string());
        pCfg->Save();
    }

    pPersistenceManager =
        std::make_unique<UI::PersistenceManager>(pLogger, pCfg->GetDatabasePath());
    wxPersistenceManager::Set(*pPersistenceManager);

    if (!RunMigrations()) {
        wxMessageBox(
            "Failed to run migrations", Common::GetProgramName(), wxICON_ERROR | wxOK_DEFAULT);
        return false;
    }

    if (!InitializeTranslations()) {
        pLogger->error("Failed to initialize translations");
        wxMessageBox("Failed to initialize translations.",
            Common::GetProgramName(),
            wxICON_ERROR | wxOK_DEFAULT);
        return false;
    }

    pLogger->info("Application - Initialize MainFrame with WindowState \"{0}\"",
        WindowStateToString(pCfg->GetWindowState()));
    auto frame = new UI::MainFrame(pEnv, pCfg, pLogger);
    SetTopWindow(frame);

    auto windowState = pCfg->GetWindowState();
    switch (windowState) {
    case WindowState::Normal:
        frame->Show(true);
        break;
    case WindowState::Minimized:
        frame->Iconize();
        frame->Show(true);
        break;
    case WindowState::Hidden: {
        if (pCfg->ShowInTray()) {
            frame->MSWGetTaskBarButton()->Hide();
        } else {
            frame->Show(true);
        }
        break;
    }
    case WindowState::Maximized:
        frame->Maximize();
        frame->Show(true);
        break;
    default:
        break;
    }

    if (!pEnv->IsSetup()) {
        pLogger->info("Application - Program not yet set up");
        if (!FirstStartupProcedure(frame)) {
            return false;
        }
    }

    return true;
}

int Application::OnExit()
{
    // Under VisualStudio, this must be called before main finishes to workaround a known VS issue
    spdlog::drop_all();
    return wxApp::OnExit();
}

void Application::InitializeLogger()
{
    auto logDirectory = pEnv->GetLogFilePath().string();

#ifdef TKS_DEBUG
    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_st>();
    msvcSink->set_level(spdlog::level::info);
    msvcSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    auto dailyFileSink =
        std::make_shared<spdlog::sinks::daily_file_sink_st>(logDirectory, 5, 0, false, 5);
    dailyFileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    dailyFileSink->set_level(spdlog::level::warn);

    std::vector<spdlog::sink_ptr> sinks{ msvcSink, dailyFileSink };

    pLogger = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
#else
    auto dailyFileSink =
        std::make_shared<spdlog::sinks::daily_file_sink_st>(logDirectory, 5, 0, false, 5);
    dailyFileSink->set_level(spdlog::level::warn);
    dailyFileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    std::vector<spdlog::sink_ptr> sinks{ dailyFileSink };

    pLogger = std::make_shared<spdlog::logger>(LOGGER_NAME, sinks.begin(), sinks.end());
    pLogger->set_level(spdlog::level::warn);
    pLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
#endif // TKS_DEBUG

    SPDLOG_LOGGER_TRACE(pLogger, "{0} has been initialized", LOGGER_NAME);
}

bool Application::InitializeConfiguration()
{
    return pCfg->Load();
}

bool Application::RunMigrations()
{
    Core::DatabaseMigration migrations(pLogger, pCfg->GetDatabasePath());

    return migrations.Migrate();
}

bool Application::InitializeTranslations()
{
    return UI::Translator::GetInstance().Load(
        pCfg->GetUserInterfaceLanguage(), pEnv->GetLanguagesPath());
}

bool Application::FirstStartupProcedure(wxFrame* frame)
{
    UI::wizard::SetupWizard* wizard = new UI::wizard::SetupWizard(frame, pLogger, pEnv, pCfg);
    wizard->CenterOnScreen();
    auto result = wizard->RunWizard(wizard->GetFirstPage());
    wizard->Destroy();

    if (result) {
        result = pEnv->SetIsSetup();
        if (result) {
            return true;
        }
        pLogger->error("Application::FirstStartupProcedure - Error occured when setting 'IsSetup' "
                       "Windows registry key.");
    }
    pLogger->error(
        "Application::FirstStartupProcedure - Wizard canceled or unexpected error occured");

    return false;
}

void Application::ActivateOtherInstance()
{
    pLogger->info("MainFrame::ActivateOtherInstance begin");

    wxClient client;
    auto connection =
        client.MakeConnection("localhost", Common::GetProgramName(), "ApplicationOptions");

    if (connection) {
        pLogger->info("MainFrame::ActivateOtherInstance connection established");
        connection->Execute("");
        connection->Disconnect();
    }
}
} // namespace tks
