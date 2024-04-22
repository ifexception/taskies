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

#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/clrpicker.h>
#include <wx/wizard.h>

#include <spdlog/logger.h>

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
} // namespace Core
namespace UI::wizard
{
class WelcomePage;
class OptionPage;
class CreateEmployerAndClientPage;
class CreateProjectAndCategoryPage;
class SetupCompletePage;
class RestoreDatabasePage;
class RestoreDatabaseResultPage;
class SkipWizardPage;

class SetupWizard final : public wxWizard
{
public:
    SetupWizard() = delete;
    SetupWizard(const SetupWizard&) = delete;
    SetupWizard(wxFrame* frame,
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        const std::string& databasePath);
    virtual ~SetupWizard() = default;

    SetupWizard& operator=(const SetupWizard&) = delete;

    wxWizardPage* GetFirstPage() const;

    const std::int64_t GetEmployerId() const;
    void SetEmployerId(const std::int64_t employerId);
    const std::int64_t GetClientId() const;
    void SetClientId(const std::int64_t clientId);

    const std::string& GetBackupDatabasePath() const;
    void SetBackupDatabasePath(const std::string& value);
    const std::string& GetRestoreDatabasePath() const;
    void SetRestoreDatabasePath(const std::string& value);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::string mDatabasePath;

    WelcomePage* pWelcomePage;
    OptionPage* pOptionPage;
    CreateEmployerAndClientPage* pCreateEmployerAndClientPage;
    CreateProjectAndCategoryPage* pCreateProjectAndCategoryPage;
    SetupCompletePage* pSetupCompletePage;
    RestoreDatabasePage* pRestoreDatabasePage;
    RestoreDatabaseResultPage* pRestoreDatabaseResultPage;
    SkipWizardPage* pSkipWizardPage;

    std::int64_t mEmployerId;
    std::int64_t mClientId;

    std::string mBackupDatabasePath;
    std::string mRestoreDatabasePath;
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
    WelcomePage(SetupWizard* parent);
    virtual ~WelcomePage() = default;

    WelcomePage& operator=(const WelcomePage&) = delete;

private:
    void CreateControls();

    SetupWizard* pParent;
};

class OptionPage final : public wxWizardPage
{
public:
    OptionPage() = delete;
    OptionPage(const OptionPage&) = delete;
    OptionPage(SetupWizard* parent,
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

    SetupWizard* pParent;
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
    CreateEmployerAndClientPage(SetupWizard* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath);
    virtual ~CreateEmployerAndClientPage() = default;

    CreateEmployerAndClientPage& operator=(const CreateEmployerAndClientPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    void CreateControls();

    SetupWizard* pParent;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabasePath;

    wxTextCtrl* pEmployerNameTextCtrl;
    wxTextCtrl* pClientNameTextCtrl;

    enum { tksIDC_EMPLOYERNAME = wxID_HIGHEST + 100, tksIDC_CLIENTNAME };
};

class CreateProjectAndCategoryPage final : public wxWizardPageSimple
{
public:
    CreateProjectAndCategoryPage() = delete;
    CreateProjectAndCategoryPage(const CreateProjectAndCategoryPage&) = delete;
    CreateProjectAndCategoryPage(SetupWizard* parent,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath);
    virtual ~CreateProjectAndCategoryPage() = default;

    CreateProjectAndCategoryPage& operator=(const CreateProjectAndCategoryPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnProjectNameChange(wxCommandEvent& event);

    SetupWizard* pParent;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabasePath;

    wxTextCtrl* pProjectNameTextCtrl;
    wxTextCtrl* pProjectDisplayNameCtrl;
    wxCheckBox* pProjectIsDefaultCtrl;
    wxTextCtrl* pCategoryNameTextCtrl;
    wxColourPickerCtrl* pColorPickerCtrl;
    wxCheckBox* pBillableCtrl;

    enum {
        tksIDC_PROJECTNAME = wxID_HIGHEST + 100,
        tksIDC_PROJECTDISPLAYNAME,
        tksIDC_PROJECTISDEFAULT,
        tksIDC_CATEGORYNAME,
        tksIDC_CATEGORYCOLORPICKER,
        tksIDC_CATEGORYBILLABLE,
    };
};

class SetupCompletePage final : public wxWizardPageSimple
{
public:
    SetupCompletePage() = delete;
    SetupCompletePage(const SetupCompletePage&) = delete;
    SetupCompletePage(SetupWizard* parent);
    virtual ~SetupCompletePage() = default;

private:
    void CreateControls();
    void DisableBackButton() const;

    SetupWizard* pParent;
};

class RestoreDatabasePage final : public wxWizardPageSimple
{
public:
    RestoreDatabasePage() = delete;
    RestoreDatabasePage(const RestoreDatabasePage&) = delete;
    RestoreDatabasePage(SetupWizard* parent,
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg);
    virtual ~RestoreDatabasePage() = default;

    RestoreDatabasePage& operator=(const RestoreDatabasePage&) = delete;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnOpenFileForBackupLocation(wxCommandEvent& event);
    void OnOpenFileForRestoreLocation(wxCommandEvent& event);

    SetupWizard* pParent;
    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;

    wxTextCtrl* pBackupPathTextCtrl;
    wxButton* pBrowseBackupPathButton;
    wxTextCtrl* pRestorePathTextCtrl;
    wxButton* pBrowseRestorePathButton;

    enum {
        tksIDC_BACKUP_PATH = wxID_HIGHEST + 100,
        tksIDC_BACKUP_PATH_BUTTON,
        tksIDC_RESTORE_PATH,
        tksIDC_RESTORE_PATH_BUTTON,
    };
};

class RestoreDatabaseResultPage final : public wxWizardPageSimple
{
public:
    RestoreDatabaseResultPage() = delete;
    RestoreDatabaseResultPage(const RestoreDatabaseResultPage&) = delete;
    RestoreDatabaseResultPage(SetupWizard* parent, std::shared_ptr<spdlog::logger> logger);
    virtual ~RestoreDatabaseResultPage() = default;

    RestoreDatabaseResultPage& operator=(const RestoreDatabaseResultPage&) = delete;

private:
    void CreateControls();
    void ConfigureEventBindings();
    void DisableBackButton() const;

    void OnWizardPageShown(wxWizardEvent& event);

    SetupWizard* pParent;
    std::shared_ptr<spdlog::logger> pLogger;

    wxGauge* pRestoreProgressGaugeCtrl;
    wxStaticText* pStatusFeedbackLabel;
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
    void CreateControls();

    wxWizard* pParent;
};
} // namespace UI::wizard
} // namespace tks
