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

#include <wx/richtooltip.h>

#include <fmt/format.h>

#include "../../common/common.h"
#include "../../common/constants.h"
#include "../../common/validator.h"

#include "../../core/environment.h"

#include "../../dao/employerdao.h"
#include "../../dao/clientdao.h"

#include "../../models/employermodel.h"

#include "../../utils/utils.h"

namespace tks::UI::wizard
{
SetupWizard::SetupWizard(wxFrame* frame,
    std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Core::Environment> env,
    const std::string& databasePath)
    : wxWizard(frame,
          wxID_ANY,
          "Setup/Restore Wizard",
          wxBitmapBundle::FromSVGFile((env->GetResourcesPath() / Common::Resources::Wizard()).string(),
              wxSize(116, 260)))
    , pLogger(logger)
    , pEnv(env)
    , mDatabasePath(databasePath)
    , pWelcomePage(nullptr)
    , pOptionPage(nullptr)
    , pCreateEmployerAndClientPage(nullptr)
    , pCreateProjectAndCategoryPage(nullptr)
    , pRestoreDatabasePage(nullptr)
    , pSkipWizardPage(nullptr)
    , mEmployerId(-1)
    , mClientId(-1)
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
    pCreateProjectAndCategoryPage = new CreateProjectAndCategoryPage(this);
    pRestoreDatabasePage = new RestoreDatabasePage(this);
    pSkipWizardPage = new SkipWizardPage(this);

    pOptionPage =
        new OptionPage(this, pWelcomePage, pCreateEmployerAndClientPage, pRestoreDatabasePage, pSkipWizardPage);

    pWelcomePage->SetNext(pOptionPage);
    pCreateEmployerAndClientPage->SetPrev(pOptionPage);
    pRestoreDatabasePage->SetPrev(pOptionPage);

    pCreateEmployerAndClientPage->Chain(pCreateProjectAndCategoryPage);

    GetPageAreaSizer()->Add(pWelcomePage);
}

bool SetupWizard::Run()
{
    pLogger->info("SetupWizard::Run - begin initialization of wizard");
    bool wizardSuccess = wxWizard::RunWizard(pWelcomePage);
    pLogger->info("SetupWizard::Run - wizard exited with status \"{0}\"", wizardSuccess);

    return wizardSuccess;
}

void SetupWizard::SetEmployerId(const std::int64_t employerId) const {}

void SetupWizard::SetClientId(const std::int64_t clientId) const {}

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

RestoreDatabasePage::RestoreDatabasePage(wxWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
}

void RestoreDatabasePage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Restore the program with an existing database";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
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

    std::string continueNextMessage = "\n\nTo continue, click 'Finish'";
    auto continueNextLabel = new wxStaticText(this, wxID_ANY, continueNextMessage);

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
    sizer->Add(continueNextLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));

    SetSizerAndFit(sizer);
}

CreateProjectAndCategoryPage::CreateProjectAndCategoryPage(wxWizard* parent)
    : wxWizardPageSimple(parent)
    , pParent(parent)
{
    CreateControls();
}

bool CreateProjectAndCategoryPage::TransferDataFromWindow()
{
    return true;
}

void CreateProjectAndCategoryPage::CreateControls()
{
    auto sizer = new wxBoxSizer(wxVERTICAL);

    std::string welcome = "Setup a project and category";
    auto welcomeLabel = new wxStaticText(this, wxID_ANY, welcome);
    welcomeLabel->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

    sizer->Add(welcomeLabel, wxSizerFlags().Border(wxALL, FromDIP(5)));
}
} // namespace tks::UI::wizard
