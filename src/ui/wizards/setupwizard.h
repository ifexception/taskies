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
namespace Core
{
class Environment;
}
namespace UI::wizard
{
class WelcomePage;
class OptionPage;
class CreateEmployerAndClientPage;
class CreateProjectAndCategoryPage;
class SkipWizardPage;

class SetupWizard final : public wxWizard
{
public:
    SetupWizard() = delete;
    SetupWizard(const SetupWizard&) = delete;
    SetupWizard(wxFrame* frame, std::shared_ptr<spdlog::logger> logger, std::shared_ptr<Core::Environment> env);
    virtual ~SetupWizard() = default;

    SetupWizard& operator=(const SetupWizard&) = delete;

    bool Run();

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;

    WelcomePage* pWelcomePage;
    OptionPage* pOptionPage;
    CreateEmployerAndClientPage* pCreateEmployerAndClientPage;
    CreateProjectAndCategoryPage* pCreateProjectAndCategoryPage;
    RestoreDatabasePage* pRestoreDatabasePage;
    SkipWizardPage* pSkipWizardPage;
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

class WelcomePage final : public wxWizardPageSimple
{
public:
    WelcomePage() = delete;
    WelcomePage(const WelcomePage&) = delete;
    WelcomePage(wxWizard* parent);
    virtual ~WelcomePage() = default;

    WelcomePage& operator=(const WelcomePage&) = delete;

private:
    void CreateControls();

    wxWizard* pParent;
};

class OptionPage final : public wxWizardPage
{
public:
    OptionPage() = delete;
    OptionPage(const OptionPage&) = delete;
    OptionPage(wxWizard* parent,
        wxWizardPage* prev,
        wxWizardPage* nextOption1,
        wxWizardPage* nextOption2,
        wxWizardPage* nextOption3);
    virtual ~OptionPage() = default;

    OptionPage& operator=(const OptionPage&) = delete;

    virtual wxWizardPage* GetPrev() const override;
    virtual wxWizardPage* GetNext() const override;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnSetupWizardFlowCheck(wxCommandEvent& event);
    void OnRestoreWizardFlowCheck(wxCommandEvent& event);
    void OnSkipWizardFlowCheck(wxCommandEvent& event);

    wxWizard* pParent;
    wxWizardPage* pNextOption1;
    wxWizardPage* pNextOption2;
    wxWizardPage* pNextOption3;
    wxWizardPage* pPrev;

    wxCheckBox* pSetupWizardFlowCheckBox;
    wxCheckBox* pRestoreWizardFlowCheckBox;
    wxCheckBox* pSkipWizardFlowCheckBox;

    enum {
        tksIDC_SETUPWIZARD_CHECKBOX = wxID_HIGHEST + 100,
        tksIDC_RESTOREWIZARD_CHECKBOX,
        tksIDC_SKIPWIZARD_CHECKBOX
    };
};

class CreateEmployerAndClientPage final : public wxWizardPageSimple
{
public:
    CreateEmployerAndClientPage() = delete;
    CreateEmployerAndClientPage(const CreateEmployerAndClientPage&) = delete;
    CreateEmployerAndClientPage(wxWizard* parent);
    virtual ~CreateEmployerAndClientPage() = default;

    CreateEmployerAndClientPage& operator=(const CreateEmployerAndClientPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    wxWizard* pParent;

    wxTextCtrl* pEmployerNameTextCtrl;
    wxTextCtrl* pClientNameTextCtrl;

    void CreateControls();

    enum {
        tksIDC_EMPLOYERNAME = wxID_HIGHEST + 100,
        tksIDC_CLIENTNAME
    };
};

class CreateProjectAndCategoryPage final : public wxWizardPageSimple
{
public:
    CreateProjectAndCategoryPage() = delete;
    CreateProjectAndCategoryPage(const CreateProjectAndCategoryPage&) = delete;
    CreateProjectAndCategoryPage(wxWizard* parent);
    virtual ~CreateProjectAndCategoryPage() = default;

    CreateProjectAndCategoryPage& operator=(const CreateProjectAndCategoryPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    wxWizard* pParent;

    void CreateControls();
};

class RestoreDatabasePage final : public wxWizardPageSimple
{
public:
    RestoreDatabasePage() = delete;
    RestoreDatabasePage(const RestoreDatabasePage&) = delete;
    RestoreDatabasePage(wxWizard* parent);
    virtual ~RestoreDatabasePage() = default;

    RestoreDatabasePage& operator=(const RestoreDatabasePage&) = delete;

private:
    wxWizard* pParent;

    void CreateControls();
};

class SkipWizardPage final : public wxWizardPageSimple
{
public:
    SkipWizardPage() = delete;
    SkipWizardPage(const SkipWizardPage&) = delete;
    SkipWizardPage(wxWizard* parent);
    virtual ~SkipWizardPage() = default;

    SkipWizardPage& operator=(const SkipWizardPage&) = delete;

private:
    wxWizard* pParent;

    void CreateControls();
};
} // namespace UI::wizard
} // namespace tks
