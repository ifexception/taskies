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

#include "../../repository/setupwizardrepository.h"

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
    virtual ~SetupWizard();

    SetupWizard& operator=(const SetupWizard&) = delete;

    wxWizardPage* GetFirstPage() const;

    const std::int64_t GetEmployerId() const;
    void SetEmployerId(const std::int64_t employerId);

    const std::int64_t GetClientId() const;
    void SetClientId(const std::int64_t clientId);

    const std::int64_t GetProjectId() const;
    void SetProjectId(const std::int64_t projectId);

    const std::int64_t GetCategoryId() const;
    void SetCategoryId(const std::int64_t categoryId);

    const std::string& GetBackupDatabasePath() const;
    void SetBackupDatabasePath(const std::string& value);

    const std::string& GetRestoreDatabasePath() const;
    void SetRestoreDatabasePath(const std::string& value);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::string mDatabasePath;

    repos::SetupWizardRepository* pSetupWizardRepository;

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
    std::int64_t mProjectId;
    std::int64_t mCategoryId;

    std::string mBackupDatabasePath;
    std::string mRestoreDatabasePath;
};

class WelcomePage final : public wxWizardPageSimple
{
public:
    WelcomePage() = delete;
    WelcomePage(const WelcomePage&) = delete;
    WelcomePage(SetupWizard* parent, std::shared_ptr<spdlog::logger> logger);
    virtual ~WelcomePage() = default;

    WelcomePage& operator=(const WelcomePage&) = delete;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnWizardCancel(wxWizardEvent& event);

    SetupWizard* pParent;

    std::shared_ptr<spdlog::logger> pLogger;
};

class OptionPage final : public wxWizardPage
{
public:
    OptionPage() = delete;
    OptionPage(const OptionPage&) = delete;
    OptionPage(SetupWizard* parent,
        repos::SetupWizardRepository* setupWizardRepository,
        std::shared_ptr<spdlog::logger> logger,
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

    void OnWizardCancel(wxWizardEvent& event);
    void OnSetupWizardFlowCheck(wxCommandEvent& event);
    void OnRestoreWizardFlowCheck(wxCommandEvent& event);
    void OnSkipWizardFlowCheck(wxCommandEvent& event);

    SetupWizard* pParent;
    repos::SetupWizardRepository* pSetupWizardRepository;
    std::shared_ptr<spdlog::logger> pLogger;
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
        repos::SetupWizardRepository* setupWizardRepository,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath);
    virtual ~CreateEmployerAndClientPage() = default;

    CreateEmployerAndClientPage& operator=(const CreateEmployerAndClientPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnWizardCancel(wxWizardEvent& event);
    void OnWizardPageShown(wxWizardEvent& event);

    SetupWizard* pParent;
    repos::SetupWizardRepository* pSetupWizardRepository;
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
        repos::SetupWizardRepository* setupWizardRepository,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath);
    virtual ~CreateProjectAndCategoryPage() = default;

    CreateProjectAndCategoryPage& operator=(const CreateProjectAndCategoryPage&) = delete;

    virtual bool TransferDataFromWindow() override;

private:
    void CreateControls();
    void ConfigureEventBindings();

    void OnProjectNameChange(wxCommandEvent& event);
    void OnWizardCancel(wxWizardEvent& event);
    void OnWizardPageShown(wxWizardEvent& event);

    SetupWizard* pParent;
    repos::SetupWizardRepository* pSetupWizardRepository;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabasePath;

    wxTextCtrl* pProjectNameTextCtrl;
    wxTextCtrl* pProjectDisplayNameCtrl;
    wxCheckBox* pProjectIsDefaultCtrl;
    wxTextCtrl* pCategoryNameTextCtrl;
    wxColourPickerCtrl* pCategoryColorPickerCtrl;
    wxCheckBox* pCategoryBillableCtrl;

    enum {
        tksIDC_PROJECTNAME = wxID_HIGHEST + 100,
        tksIDC_PROJECTDISPLAYNAME,
        tksIDC_PROJECTISDEFAULT,
        tksIDC_CATEGORYNAME,
        tksIDC_CATEGORYCOLORPICKER,
        tksIDC_CATEGORYBILLABLE,
    };
};

#define ID_SETUPCOMPLETEPAGE wxID_HIGHEST + 1000

class SetupCompletePage final : public wxWizardPageSimple
{
public:
    SetupCompletePage() = delete;
    SetupCompletePage(const SetupCompletePage&) = delete;
    SetupCompletePage(SetupWizard* parent,
        std::shared_ptr<spdlog::logger> logger,
        repos::SetupWizardRepository* setupWizardRepository);
    virtual ~SetupCompletePage();

private:
    void CreateControls();
    void ConfigureEventBindings();
    void DisableBackButton() const;

    void OnSetupCompleteWizardPageShown(wxWizardEvent& event);
    void OnWizardCancel(wxWizardEvent& event);
    void OnSetupCompleteWizardFinished(wxWizardEvent& event);

    SetupWizard* pParent;
    std::shared_ptr<spdlog::logger> pLogger;
    repos::SetupWizardRepository* pSetupWizardRepository;
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
    void ConfigureEventBindings();

    void OnSkipWizardPageShown(wxWizardEvent& event);

    void DisableBackButton() const;

    wxWizard* pParent;
};
} // namespace UI::wizard
} // namespace tks
