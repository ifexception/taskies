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

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/wizard.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

namespace tks
{
namespace Core
{
class Configuration;
class Environment;
} // namespace Core
namespace UI::wizard
{
class PortableSetupWizard final : public wxWizard
{
public:
    PortableSetupWizard() = delete;
    PortableSetupWizard(const PortableSetupWizard&) = delete;
    PortableSetupWizard(wxFrame* frame,
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg);
    virtual ~PortableSetupWizard();

    PortableSetupWizard& operator=(const PortableSetupWizard&) = delete;

private:
    void ConfigureEventBindings();

    void OnWizardFinished(wxWizardEvent& event);

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
};
} // namespace UI::wizard
} // namespace tks
