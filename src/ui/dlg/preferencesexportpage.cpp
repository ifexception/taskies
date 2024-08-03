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

#include "preferencesexportpage.h"

#include <wx/artprov.h>
#include <wx/dirdlg.h>
#include <wx/richtooltip.h>

#include "../../core/environment.h"

namespace tks::UI::dlg
{
PreferencesExportPage::PreferencesExportPage(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger)
    : wxPanel(parent, wxID_ANY)
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , pExportPathTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , mSelectedItemIndexes()
    , mPresetSettings()
{
    CreateControls();
    ConfigureEventBindings();
    FillControls();
    DataToControls();
}

bool PreferencesExportPage::IsValid()
{
    auto databasePathSelected = pExportPathTextCtrl->GetValue().ToStdString();
    if (databasePathSelected.empty()) {
        auto valMsg = "An export directory is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pExportPathTextCtrl);
        return false;
    }

    return true;
}

void PreferencesExportPage::Save()
{
    pCfg->SetExportPath(pExportPathTextCtrl->GetValue().ToStdString());
    pCfg->SetPresets(mPresetSettings);
}

void PreferencesExportPage::Reset()
{
    pExportPathTextCtrl->ChangeValue(pCfg->GetExportPath());

    pCfg->ClearPresets();
    pPresetsListView->DeleteAllItems();
}

void PreferencesExportPage::CreateControls()
{
    /* Base Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    /* Export box */
    auto exportStaticBox = new wxStaticBox(this, wxID_ANY, "Export");
    auto exportStaticBoxSizer = new wxStaticBoxSizer(exportStaticBox, wxVERTICAL);
    sizer->Add(exportStaticBoxSizer, wxSizerFlags().Expand());

    /* Export path sizer */
    auto exportPathSizer = new wxBoxSizer(wxHORIZONTAL);
    auto exportPathLabel = new wxStaticText(exportStaticBox, wxID_ANY, "Path");

    /* Export path controls */
    pExportPathTextCtrl = new wxTextCtrl(exportStaticBox,
        tksIDC_EXPORT_PATH,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_LEFT | wxTE_READONLY);
    pBrowseExportPathButton = new wxButton(exportStaticBox, tksIDC_EXPORT_PATH_BUTTON, "Browse...");
    pBrowseExportPathButton->SetToolTip("Browse and select a directory to export data to");
    exportPathSizer->Add(exportPathLabel, wxSizerFlags().Left().Border(wxRIGHT, FromDIP(5)).CenterVertical());
    exportPathSizer->Add(
        pExportPathTextCtrl, wxSizerFlags().Border(wxRIGHT | wxLEFT, FromDIP(5)).Expand().Proportion(1));
    exportPathSizer->Add(pBrowseExportPathButton, wxSizerFlags().Border(wxLEFT, FromDIP(5)));
    exportStaticBoxSizer->Add(exportPathSizer, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand().Proportion(1));

    /* Presets box */
    auto presetsStaticBox = new wxStaticBox(this, wxID_ANY, "Presets");
    auto presetsStaticBoxSizer = new wxStaticBoxSizer(presetsStaticBox, wxVERTICAL);
    sizer->Add(presetsStaticBoxSizer, wxSizerFlags().Expand());

    /* Sizers for list view and button */
    auto listAndButtonHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    presetsStaticBoxSizer->Add(listAndButtonHorizontalSizer, wxSizerFlags().Expand().Proportion(1));

    auto listViewSizer = new wxBoxSizer(wxVERTICAL);
    auto buttonSizer = new wxBoxSizer(wxVERTICAL);
    listAndButtonHorizontalSizer->Add(listViewSizer, wxSizerFlags().Expand().Proportion(1));
    listAndButtonHorizontalSizer->Add(buttonSizer, wxSizerFlags().Expand());

    pPresetsListView = new wxListView(presetsStaticBox,
        tksIDC_PRESETS_LIST_VIEW,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pPresetsListView->EnableCheckBoxes();
    pPresetsListView->SetToolTip("View and manage your export presets");
    listViewSizer->Add(pPresetsListView, wxSizerFlags().Left().Border(wxALL, FromDIP(5)).Expand());

    int columnIndex = 0;

    wxListItem presetNamesColumn;
    presetNamesColumn.SetId(columnIndex);
    presetNamesColumn.SetText("Presets");
    presetNamesColumn.SetWidth(100);
    pPresetsListView->InsertColumn(columnIndex++, presetNamesColumn);

    auto providedDeleteBitmap =
        wxArtProvider::GetBitmapBundle(wxART_DELETE, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    pRemovePresetButton = new wxBitmapButton(presetsStaticBox, tksIDC_REMOVE_PRESET_BUTTON, providedDeleteBitmap);
    pRemovePresetButton->SetToolTip("Remove selected preset(s)");
    buttonSizer->Add(pRemovePresetButton, wxSizerFlags().Right().Border(wxALL, FromDIP(5)));

    auto infoLabelSizer = new wxBoxSizer(wxHORIZONTAL);
    presetsStaticBoxSizer->Add(infoLabelSizer, wxSizerFlags().Expand());

    auto providedInfoBitmap =
        wxArtProvider::GetBitmapBundle(wxART_INFORMATION, "wxART_OTHER_C", wxSize(FromDIP(16), FromDIP(16)));
    auto infoStaticBitmap = new wxStaticBitmap(presetsStaticBox, wxID_ANY, providedInfoBitmap);
    infoLabelSizer->Add(infoStaticBitmap, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    auto presetManagementLabel = new wxStaticText(
        presetsStaticBox, wxID_ANY, "Presets creation and management is done from the \"Export to CSV\" dialog");
    presetManagementLabel->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    infoLabelSizer->Add(presetManagementLabel, wxSizerFlags().Border(wxALL, FromDIP(5)).Expand());

    SetSizerAndFit(sizer);
}

// clang-format off
void PreferencesExportPage::ConfigureEventBindings()
{
    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &PreferencesExportPage::OnOpenDirectoryForExportLocation,
        this,
        tksIDC_EXPORT_PATH_BUTTON
    );

    pPresetsListView->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &PreferencesExportPage::OnPresetItemCheck,
        this,
        tksIDC_PRESETS_LIST_VIEW
    );

    pPresetsListView->Bind(
        wxEVT_LIST_ITEM_UNCHECKED,
        &PreferencesExportPage::OnPresetItemUncheck,
        this,
        tksIDC_PRESETS_LIST_VIEW
    );

    pRemovePresetButton->Bind(
        wxEVT_BUTTON,
        &PreferencesExportPage::OnRemovePreset,
        this,
        tksIDC_REMOVE_PRESET_BUTTON
    );
}
// clang-format on

void PreferencesExportPage::FillControls()
{
    mPresetSettings = pCfg->GetPresets();
}

void PreferencesExportPage::DataToControls()
{
    pExportPathTextCtrl->ChangeValue(pCfg->GetExportPath());
    pExportPathTextCtrl->SetToolTip(pCfg->GetExportPath());

    for (const auto& presetSetting : pCfg->GetPresets()) {
        pPresetsListView->InsertItem(0, presetSetting.Name);
    }
}

void PreferencesExportPage::OnOpenDirectoryForExportLocation(wxCommandEvent& event)
{
    std::string directoryToOpen = "";
    if (pCfg->GetExportPath().empty()) {
        directoryToOpen = pEnv->GetExportPath().string();
    } else {
        directoryToOpen = pCfg->GetExportPath();
    }

    auto openDirDialog =
        new wxDirDialog(this, "Select an export directory", directoryToOpen, wxDD_DEFAULT_STYLE, wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        auto selectedExportPath = openDirDialog->GetPath().ToStdString();
        pExportPathTextCtrl->SetValue(selectedExportPath);
        pExportPathTextCtrl->SetToolTip(selectedExportPath);
    }

    openDirDialog->Destroy();
}

void PreferencesExportPage::OnPresetItemCheck(wxListEvent& event)
{
    long index = event.GetIndex();
    mSelectedItemIndexes.push_back(index);

    std::string name;

    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pPresetsListView->GetItem(item);

    name = item.GetText().ToStdString();

    pLogger->info("PreferencesExportPage::OnPresetItemCheck - Selected preset name \"{0}\"", name);
}

void PreferencesExportPage::OnPresetItemUncheck(wxListEvent& event)
{
    long index = event.GetIndex();
    mSelectedItemIndexes.erase(
        std::remove(mSelectedItemIndexes.begin(), mSelectedItemIndexes.end(), index), mSelectedItemIndexes.end());

    std::string name;

    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pPresetsListView->GetItem(item);

    name = item.GetText().ToStdString();

    pLogger->info("PreferencesExportPage::OnPresetItemUncheck - Unselected preset name \"{0}\"", name);
}

void PreferencesExportPage::OnRemovePreset(wxCommandEvent& event)
{
    if (mSelectedItemIndexes.size() == 0) {
        pLogger->info("PreferencesExportPage::OnRemovePreset - No items (presets) selected to remove");
        return;
    }

    // Sort the item indexes by ascending order so the
    // subsequent for loop correctly iterates over the entries in reverse
    std::sort(mSelectedItemIndexes.begin(), mSelectedItemIndexes.end(), std::less{});

    int presetCount = pCfg->GetPresetCount();
    int orderIndex = 0;
    int columnIndex = 0;

    for (long i = (mSelectedItemIndexes.size() - 1); 0 <= i; i--) {
        // Extract the preset name text from item index
        std::string name;
        wxListItem item;
        item.m_itemId = mSelectedItemIndexes[i];
        item.m_col = columnIndex;
        item.m_mask = wxLIST_MASK_TEXT;
        pPresetsListView->GetItem(item);

        name = item.GetText().ToStdString();

        /* Remove preset from preset list control */
        pPresetsListView->DeleteItem(mSelectedItemIndexes[i]);

        mSelectedItemIndexes.erase(mSelectedItemIndexes.begin() + i);

        // clang-format off
        mPresetSettings.erase(
            std::remove_if(
                mPresetSettings.begin(),
                mPresetSettings.end(),
                [&](const Core::Configuration::PresetSettings& preset) {
                    return preset.Name == name;
                }
            ),
            mPresetSettings.end()
        );
        // clang-format on

        presetCount--;

        pLogger->info("PreferencesExportPage::OnRemovePreset - Preset \"{0}\" removed", name);
    }

    pCfg->SetPresetCount(presetCount);
}
} // namespace tks::UI::dlg
