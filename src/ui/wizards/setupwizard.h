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

#pragma once

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/wizard.h>

#include <spdlog/logger.h>

namespace tks
{
namespace UI::wizard
{
class WelcomePage;

class SetupWizard final : public wxWizard
{
public:
    SetupWizard() = delete;
    SetupWizard(const SetupWizard&) = delete;
    SetupWizard(wxFrame* frame, std::shared_ptr<spdlog::logger> logger);
    virtual ~SetupWizard() = default;

    SetupWizard& operator=(const SetupWizard&) = delete;

    bool Run();

private:
    std::shared_ptr<spdlog::logger> pLogger;

    WelcomePage* pPage1;
};

/*
 * Wizard flow:
 * Welcome page
 * |
 * Restore database backup
 * ---|
 *    SelectAndRestoreDatabaseBackup
 *    Complete
 * |
 * Create entities
 * ---|
 *    AddEmployerAndClient
 *    AddProject
 *    AddCategory
 *    Complete
 * |
 * Skip
 */

class WelcomePage final : public wxWizard
{
public:
    WelcomePage()=delete;
    WelcomePage(const WelcomePage&) = delete;
    WelcomePage(wxWizard* parent);
    virtual ~WelcomePage() = default;

private:
    void CreateControls();

    SetupWizard* pParent;
};
} // namespace UI::wizard
} // namespace tks
