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

#pragma once

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/snglinst.h>

#include <spdlog/spdlog.h>

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
}

namespace UI
{
class PersistenceManager;
}

class Application : public wxApp
{
public:
    Application();
    virtual ~Application() = default;

    bool OnInit() override;
    int OnExit() override;

private:
    void InitializeLogger();
    bool InitializeConfiguration();
    bool RunMigrations();
    bool InitializeTranslations();

    bool FirstStartupProcedure(wxFrame* frame);

    void ActivateOtherInstance();

    std::unique_ptr<wxSingleInstanceChecker> pInstanceChecker;

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<UI::PersistenceManager> pPersistenceManager;
};
} // namespace tks
