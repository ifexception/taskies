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

#include "setupwizard.h"

#include <wx/filedlg.h>
#include <wx/richtooltip.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"
#include "../../core/configuration.h"

#include "../../dao/employerdao.h"
#include "../../dao/clientdao.h"
#include "../../dao/projectdao.h"
#include "../../dao/categorydao.h"

#include "../../models/employermodel.h"
#include "../../models/clientmodel.h"
#include "../../models/projectmodel.h"
#include "../../models/categorymodel.h"

#include "../../utils/utils.h"

namespace tks::UI::wizard
{
SetupWizard::SetupWizard(wxFrame* frame,
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    const std::string& databasePath)
    : wxWizard(frame,
          wxID_ANY,
          "Setup/Restore Wizard",
          wxBitmapBundle::FromSVGFile((env->GetResourcesPath() / Common::Resources::Wizard()).string(),
              wxSize(116, 260)))
    , pLogger(logger)
    , pEnv(env)
    , pCfg(cfg)
    , mDatabasePath(databasePath)
    , pWelcomePage(nullptr)
    , pOptionPage(nullptr)
    , pCreateEmployerAndClientPage(nullptr)
    , pCreateProjectAndCategoryPage(nullptr)
    , pRestoreDatabasePage(nullptr)
    , pSkipWizardPage(nullptr)
    , mEmployerId(-1)
    , mClientId(-1)
    , mBackupDatabasePath()
    , mRestoreDatabasePath()
{
    pLogger->info("SetupWizard::SetupWizard - set the left side wizard image");
    // Set left side wizard image
    /*auto wizardImage = pEnv->GetResourcesPath() / Common::Resources::Wizard();
    auto wizardImagePath = wizardImage.string();
    SetBitmap(wxBitmapBundle::FromSVGFile(
        (pEnv->GetResourcesPath() / Common::Resources::Wizard()).string(), wxSize(116, 260)));*/

    // Set icon in titlebar
    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);

    pLogger->info("SetupWizard::SetupWizard - initialize pages");
    pWelcomePage = new WelcomePage(this);
    pCreateEmployerAndClientPage = new CreateEmployerAndClientPage(this, pLogger, mDatabasePath);
    pCreateProjectAndCategoryPage = new CreateProjectAndCategoryPage(this, pLogger, mDatabasePath);
    pSetupCompletePage = new SetupCompletePage(this);
    pRestoreDatabasePage = new RestoreDatabasePage(this, pLogger, pEnv, pCfg);
    pRestoreDatabaseResultPage = new RestoreDatabaseResultPage(this, pLogger);
    pSkipWizardPage = new SkipWizardPage(this);

    pOptionPage =
        new OptionPage(this, pWelcomePage, pCreateEmployerAndClientPage, pRestoreDatabasePage, pSkipWizardPage);

    pWelcomePage->SetNext(pOptionPage);
    pCreateEmployerAndClientPage->SetPrev(pOptionPage);
    pRestoreDatabasePage->SetPrev(pOptionPage);

    pCreateEmployerAndClientPage->Chain(pCreateProjectAndCategoryPage);
    pCreateProjectAndCategoryPage->Chain(pSetupCompletePage);

    pRestoreDatabasePage->Chain(pRestoreDatabaseResultPage);

    GetPageAreaSizer()->Add(pWelcomePage);
}

wxWizardPage* SetupWizard::GetFirstPage() const
{
    return pWelcomePage;
}

const std::int64_t SetupWizard::GetEmployerId() const
{
    return mEmployerId;
}

void SetupWizard::SetEmployerId(const std::int64_t employerId)
{
    mEmployerId = employerId;
}

const std::int64_t SetupWizard::GetClientId() const
{
    return mClientId;
}

void SetupWizard::SetClientId(const std::int64_t clientId)
{
    mClientId = clientId;
}

const std::string& SetupWizard::GetBackupDatabasePath() const
{
    return mBackupDatabasePath;
}

void SetupWizard::SetBackupDatabasePath(const std::string& value)
{
    mBackupDatabasePath = value;
}

const std::string& SetupWizard::GetRestoreDatabasePath() const
{
    return mRestoreDatabasePath;
}

void SetupWizard::SetRestoreDatabasePath(const std::string& value)
{
    mRestoreDatabasePath = value;
}

WelcomePage::WelcomePage(SetupWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
}

void WelcomePage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Welcome to the Taskies Setup/Restore Wizard";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    std::string introMessage = "This wizard will help you get Taskies setup or restored to your computer";
    auto introLabel = new wxStaticText(this, wxID_ANY, introMessage);

    std::string continueNextMessage = "To continue, click Next";
    auto continueNextLabel = new wxStaticText(this, wxID_ANY, continueNextMessage);

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(introLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(continueNextLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

OptionPage::OptionPage(SetupWizard* parent,
    wxWizardPage* prev,
    wxWizardPage* nextOption1,
    wxWizardPage* nextOption2,
    wxWizardPage* nextOption3)
    : wxWizardPage(parent)
    , pParent(parent)
    , pPrev(prev)
    , pNextOption1(nextOption1)
    , pNextOption2(nextOption2)
    , pNextOption3(nextOption3)
{
    CreateControls();
    ConfigureEventBindings();
}

wxWizardPage* OptionPage::GetPrev() const
{
    return pPrev;
}

wxWizardPage* OptionPage::GetNext() const
{
    if (pSetupWizardFlowCheckBox->IsChecked()) {
        return pNextOption1;
    } else if (pRestoreWizardFlowCheckBox->IsChecked()) {
        return pNextOption2;
    } else if (pSkipWizardFlowCheckBox->IsChecked()) {
        return pNextOption3;
    } else {
        return pNextOption1;
    }
}

void OptionPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto introductionText = "Please select an option below:";
    auto introductionLabel = new wxStaticText(this, wxID_ANY, introductionText);
    introductionLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    auto defaultOptionText = "(not selecting an option will default to the setup wizard)";
    auto defaultOptionLabel = new wxStaticText(this, wxID_ANY, defaultOptionText);
    defaultOptionLabel->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));

    auto staticBox = new wxStaticBox(this, wxID_ANY, "Options");
    auto staticBoxSizer = new wxStaticBoxSizer(staticBox, wxVERTICAL);

    pSetupWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_SETUPWIZARD_CHECKBOX, "Setup program wizard");
    pRestoreWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_RESTOREWIZARD_CHECKBOX, "Restore database wizard");
    pSkipWizardFlowCheckBox = new wxCheckBox(staticBox, tksIDC_SKIPWIZARD_CHECKBOX, "Skip program wizard");

    staticBoxSizer->Add(pSetupWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));
    staticBoxSizer->Add(pRestoreWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));
    staticBoxSizer->Add(pSkipWizardFlowCheckBox, wxSizerFlags().Border(wxALL, FromDIP(5)));

    sizer->Add(introductionLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(defaultOptionLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(staticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    SetSizerAndFit(sizer);
}

// clang-format off
void OptionPage::ConfigureEventBindings()
{
    pSetupWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnSetupWizardFlowCheck,
        this
    );

    pRestoreWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnRestoreWizardFlowCheck,
        this
    );

    pSkipWizardFlowCheckBox->Bind(
        wxEVT_CHECKBOX,
        &OptionPage::OnSkipWizardFlowCheck,
        this
    );
}
// clang-format on

void OptionPage::OnSetupWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pRestoreWizardFlowCheckBox->SetValue(false);
        pSkipWizardFlowCheckBox->SetValue(false);
    }
}

void OptionPage::OnRestoreWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSetupWizardFlowCheckBox->SetValue(false);
        pSkipWizardFlowCheckBox->SetValue(false);
    }
}

void OptionPage::OnSkipWizardFlowCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSetupWizardFlowCheckBox->SetValue(false);
        pRestoreWizardFlowCheckBox->SetValue(false);
    }
}

CreateEmployerAndClientPage::CreateEmployerAndClientPage(SetupWizard* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath)
    : wxWizardPageSimple(parent)
    , pParent(parent)
    , pLogger(logger)
    , mDatabasePath(databasePath)
{
    CreateControls();
}

bool CreateEmployerAndClientPage::TransferDataFromWindow()
{
    auto employerName = pEmployerNameTextCtrl->GetValue().ToStdString();
    if (employerName.empty()) {
        auto valMsg = "Name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pEmployerNameTextCtrl);
        return false;
    }

    if (employerName.length() < MIN_CHARACTER_COUNT || employerName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pEmployerNameTextCtrl);
        return false;
    }

    auto clientName = pClientNameTextCtrl->GetValue().ToStdString();
    if (!clientName.empty() &&
        (clientName.length() < MIN_CHARACTER_COUNT || clientName.length() > MAX_CHARACTER_COUNT_NAMES)) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pClientNameTextCtrl);
        return false;
    }

    DAO::EmployerDao employerDao(pLogger, mDatabasePath);
    Model::EmployerModel employerModel;
    employerModel.Name = Utils::TrimWhitespace(employerName);

    std::int64_t employerId = employerDao.Create(employerModel);
    if (employerId == -1) {
        wxMessageBox("The setup wizard encountered an unexpected error", "Setup Error", wxOK | wxICON_ERROR, this);
        return false;
    } else {
        pParent->SetEmployerId(employerId);
    }

    if (!clientName.empty()) {
        DAO::ClientDao clientDao(pLogger, mDatabasePath);
        Model::ClientModel clientModel;
        clientModel.Name = Utils::TrimWhitespace(clientName);
        clientModel.EmployerId = employerId;

        std::int64_t clientId = clientDao.Create(clientModel);
        if (clientId == -1) {
            wxMessageBox("The setup wizard encountered an unexpected error", "Setup Error", wxOK | wxICON_ERROR, this);
            return false;
        } else {
            pParent->SetClientId(clientId);
        }
    }

    return true;
}

void CreateEmployerAndClientPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Setup an employer and (optional) client";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Employer */
    auto employerBox = new wxStaticBox(this, wxID_ANY, "Employer");
    auto employerBoxSizer = new wxStaticBoxSizer(employerBox, wxVERTICAL);
    sizer->Add(employerBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Employer Name Control */
    auto employerNameLabel = new wxStaticText(employerBox, wxID_ANY, "Name");

    pEmployerNameTextCtrl = new wxTextCtrl(employerBox, tksIDC_EMPLOYERNAME);
    pEmployerNameTextCtrl->SetHint("Employer name");
    pEmployerNameTextCtrl->SetToolTip("Enter a name for an employer");

    pEmployerNameTextCtrl->SetValidator(NameValidator());

    auto employerDetailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    employerDetailsGridSizer->AddGrowableCol(1, 1);

    employerDetailsGridSizer->Add(employerNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    employerDetailsGridSizer->Add(
        pEmployerNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    employerBoxSizer->Add(employerDetailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Client */
    auto clientBox = new wxStaticBox(this, wxID_ANY, "Client");
    auto clientBoxSizer = new wxStaticBoxSizer(clientBox, wxVERTICAL);
    sizer->Add(clientBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Client Name control */
    auto clientNameLabel = new wxStaticText(clientBox, wxID_ANY, "Name");

    pClientNameTextCtrl = new wxTextCtrl(clientBox, tksIDC_CLIENTNAME);
    pClientNameTextCtrl->SetHint("Client name");
    pClientNameTextCtrl->SetToolTip("Enter a name for a client");

    pClientNameTextCtrl->SetValidator(NameValidator());

    auto clienbtDetailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    clienbtDetailsGridSizer->AddGrowableCol(1, 1);

    clienbtDetailsGridSizer->Add(clientNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    clienbtDetailsGridSizer->Add(pClientNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    clientBoxSizer->Add(clienbtDetailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

CreateProjectAndCategoryPage::CreateProjectAndCategoryPage(SetupWizard* parent,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath)
    : wxWizardPageSimple(parent)
    , pParent(parent)
    , pLogger(logger)
    , mDatabasePath(databasePath)
{
    CreateControls();
    ConfigureEventBindings();
}

bool CreateProjectAndCategoryPage::TransferDataFromWindow()
{
    // Validate Project properties
    auto projectName = pProjectNameTextCtrl->GetValue().ToStdString();
    if (projectName.empty()) {
        auto valMsg = "Project name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pProjectNameTextCtrl);
        return false;
    }

    if (projectName.length() < MIN_CHARACTER_COUNT || projectName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pProjectNameTextCtrl);
        return false;
    }

    auto projectDisplayName = pProjectDisplayNameCtrl->GetValue().ToStdString();
    if (projectDisplayName.empty()) {
        auto valMsg = "Display name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pProjectDisplayNameCtrl);
        return false;
    }

    if (projectDisplayName.length() < MIN_CHARACTER_COUNT || projectDisplayName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Display name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pProjectDisplayNameCtrl);
        return false;
    }

    // Save project
    DAO::ProjectDao projectDao(pLogger, mDatabasePath);
    Model::ProjectModel project;
    project.Name = Utils::TrimWhitespace(projectName);
    project.DisplayName = Utils::TrimWhitespace(projectDisplayName);
    project.IsDefault = pProjectIsDefaultCtrl->GetValue();
    project.EmployerId = pParent->GetEmployerId();
    project.ClientId =
        pParent->GetClientId() == -1 ? std::nullopt : std::make_optional<std::int64_t>(pParent->GetClientId());

    std::int64_t projectId = projectDao.Create(project);
    if (projectId == -1) {
        wxMessageBox("The setup wizard encountered an unexpected error", "Setup Error", wxOK | wxICON_ERROR, this);
        return false;
    }

    // Validate category properties
    auto categoryName = pCategoryNameTextCtrl->GetValue().ToStdString();
    if (categoryName.empty()) {
        auto valMsg = "Category name is required";
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pCategoryNameTextCtrl);
        return false;
    }

    if (categoryName.length() < MIN_CHARACTER_COUNT || categoryName.length() > MAX_CHARACTER_COUNT_NAMES) {
        auto valMsg = fmt::format("Name must be at minimum {0} or maximum {1} characters long",
            MIN_CHARACTER_COUNT,
            MAX_CHARACTER_COUNT_NAMES);
        wxRichToolTip toolTip("Validation", valMsg);
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pCategoryNameTextCtrl);
        return false;
    }

    // Save category
    DAO::CategoryDao categoryDao(pLogger, mDatabasePath);
    Model::CategoryModel category;
    category.Name = Utils::TrimWhitespace(categoryName);
    category.Color = pColorPickerCtrl->GetColour().GetRGB();
    category.Billable = pBillableCtrl->GetValue();
    category.ProjectId = std::make_optional<std::int64_t>(projectId);

    std::int64_t categoryId = categoryDao.Create(category);
    if (categoryId == -1) {
        wxMessageBox("The setup wizard encountered an unexpected error", "Setup Error", wxOK | wxICON_ERROR, this);
        return false;
    }

    return true;
}

void CreateProjectAndCategoryPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Setup a project and category";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Project Box */
    auto projectBox = new wxStaticBox(this, wxID_ANY, "Project");
    auto projectBoxSizer = new wxStaticBoxSizer(projectBox, wxVERTICAL);
    sizer->Add(projectBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Project Name Ctrl */
    auto projectNameLabel = new wxStaticText(projectBox, wxID_ANY, "Name");

    pProjectNameTextCtrl = new wxTextCtrl(projectBox, tksIDC_PROJECTNAME);
    pProjectNameTextCtrl->SetHint("Project name");
    pProjectNameTextCtrl->SetToolTip("Enter a name for a project");
    pProjectNameTextCtrl->SetValidator(NameValidator());

    /* Display Name Ctrl */
    auto displayNameLabel = new wxStaticText(projectBox, wxID_ANY, "Display Name");

    pProjectDisplayNameCtrl = new wxTextCtrl(projectBox, tksIDC_PROJECTDISPLAYNAME);
    pProjectDisplayNameCtrl->SetHint("Display name");
    pProjectDisplayNameCtrl->SetToolTip("Enter a nickname, abbreviation or common name for a project (if applicable)");
    pProjectDisplayNameCtrl->SetValidator(NameValidator());

    /* Is Default Checkbox Ctrl */
    pProjectIsDefaultCtrl = new wxCheckBox(projectBox, tksIDC_PROJECTISDEFAULT, "Is Default");
    pProjectIsDefaultCtrl->SetToolTip("Enabling this option for a project will auto-select it on a task entry");

    /* Details Grid Sizer */
    auto projectDetailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    projectDetailsGridSizer->AddGrowableCol(1, 1);

    projectDetailsGridSizer->Add(projectNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    projectDetailsGridSizer->Add(pProjectNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    projectDetailsGridSizer->Add(displayNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    projectDetailsGridSizer->Add(
        pProjectDisplayNameCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    projectDetailsGridSizer->Add(0, 0);
    projectDetailsGridSizer->Add(pProjectIsDefaultCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    projectBoxSizer->Add(projectDetailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Category Box */
    auto categoryBox = new wxStaticBox(this, wxID_ANY, "Category");
    auto categoryBoxSizer = new wxStaticBoxSizer(categoryBox, wxVERTICAL);
    sizer->Add(categoryBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Name Ctrl */
    auto categoryNameLabel = new wxStaticText(categoryBox, wxID_ANY, "Name");

    pCategoryNameTextCtrl = new wxTextCtrl(categoryBox, tksIDC_CATEGORYNAME);
    pCategoryNameTextCtrl->SetHint("Category name");
    pCategoryNameTextCtrl->SetToolTip("Enter a name for a category");

    pCategoryNameTextCtrl->SetValidator(NameValidator());

    /* Color Picker Ctrl */
    pColorPickerCtrl = new wxColourPickerCtrl(categoryBox, tksIDC_CATEGORYCOLORPICKER);
    pColorPickerCtrl->SetToolTip("Pick a color to associate with the category");

    pBillableCtrl = new wxCheckBox(categoryBox, tksIDC_CATEGORYBILLABLE, "Billable");
    pBillableCtrl->SetToolTip("Indicates if a task captured with this category is billable");

    /* Details Grid Sizer */
    auto categoryDetailsGridSizer = new wxFlexGridSizer(2, FromDIP(7), FromDIP(25));
    categoryDetailsGridSizer->AddGrowableCol(1, 1);

    categoryDetailsGridSizer->Add(categoryNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    categoryDetailsGridSizer->Add(
        pCategoryNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    categoryDetailsGridSizer->Add(0, 0);
    categoryDetailsGridSizer->Add(pColorPickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    categoryDetailsGridSizer->Add(0, 0);
    categoryDetailsGridSizer->Add(pBillableCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    categoryBoxSizer->Add(categoryDetailsGridSizer, wxSizerFlags().Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

// clang-format off
void CreateProjectAndCategoryPage::ConfigureEventBindings()
{
    pProjectNameTextCtrl->Bind(
        wxEVT_TEXT,
        &CreateProjectAndCategoryPage::OnProjectNameChange,
        this
    );
}
// clang-format on

void CreateProjectAndCategoryPage::OnProjectNameChange(wxCommandEvent& event)
{
    auto name = pProjectNameTextCtrl->GetValue().ToStdString();
    pProjectDisplayNameCtrl->ChangeValue(name);
}

SetupCompletePage::SetupCompletePage(SetupWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
    DisableBackButton();
}

void SetupCompletePage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string labelMessage = "The wizard has completed setting up\nTaskies on your computer";
    auto label = new wxStaticText(this, wxID_ANY, labelMessage);
    label->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    std::string continueNextMessage = "\n\nTo exit the wizard, click 'Finish'";
    auto continueNextLabel = new wxStaticText(this, wxID_ANY, continueNextMessage);

    sizer->Add(label, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(continueNextLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

void SetupCompletePage::DisableBackButton() const
{
    auto backButton = FindWindowById(wxID_BACKWARD, GetParent());
    backButton->Disable();
}

// -- Restore Wizard
RestoreDatabasePage::RestoreDatabasePage(SetupWizard* parent,
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg)
    : wxWizardPageSimple(parent)
    , pParent(parent)
    , pLogger(logger)
    , pEnv(env)
    , pCfg(cfg)
{
    CreateControls();
    ConfigureEventBindings();
}

void RestoreDatabasePage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Restore the program with an existing database";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    /* Backup box */
    auto backupBox = new wxStaticBox(this, wxID_ANY, "Backup");
    auto backupBoxSizer = new wxStaticBoxSizer(backupBox, wxVERTICAL);
    sizer->Add(backupBoxSizer, wxSizerFlags().Expand());

    /* Backup path sizer */
    auto backupPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto backupPathLabel = new wxStaticText(backupBox, wxID_ANY, "Path");
    pBackupPathTextCtrl = new wxTextCtrl(
        backupBox, tksIDC_BACKUP_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseBackupPathButton = new wxButton(backupBox, tksIDC_BACKUP_PATH_BUTTON, "Browse...");
    pBrowseBackupPathButton->SetToolTip("Browse and select the backups directory");
    backupPathSizer->Add(backupPathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    backupPathSizer->Add(
        pBackupPathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    backupPathSizer->Add(pBrowseBackupPathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    backupBoxSizer->Add(backupPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Restore box */
    auto restoreBox = new wxStaticBox(this, wxID_ANY, "Restore");
    auto restoreBoxSizer = new wxStaticBoxSizer(restoreBox, wxVERTICAL);
    sizer->Add(restoreBoxSizer, wxSizerFlags().Expand());

    /* Backup path sizer*/
    auto restorePathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto restorePathLabel = new wxStaticText(restoreBox, wxID_ANY, "Path");
    pRestorePathTextCtrl = new wxTextCtrl(
        restoreBox, tksIDC_RESTORE_PATH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_READONLY);
    pBrowseRestorePathButton = new wxButton(restoreBox, tksIDC_RESTORE_PATH_BUTTON, "Browse...");
    pBrowseRestorePathButton->SetToolTip("Browse and select the restore directory");
    restorePathSizer->Add(restorePathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    restorePathSizer->Add(
        pRestorePathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    restorePathSizer->Add(pBrowseRestorePathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    restoreBoxSizer->Add(restorePathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

// clang-format off
void RestoreDatabasePage::ConfigureEventBindings()
{
    pBrowseBackupPathButton->Bind(
        wxEVT_BUTTON,
        &RestoreDatabasePage::OnOpenFileForBackupLocation,
        this,
        tksIDC_BACKUP_PATH_BUTTON
    );

    pBrowseRestorePathButton->Bind(
        wxEVT_BUTTON,
        &RestoreDatabasePage::OnOpenFileForRestoreLocation,
        this,
        tksIDC_RESTORE_PATH_BUTTON
    );
}
// clang-format on

void RestoreDatabasePage::OnOpenFileForBackupLocation(wxCommandEvent& event)
{
    // std::string pathDirectoryToOpenOn = pCfg->GetBackupPath();
    std::string pathDirectoryToOpenOn;
    if (pCfg->GetBackupPath().empty()) {
        pathDirectoryToOpenOn = pCfg->GetDatabasePath();
    } else {
        pathDirectoryToOpenOn = pCfg->GetBackupPath();
    }

    auto openFileDialog = new wxFileDialog(this,
        "Select a backup database file to restore from",
        pathDirectoryToOpenOn,
        wxEmptyString,
        "DB files (*.db)|*.db",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int ret = openFileDialog->ShowModal();

    if (ret == wxID_OK) {
        auto selectedBackupPath = openFileDialog->GetPath().ToStdString();
        pBackupPathTextCtrl->ChangeValue(selectedBackupPath);
        pBackupPathTextCtrl->SetToolTip(selectedBackupPath);

        pParent->SetBackupDatabasePath(selectedBackupPath);
    }

    openFileDialog->Destroy();
}

void RestoreDatabasePage::OnOpenFileForRestoreLocation(wxCommandEvent& event)
{
    std::string pathDirectoryToOpenOn;
    if (pCfg->GetDatabasePath().empty()) {
        pathDirectoryToOpenOn = pEnv->GetDatabasePath().string();
    } else {
        pathDirectoryToOpenOn = pCfg->GetDatabasePath();
    }

    auto fullPath = std::filesystem::path(pathDirectoryToOpenOn);
    auto& pathWithoutFileName = fullPath.remove_filename();
    pathDirectoryToOpenOn = pathWithoutFileName.string();

    auto openFileDialog = new wxFileDialog(this,
        "Select a restore database file",
        pathDirectoryToOpenOn,
        wxEmptyString,
        "DB files (*.db)|*.db",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    int res = openFileDialog->ShowModal();

    if (res == wxID_OK) {
        auto selectedPath = openFileDialog->GetPath().ToStdString();
        pRestorePathTextCtrl->SetValue(selectedPath);
        pRestorePathTextCtrl->SetToolTip(selectedPath);

        pParent->SetRestoreDatabasePath(selectedPath);
    }

    openFileDialog->Destroy();
}

RestoreDatabaseResultPage::RestoreDatabaseResultPage(SetupWizard* parent, std::shared_ptr<spdlog::logger> logger)
    : wxWizardPageSimple(parent)
    , pParent(parent)
    , pLogger(logger)
{
    CreateControls();
    ConfigureEventBindings();
    DisableBackButton();
}

void RestoreDatabaseResultPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Restoring database";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    pRestoreProgressGaugeCtrl = new wxGauge(this, wxID_ANY, 100);
    sizer->Add(pRestoreProgressGaugeCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pStatusFeedbackLabel = new wxStaticText(this, wxID_ANY, wxEmptyString);
    sizer->Add(pStatusFeedbackLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).Left());

    SetSizerAndFit(sizer);
}

// clang-format off
void RestoreDatabaseResultPage::ConfigureEventBindings()
{
    Bind(
        wxEVT_WIZARD_PAGE_SHOWN,
        &RestoreDatabaseResultPage::OnWizardPageShown,
        this
    );
}
// clang-format on

void RestoreDatabaseResultPage::DisableBackButton() const
{
    auto backButton = FindWindowById(wxID_BACKWARD, GetParent());
    backButton->Disable();
}

void RestoreDatabaseResultPage::OnWizardPageShown(wxWizardEvent& WXUNUSED(event))
{
    pRestoreProgressGaugeCtrl->Pulse();

    int rc = 0;
    std::string backupDatabasePath = pParent->GetBackupDatabasePath();
    std::string restoreDatabasePath = pParent->GetRestoreDatabasePath();

    sqlite3* backupDb = nullptr;
    sqlite3* restoreDb = nullptr;
    sqlite3_backup* backup = nullptr;

    rc = sqlite3_open(backupDatabasePath.c_str(), &backupDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(backupDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "RestoreDatabaseResultPage::OnWizardPageShown",
            backupDatabasePath,
            rc,
            err);
        return;
    }

    rc = sqlite3_open(restoreDatabasePath.c_str(), &restoreDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(restoreDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "RestoreDatabaseResultPage::OnWizardPageShown",
            restoreDatabasePath,
            rc,
            err);
        return;
    }

    backup = sqlite3_backup_init(/*destination*/ restoreDb, "main", /*source*/ backupDb, "main");
    if (backup) {
        sqlite3_backup_step(backup, -1);
        sqlite3_backup_finish(backup);
    }

    rc = sqlite3_errcode(restoreDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(restoreDb);
        pLogger->error("{0} - Failed to restore database to {1}. Error {2}: \"{3}\"",
            "MainFrame::OnTasksBackupDatabase",
            restoreDatabasePath,
            rc,
            err);
        return;
    }

    sqlite3_close(restoreDb);
    sqlite3_close(backupDb);

    /* Complete operation */
    std::string continueNextMessage = "\n\n\nTo exit the wizard, click 'Finish'";
    auto statusComplete = "The wizard has restored the database successfully!" + continueNextMessage;
    pStatusFeedbackLabel->SetLabel(statusComplete);
    pRestoreProgressGaugeCtrl->SetValue(100);
}

SkipWizardPage::SkipWizardPage(wxWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
}

void SkipWizardPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Wizard skipped";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    std::string continueNextMessage = "\n\nTo exit the wizard, click 'Finish'";
    auto continueNextLabel = new wxStaticText(this, wxID_ANY, continueNextMessage);

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(continueNextLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}
} // namespace tks::UI::wizard
