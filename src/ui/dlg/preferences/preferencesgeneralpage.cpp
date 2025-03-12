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

#include "preferencesgeneralpage.h"

#include <wx/richtooltip.h>
#include <wx/stdpaths.h>
#include <wx/msw/registry.h>

#include "../../../common/common.h"
#include "../../../core/configuration.h"
#include "../../clientdata.h"

#ifdef _WIN32
namespace
{
struct StartWithWindowsRegKey {
    StartWithWindowsRegKey(std::shared_ptr<spdlog::logger> logger)
        : pLogger(logger)
        , mKey(wxRegKey::HKCU, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run")
    {
    }

    void Create()
    {
        auto executablePath = wxStandardPaths::Get().GetExecutablePath().ToStdString();
        if (!mKey.SetValue(tks::Common::GetProgramName(), executablePath)) {
            pLogger->error(
                "StartWithWindowsRegKey - Failed to set registry key of \"{0}\" with value \"{1}\"",
                tks::Common::GetProgramName(),
                executablePath);
        }
    }

    void Delete()
    {
        if (!mKey.DeleteValue(tks::Common::GetProgramName())) {
            pLogger->error("StartWithWindowsRegKey - Failed to delete registry key of \"{0}\"",
                tks::Common::GetProgramName());
        }
    }

    bool Exists() const
    {
        return mKey.Exists();
    }

    std::shared_ptr<spdlog::logger> pLogger;
    wxRegKey mKey;
};
} // namespace
#endif

namespace tks::UI::dlg
{
PreferencesGeneralPage::PreferencesGeneralPage(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pCfg(cfg)
    , pLogger(logger)
    , pUserInterfaceLanguageCtrl(nullptr)
    , pStartWithWindowsCtrl(nullptr)
    , pWindowStartPositionCtrl(nullptr)
    , pShowInTrayCtrl(nullptr)
    , pMinimizeToTrayCtrl(nullptr)
    , pCloseToTrayCtrl(nullptr)
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesGeneralPage::IsValid()
{
    int langIndex = pUserInterfaceLanguageCtrl->GetSelection();
    if (langIndex == 0) {
        auto valMsg = "A user interface language selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pUserInterfaceLanguageCtrl);
        return false;
    }

    int startPosIndex = pWindowStartPositionCtrl->GetSelection();
    if (startPosIndex == 0) {
        auto valMsg = "A start position selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pWindowStartPositionCtrl);
        return false;
    }
    return true;
}

void PreferencesGeneralPage::Save()
{
    int langIndex = pUserInterfaceLanguageCtrl->GetSelection();
    ClientData<std::string>* langData = reinterpret_cast<ClientData<std::string>*>(
        pUserInterfaceLanguageCtrl->GetClientObject(langIndex));

    int startPosIndex = pWindowStartPositionCtrl->GetSelection();
    ClientData<WindowState>* startPosData = reinterpret_cast<ClientData<WindowState>*>(
        pWindowStartPositionCtrl->GetClientObject(startPosIndex));

    if (langData->GetValue() != pCfg->GetUserInterfaceLanguage()) {
        // program will need a restart
    }

    pCfg->SetUserInterfaceLanguage(langData->GetValue());
    pCfg->StartOnBoot(pStartWithWindowsCtrl->GetValue());
    pCfg->SetWindowState(startPosData->GetValue());

    {
        StartWithWindowsRegKey key(pLogger);
        if (key.Exists() && !pCfg->StartOnBoot()) {
            key.Delete();
        }
        if (!key.Exists() && pCfg->StartOnBoot()) {
            key.Create();
        }
    }

    pCfg->ShowInTray(pShowInTrayCtrl->GetValue());
    pCfg->MinimizeToTray(pMinimizeToTrayCtrl->GetValue());
    pCfg->CloseToTray(pCloseToTrayCtrl->GetValue());
}

void PreferencesGeneralPage::Reset()
{
    pUserInterfaceLanguageCtrl->Append("en-US", new ClientData<std::string>("en-US"));
    pUserInterfaceLanguageCtrl->SetSelection(1);
    pStartWithWindowsCtrl->SetValue(pCfg->StartOnBoot());

    pWindowStartPositionCtrl->SetSelection(static_cast<int>(pCfg->GetWindowState()));

    pShowInTrayCtrl->SetValue(pCfg->ShowInTray());
    pMinimizeToTrayCtrl->SetValue(pCfg->MinimizeToTray());
    pCloseToTrayCtrl->SetValue(pCfg->CloseToTray());

    if (!pCfg->ShowInTray()) {
        pMinimizeToTrayCtrl->Disable();
        pCloseToTrayCtrl->Disable();
    }
}

void PreferencesGeneralPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* User Interface box */
    auto uiBox = new wxStaticBox(this, wxID_ANY, "User Interface");
    auto uiBoxSizer = new wxStaticBoxSizer(uiBox, wxHORIZONTAL);
    sizer->Add(uiBoxSizer, wxSizerFlags().Expand());
    auto uiGridSizer = new wxFlexGridSizer(2, FromDIP(10), FromDIP(10));
    uiGridSizer->AddGrowableCol(1, 1);

    /* Language label*/
    auto languageLabel = new wxStaticText(uiBox, wxID_ANY, "Language");

    pUserInterfaceLanguageCtrl = new wxChoice(uiBox, tksIDC_LANG);
    pUserInterfaceLanguageCtrl->SetToolTip("Set the language for the program to use");

    uiGridSizer->Add(languageLabel, wxSizerFlags().CenterVertical());
    uiGridSizer->Add(pUserInterfaceLanguageCtrl, wxSizerFlags().Right().Expand().Proportion(1));

    uiBoxSizer->Add(uiGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Misc options */
    auto miscBox = new wxStaticBox(this, wxID_ANY, "Miscellaneous");
    auto miscBoxSizer = new wxStaticBoxSizer(miscBox, wxVERTICAL);
    sizer->Add(miscBoxSizer, wxSizerFlags().Expand());
    auto miscGridSizer = new wxFlexGridSizer(2, FromDIP(10), FromDIP(10));
    miscGridSizer->AddGrowableCol(1, 1);

    /* Start with Windows */
    pStartWithWindowsCtrl = new wxCheckBox(miscBox, wxID_ANY, "Start with Windows");
    pStartWithWindowsCtrl->SetToolTip("Program gets launched by Windows on start");
    miscGridSizer->Add(pStartWithWindowsCtrl, wxSizerFlags().CenterVertical());
    miscGridSizer->Add(0, 0);

    /* Start Position */
    auto startPositionLabel = new wxStaticText(miscBox, wxID_ANY, "Start Position");

    pWindowStartPositionCtrl = new wxChoice(miscBox, tksIDC_START_POSITION);
    pWindowStartPositionCtrl->SetToolTip("Select the state of the program when launched");
    miscGridSizer->Add(startPositionLabel, wxSizerFlags().CenterVertical());
    miscGridSizer->Add(
        pWindowStartPositionCtrl, wxSizerFlags().Right().CenterVertical().Proportion(1));
    miscBoxSizer->Add(
        miscGridSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* System Tray */
    auto systemTrayBox = new wxStaticBox(this, wxID_ANY, "System Tray");
    auto systemTrayBoxSizer = new wxStaticBoxSizer(systemTrayBox, wxVERTICAL);
    sizer->Add(systemTrayBoxSizer, wxSizerFlags().Expand());
    auto systemTrayFlexSizer = new wxFlexGridSizer(1, FromDIP(10), FromDIP(10));
    systemTrayFlexSizer->AddGrowableCol(0, 1);

    pShowInTrayCtrl =
        new wxCheckBox(systemTrayBox, tksIDC_SHOW_IN_TRAY, "Show Taskies in the system tray area");
    pMinimizeToTrayCtrl =
        new wxCheckBox(systemTrayBox, tksIDC_MINIMIZE_TO_TRAY, "Minimize to the system tray area");
    pCloseToTrayCtrl =
        new wxCheckBox(systemTrayBox, tksIDC_CLOSE_TO_TRAY, "Close to the system tray area");

    systemTrayFlexSizer->Add(pShowInTrayCtrl);
    systemTrayFlexSizer->Add(pMinimizeToTrayCtrl, wxSizerFlags().Border(wxLEFT, FromDIP(15)));
    systemTrayFlexSizer->Add(pCloseToTrayCtrl, wxSizerFlags().Border(wxLEFT, FromDIP(15)));
    systemTrayBoxSizer->Add(
        systemTrayFlexSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesGeneralPage::ConfigureEventBindings()
{
    pShowInTrayCtrl->Bind(
        wxEVT_CHECKBOX,
        &PreferencesGeneralPage::OnShowInTrayCheck,
        this
    );
}
// clang-format on

void PreferencesGeneralPage::FillControls()
{
    pUserInterfaceLanguageCtrl->Append("Please Select");
    pWindowStartPositionCtrl->Append("Please Select");
    pWindowStartPositionCtrl->Append("Normal", new ClientData<WindowState>(WindowState::Normal));
    pWindowStartPositionCtrl->Append(
        "Minimized", new ClientData<WindowState>(WindowState::Minimized));
    pWindowStartPositionCtrl->Append("Hidden", new ClientData<WindowState>(WindowState::Hidden));
    pWindowStartPositionCtrl->Append(
        "Maximized", new ClientData<WindowState>(WindowState::Maximized));

    pUserInterfaceLanguageCtrl->SetSelection(0);
    pWindowStartPositionCtrl->SetSelection(0);
}

void PreferencesGeneralPage::DataToControls()
{
    pUserInterfaceLanguageCtrl->Append("en-US", new ClientData<std::string>("en-US"));
    pUserInterfaceLanguageCtrl->SetSelection(1);
    pStartWithWindowsCtrl->SetValue(pCfg->StartOnBoot());

    pWindowStartPositionCtrl->SetSelection(static_cast<int>(pCfg->GetWindowState()));

    pShowInTrayCtrl->SetValue(pCfg->ShowInTray());
    pMinimizeToTrayCtrl->SetValue(pCfg->MinimizeToTray());
    pCloseToTrayCtrl->SetValue(pCfg->CloseToTray());

    if (!pCfg->ShowInTray()) {
        pMinimizeToTrayCtrl->Disable();
        pCloseToTrayCtrl->Disable();
    }
}

void PreferencesGeneralPage::OnShowInTrayCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pMinimizeToTrayCtrl->Enable();
        pCloseToTrayCtrl->Enable();
    } else {
        pMinimizeToTrayCtrl->Disable();
        pMinimizeToTrayCtrl->SetValue(false);
        pCloseToTrayCtrl->Disable();
        pCloseToTrayCtrl->SetValue(false);
    }
}
} // namespace tks::UI::dlg
