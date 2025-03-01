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

#include "quickexporttocsvdlg.h"

#include <fmt/format.h>

#include <wx/clipbrd.h>
#include <wx/dirdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/enums.h"

#include "../../core/configuration.h"

#include "../../services/export/availablecolumns.h"
#include "../../services/export/columnexportmodel.h"
#include "../../services/export/columnjoinprojection.h"
#include "../../services/export/projection.h"
#include "../../services/export/projectionbuilder.h"

#include "../clientdata.h"
#include "../events.h"
#include "../notificationclientdata.h"

namespace
{
// This date was selected arbitrarily
// wxDatePickerCtrl needs a from and to date for the range
// So we pick 2020-01-01 as that date
// Conceivably, a user shouldn't go that far back
wxDateTime MakeMaximumFromDate()
{
    wxDateTime maxFromDate = wxDateTime::Now();
    maxFromDate.SetYear(2020);
    maxFromDate.SetMonth(wxDateTime::Jan);
    maxFromDate.SetDay(1);

    return maxFromDate;
}
} // namespace

namespace tks::UI::dlg
{
QuickExportToCsvDialog::QuickExportToCsvDialog(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Quick Export to CSV",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX,
          name)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pDateStore(nullptr)
    , pExportToClipboardCheckBoxCtrl(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , mFromCtrlDate()
    , mToCtrlDate()
    , mToLatestPossibleDate()
    , pPresetsChoiceCtrl(nullptr)
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
    , bExportToClipboard(false)
    , bExportTodaysTasksOnly(false)
    , mCsvOptions()
    , mCsvExporter(pCfg->GetDatabasePath(), pLogger)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    Create();
    SetSize(wxSize(FromDIP(640), -1));

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void QuickExportToCsvDialog::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void QuickExportToCsvDialog::CreateControls()
{
    /* Main sizer window */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    /* Output static box (top) */
    auto outputStaticBox = new wxStaticBox(this, wxID_ANY, "Output");
    auto outputStaticBoxSizer = new wxStaticBoxSizer(outputStaticBox, wxVERTICAL);
    mainSizer->Add(outputStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto outputFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    outputStaticBoxSizer->Add(outputFlexGridSizer, wxSizerFlags().Expand());

    /* Export to clipboard checkbox control */
    pExportToClipboardCheckBoxCtrl =
        new wxCheckBox(outputStaticBox, tksIDC_COPY_TO_CLIPBOARD_CTRL, "Copy to clipboard");
    pExportToClipboardCheckBoxCtrl->SetToolTip(
        "If selected, the data will be copied to the clipboard");

    /* Save to file text control */
    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVE_TO_FILE_CTRL, wxEmptyString);

    pBrowseExportPathButton =
        new wxButton(outputStaticBox, tksIDC_BROWSE_EXPORT_PATH_CTRL, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set the path on where to the save the exported data to");

    outputFlexGridSizer->AddGrowableCol(1, 1);

    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(
        pExportToClipboardCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(2)));
    outputFlexGridSizer->Add(
        saveToFileLabel, wxSizerFlags().Border(wxALL, FromDIP(2)).CenterVertical());
    outputFlexGridSizer->Add(
        pSaveToFileTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand().Proportion(1));
    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(
        pBrowseExportPathButton, wxSizerFlags().Border(wxALL, FromDIP(2)).Right());

    /* Date range static box */
    auto dateRangeStaticBox = new wxStaticBox(this, wxID_ANY, "Date Range");
    auto dateRangeStaticBoxSizer = new wxStaticBoxSizer(dateRangeStaticBox, wxHORIZONTAL);
    mainSizer->Add(dateRangeStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* From date control */
    auto fromDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "From: ");
    pFromDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_FROM_CTRL);
    pFromDatePickerCtrl->SetToolTip("Set the earliest inclusive date to export the data from");

    /* To date control */
    auto toDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "To: ");
    pToDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_TO_CTRL);
    pToDatePickerCtrl->SetToolTip("Set the latest inclusive date to export the data from");

    /* Export only todays tasks check box control */
    pExportTodaysTasksOnlyCheckBoxCtrl = new wxCheckBox(
        dateRangeStaticBox, tksIDC_EXPORTTODAYSTASKSONLYCHECKBOXCTRL, "Export today's tasks only");
    pExportTodaysTasksOnlyCheckBoxCtrl->SetToolTip(
        "If selected, only tasks logged for today's date will be exported");

    dateRangeStaticBoxSizer->Add(
        fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pFromDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(
        toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pToDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(pExportTodaysTasksOnlyCheckBoxCtrl,
        wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());

    /* Presets static box */
    auto presetsStaticBox = new wxStaticBox(this, wxID_ANY, "Presets");
    auto presetsStaticBoxSizer = new wxStaticBoxSizer(presetsStaticBox, wxHORIZONTAL);
    mainSizer->Add(presetsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto presetsChoiceLabel = new wxStaticText(presetsStaticBox, wxID_ANY, "Preset");
    pPresetsChoiceCtrl = new wxChoice(presetsStaticBox, tksIDC_PRESET_CHOICE_CTRL);

    presetsStaticBoxSizer->Add(
        presetsChoiceLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    presetsStaticBoxSizer->Add(
        pPresetsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    mainSizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pOKButton = new wxButton(this, wxID_OK, "OK");
    pOKButton->SetDefault();
    pOKButton->SetFocus();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Close");

    buttonsSizer->Add(pOKButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(mainSizer);
}

void QuickExportToCsvDialog::FillControls()
{
    /* Export File Controls */
    auto saveToFile = fmt::format(
        "{0}\\taskies-tasks-export-{1}.csv", pCfg->GetExportPath(), pDateStore->PrintTodayDate);
    pSaveToFileTextCtrl->ChangeValue(saveToFile);
    pSaveToFileTextCtrl->SetToolTip(saveToFile);

    /* Date Controls */
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();

    /* Presets controls */
    pPresetsChoiceCtrl->Append("(none)", new ClientData<std::string>(""));
    pPresetsChoiceCtrl->SetSelection(0);

    for (const auto& preset : pCfg->GetPresets()) {
        pPresetsChoiceCtrl->Append(preset.Name, new ClientData<std::string>(preset.Uuid));
    }

    auto presets = pCfg->GetPresets();
    const auto& defaultPresetToApplyIterator = std::find_if(
        presets.begin(), presets.end(), [&](const Core::Configuration::PresetSettings& preset) {
            return preset.IsDefault == true;
        });

    if (defaultPresetToApplyIterator == presets.end()) {
        pLogger->info("ExportToCsvDialog::FillControls - No default preset found");
    } else {
        auto& selectedPresetToApply = *defaultPresetToApplyIterator;
        ApplyPreset(selectedPresetToApply);

        pPresetsChoiceCtrl->SetStringSelection(selectedPresetToApply.Name);
    }
}

// clang-format off
void QuickExportToCsvDialog::ConfigureEventBindings()
{
    pExportToClipboardCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &QuickExportToCsvDialog::OnExportToClipboardCheck,
        this,
        tksIDC_COPY_TO_CLIPBOARD_CTRL
    );

    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation,
        this,
        tksIDC_BROWSE_EXPORT_PATH_CTRL
    );

    pFromDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &QuickExportToCsvDialog::OnFromDateSelection,
        this,
        tksIDC_DATE_FROM_CTRL
    );

    pToDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &QuickExportToCsvDialog::OnToDateSelection,
        this,
        tksIDC_DATE_TO_CTRL
    );

    pExportTodaysTasksOnlyCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &QuickExportToCsvDialog::OnExportTodaysTasksOnlyCheck,
        this,
        tksIDC_EXPORTTODAYSTASKSONLYCHECKBOXCTRL
    );

    pPresetsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &QuickExportToCsvDialog::OnPresetChoiceSelection,
        this,
        tksIDC_PRESET_CHOICE_CTRL
    );

    pOKButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToCsvDialog::OnOK,
        this,
        wxID_OK
    );
}
// clang-format on

void QuickExportToCsvDialog::OnExportToClipboardCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSaveToFileTextCtrl->Disable();
        pBrowseExportPathButton->Disable();
    } else {
        pSaveToFileTextCtrl->Enable();
        pBrowseExportPathButton->Enable();
    }

    bExportToClipboard = event.IsChecked();
}

void QuickExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event)
{
    std::string directoryToOpen = pCfg->GetExportPath();

    auto openDirDialog = new wxDirDialog(this,
        "Select a directory to export the data to",
        directoryToOpen,
        wxDD_DEFAULT_STYLE,
        wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        auto selectedExportPath = openDirDialog->GetPath().ToStdString();
        auto saveToFile = fmt::format(
            "{0}\\taskies-tasks-export-{1}.csv", selectedExportPath, pDateStore->PrintTodayDate);

        pSaveToFileTextCtrl->SetValue(saveToFile);
        pSaveToFileTextCtrl->SetToolTip(saveToFile);
    }

    openDirDialog->Destroy();
}

void QuickExportToCsvDialog::OnFromDateSelection(wxDateEvent& event)
{
    pLogger->info(
        "ExportToCsvDialog::OnFromDateSelection - Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());
    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > mToCtrlDate) {
        SetFromDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot exceed \"to\" date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pFromDatePickerCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newFromDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    pLogger->info("ExportToCsvDialog::OnFromDateSelection - New date value \"{0}\"",
        date::format("%F", newFromDate));

    mFromCtrlDate = eventDateUtc;
    mFromDate = newFromDate;
}

void QuickExportToCsvDialog::OnToDateSelection(wxDateEvent& event)
{
    pLogger->info("ExportToCsvDialog::OnToDateSelection - Received date (wxDateTime) event with "
                  "value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());

    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > mToLatestPossibleDate) {
        SetToDateAndDatePicker();
        return;
    }

    if (eventDateUtc < mFromCtrlDate) {
        SetToDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot go past \"from\" date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pToDatePickerCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newToDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    pLogger->info("ExportToCsvDialog::OnToDateSelection - New date value \"{0}\"",
        date::format("%F", newToDate));

    mToCtrlDate = eventDateUtc;
    mToDate = newToDate;
}

void QuickExportToCsvDialog::OnExportTodaysTasksOnlyCheck(wxCommandEvent& event)
{
    bExportTodaysTasksOnly = event.IsChecked();

    if (bExportTodaysTasksOnly) {
        pFromDatePickerCtrl->SetValue(pDateStore->TodayDateSeconds);
        mFromCtrlDate = pDateStore->TodayDateSeconds;

        pToDatePickerCtrl->SetValue(pDateStore->TodayDateSeconds);
        mToCtrlDate = pDateStore->TodayDateSeconds;

        pFromDatePickerCtrl->Disable();
        pToDatePickerCtrl->Disable();
    } else {
        SetFromAndToDatePickerRanges();

        SetFromDateAndDatePicker();
        SetToDateAndDatePicker();

        pFromDatePickerCtrl->Enable();
        pToDatePickerCtrl->Enable();
    }
}

void QuickExportToCsvDialog::OnPresetChoiceSelection(wxCommandEvent& event)
{
    const std::string TAG = "QuickExportToCsvDialog::OnPresetChoiceSelection";
    pLogger->info("{0} - Begin to apply selected preset", TAG);

    int presetIndex = pPresetsChoiceCtrl->GetSelection();
    ClientData<std::string>* presetData = reinterpret_cast<ClientData<std::string>*>(
        pPresetsChoiceCtrl->GetClientObject(presetIndex));

    if (presetData->GetValue().empty()) {
        return;
    }

    auto presetUuid = presetData->GetValue();

    pLogger->info("{0} - Applying selected preset uuid \"{1}\"", TAG, presetUuid);

    auto presets = pCfg->GetPresets();
    const auto& selectedPresetToApplyIterator = std::find_if(
        presets.begin(), presets.end(), [&](const Core::Configuration::PresetSettings& preset) {
            return preset.Uuid == presetUuid;
        });

    if (selectedPresetToApplyIterator == presets.end()) {
        pLogger->warn("{0} - Could not find preset uuid \"{1}\" in config", TAG, presetUuid);
        return;
    }

    auto& selectedPresetToApply = *selectedPresetToApplyIterator;
    ApplyPreset(selectedPresetToApply);
}

void QuickExportToCsvDialog::OnOK(wxCommandEvent& event)
{
    const std::string TAG = "QuickExportToCsvDialog::OnOK";
    pLogger->info("{0} - Begin export", TAG);

    int presetIndex = pPresetsChoiceCtrl->GetSelection();
    ClientData<std::string>* presetData = reinterpret_cast<ClientData<std::string>*>(
        pPresetsChoiceCtrl->GetClientObject(presetIndex));

    if (presetIndex < 1) {
        wxMessageBox("A preset selection is required for quick export",
            "Preset Required",
            wxICON_WARNING | wxOK_DEFAULT);
        return;
    }

    auto presetUuid = presetData->GetValue();

    pLogger->info("{0} - Get selected preset uuid \"{1}\"", TAG, presetUuid);

    auto presets = pCfg->GetPresets();
    const auto& selectedPresetIterator = std::find_if(
        presets.begin(), presets.end(), [&](const Core::Configuration::PresetSettings& preset) {
            return preset.Uuid == presetUuid;
        });

    auto& selectedPreset = *selectedPresetIterator;

    const auto& columnsToExport = selectedPreset.Columns;
    pLogger->info("{0} - Count of columns to export: \"{1}\"", TAG, columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("No columns to export in selected preset!",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_WARNING);
        return;
    }

    auto columnExportModels = Services::Export::BuildFromPreset(columnsToExport);
    Services::Export::ProjectionBuilder projectionBuilder(pLogger);
    std::vector<Services::Export::Projection> projections =
        projectionBuilder.BuildProjections(columnExportModels);
    std::vector<Services::Export::ColumnJoinProjection> joinProjections =
        projectionBuilder.BuildJoinProjections(columnExportModels);

    const std::string fromDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mFromDate);
    const std::string toDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mToDate);

    pLogger->info("{0} - Export date range: [\"{1}\", \"{2}\"]", TAG, fromDate, toDate);

    std::string exportedData = "";
    bool success = mCsvExporter.Generate(
        mCsvOptions, projections, joinProjections, fromDate, toDate, exportedData);

    if (!success) {
        std::string message = "Failed to export data";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        return;
    }

    if (bExportToClipboard) {
        auto canOpen = wxTheClipboard->Open();
        if (canOpen) {
            auto textData = new wxTextDataObject(exportedData);
            wxTheClipboard->SetData(textData);
            wxTheClipboard->Close();
        }
    } else {
        std::ofstream exportFile;
        exportFile.open(pSaveToFileTextCtrl->GetValue().ToStdString(), std::ios_base::out);
        if (!exportFile) {
            pLogger->error("ExportToCsvDialog::OnExport - Failed to open export file at path {0}",
                pSaveToFileTextCtrl->GetValue().ToStdString());
            return;
        }

        exportFile << exportedData;

        exportFile.close();
    }

    std::string message = bExportToClipboard ? "Successfully exported data to clipboard"
                                             : "Successfully exported data to file";
    wxMessageBox(message, Common::GetProgramName(), wxICON_INFORMATION | wxOK_DEFAULT);

    wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
    NotificationClientData* clientData =
        new NotificationClientData(NotificationType::Information, message);
    addNotificationEvent->SetClientObject(clientData);

    wxQueueEvent(pParent, addNotificationEvent);

    EndModal(wxID_OK);
}

void QuickExportToCsvDialog::SetFromAndToDatePickerRanges()
{
    pFromDatePickerCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateTime fromFromDate = wxDateTime::Now(), toFromDate = wxDateTime::Now();

    if (pFromDatePickerCtrl->GetRange(&fromFromDate, &toFromDate)) {
        pLogger->info("ExportToCsvDialog::SetFromAndToDatePickerRanges - pFromDatePickerCtrl range "
                      "is [{0} - {1}]",
            fromFromDate.FormatISODate().ToStdString(),
            toFromDate.FormatISODate().ToStdString());
    }

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDatePickerCtrl->SetRange(MakeMaximumFromDate(), latestPossibleDatePlusOneDay);

    wxDateTime toFromDate2 = wxDateTime::Now(), toToDate = wxDateTime::Now();

    if (pToDatePickerCtrl->GetRange(&toFromDate2, &toToDate)) {
        pLogger->info("ExportToCsvDialog::SetFromAndToDatePickerRanges - pToDatePickerCtrl range "
                      "is [{0} - {1})",
            toFromDate2.FormatISODate().ToStdString(),
            toToDate.FormatISODate().ToStdString());
    }

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void QuickExportToCsvDialog::SetFromDateAndDatePicker()
{
    pFromDatePickerCtrl->SetValue(pDateStore->MondayDateSeconds);

    pLogger->info("ExportToCsvDialog::SetFromDateAndDatePicker - Reset pFromDatePickerCtrl to: {0}",
        pFromDatePickerCtrl->GetValue().FormatISODate().ToStdString());

    mFromCtrlDate = pDateStore->MondayDateSeconds;

    pLogger->info("ExportToCsvDialog::SetFromDateAndDatePicker - Reset mFromCtrlDate to: {0}",
        mFromCtrlDate.FormatISODate().ToStdString());
}

void QuickExportToCsvDialog::SetToDateAndDatePicker()
{
    pToDatePickerCtrl->SetValue(pDateStore->SundayDateSeconds);

    pLogger->info("ExportToCsvDialog::SetToDateAndDatePicker - Reset pToDatePickerCtrl to: {0}",
        pToDatePickerCtrl->GetValue().FormatISODate().ToStdString());

    mToCtrlDate = pDateStore->SundayDateSeconds;

    pLogger->info("ExportToCsvDialog::SetToDateAndDatePicker - Reset mToCtrlDate to: {0}",
        mToCtrlDate.FormatISODate().ToStdString());
}

void QuickExportToCsvDialog::ApplyPreset(Core::Configuration::PresetSettings& presetSettings)
{
    const std::string TAG = "QuickExportToCsvDialog::ApplyPreset";
    pLogger->info("{0} - Begin to apply selected preset", TAG);

    mCsvOptions.Delimiter = presetSettings.Delimiter;
    mCsvOptions.TextQualifier = presetSettings.TextQualifier;
    mCsvOptions.EmptyValuesHandler = presetSettings.EmptyValuesHandler;
    mCsvOptions.NewLinesHandler = presetSettings.NewLinesHandler;
    mCsvOptions.BooleanHandler = presetSettings.BooleanHandler;

    mCsvOptions.ExcludeHeaders = presetSettings.ExcludeHeaders;
}
} // namespace tks::UI::dlg
