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

#include "quickexporttoformatdlg.h"

#include <fmt/format.h>

#include <wx/clipbrd.h>
#include <wx/dirdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../../common/enums.h"

#include "../../../common/results/exportresult.h"

#include "../../../core/configuration.h"

#include "../../../services/export/availablecolumns.h"
#include "../../../services/export/csvexporterservice.h"
#include "../../../services/export/columnexportmodel.h"
#include "../../../services/export/columnjoinprojection.h"
#include "../../../services/export/excelexporterservice.h"
#include "../../../services/export/projection.h"
#include "../../../services/export/projectionbuilder.h"

#include "../../events.h"
#include "../../common/clientdata.h"

namespace tks::UI::dlg
{
QuickExportToFormatDialog::QuickExportToFormatDialog(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Quick Export",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX,
          name)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pDateStore(nullptr)
    , pExportFormatRadioBoxCtrl(nullptr)
    , pExportToClipboardCheckBoxCtrl(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pLeftDateButton(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , pRightDateButton(nullptr)
    , pDateRangeSelectionRadioBoxCtrl(nullptr)
    , pPresetsChoiceCtrl(nullptr)
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
    , bExportToClipboard(false)
    , mIsExcelInstalled()
    , mRadioExportOptions()
    , mExportOptions()
    , mExportFormat(ExportFormat::Csv)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mRadioExportOptions.Add("Export to CSV");

    std::string excelOption;
    if (mIsExcelInstalled()) {
        excelOption = "Export to Excel";
    } else {
        excelOption = "Export to Excel (unavailable)";
    }
    mRadioExportOptions.Add(excelOption);

    mDateRangeSelectionRadioBoxOptions.Add("Day");
    mDateRangeSelectionRadioBoxOptions.Add("Week");
    mDateRangeSelectionRadioBoxOptions.Add("Month");
    mDateRangeSelectionRadioBoxOptions.Add("Custom");

    Create();
    SetSize(wxSize(FromDIP(640), -1));

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void QuickExportToFormatDialog::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void QuickExportToFormatDialog::CreateControls()
{
    /* Main sizer window */
    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    /* Radio box */
    pExportFormatRadioBoxCtrl = new wxRadioBox(this,
        tksIDC_EXPORTFORMATRADIOBOXCTRL,
        "Export Format",
        wxDefaultPosition,
        wxDefaultSize,
        mRadioExportOptions,
        1,
        wxRA_SPECIFY_ROWS);
    pExportFormatRadioBoxCtrl->SetToolTip("Select an export format option");
    mainSizer->Add(pExportFormatRadioBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Output static box (top) */
    auto outputStaticBox = new wxStaticBox(this, wxID_ANY, "Output");
    auto outputStaticBoxSizer = new wxStaticBoxSizer(outputStaticBox, wxVERTICAL);
    mainSizer->Add(outputStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto outputFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    outputStaticBoxSizer->Add(outputFlexGridSizer, wxSizerFlags().Expand());

    /* Export to clipboard checkbox control */
    pExportToClipboardCheckBoxCtrl =
        new wxCheckBox(outputStaticBox, tksIDC_COPY_TO_CLIPBOARD_CTRL, "Copy to clipboard");
    pExportToClipboardCheckBoxCtrl->SetToolTip("Exported data will be copied to the clipboard");

    /* Save to file text control */
    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVE_TO_FILE_CTRL, wxEmptyString);
    pSaveToFileTextCtrl->SetToolTip("Set directory location to save file to");

    pBrowseExportPathButton =
        new wxButton(outputStaticBox, tksIDC_BROWSE_EXPORT_PATH_CTRL, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set directory location to save file to");

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
    auto dateRangeStaticBoxSizer = new wxStaticBoxSizer(dateRangeStaticBox, wxVERTICAL);
    mainSizer->Add(dateRangeStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* From date controls */
    pLeftDateButton = new wxButton(
        dateRangeStaticBox, tksIDC_LEFTDATEBUTTON, "<", wxDefaultPosition, FromDIP(wxSize(32, -1)));
    pLeftDateButton->SetToolTip("Go backwards based on date selection option");
    pFromDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_FROM_CTRL);
    pFromDatePickerCtrl->SetToolTip("Set the earliest inclusive date to export the data from");

    /* Date static seperator control */
    auto dateStaticSeperator = new wxStaticText(dateRangeStaticBox, wxID_ANY, "-");

    /* To date control */
    pToDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_TO_CTRL);
    pToDatePickerCtrl->SetToolTip("Set the latest inclusive date to export the data from");
    pRightDateButton = new wxButton(dateRangeStaticBox,
        tksIDC_RIGHTDATEBUTTON,
        ">",
        wxDefaultPosition,
        FromDIP(wxSize(32, -1)));
    pRightDateButton->SetToolTip("Go forwards based on date selection option");

    /* Radio Box date range selections */
    pDateRangeSelectionRadioBoxCtrl = new wxRadioBox(dateRangeStaticBox,
        tksIDC_DATERANGESELECTIONRADIOBOXCTRL,
        "Date Selection",
        wxDefaultPosition,
        wxDefaultSize,
        mDateRangeSelectionRadioBoxOptions,
        0,
        wxRA_SPECIFY_COLS);
    pDateRangeSelectionRadioBoxCtrl->SetToolTip("Select an export date range selection");

    /* Date from and to controls horizontal sizer */
    auto dateControlsHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    dateRangeStaticBoxSizer->Add(dateControlsHorizontalSizer, wxSizerFlags().Expand());

    dateControlsHorizontalSizer->Add(
        pLeftDateButton, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateControlsHorizontalSizer->Add(
        pFromDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateControlsHorizontalSizer->Add(dateStaticSeperator, wxSizerFlags().CenterVertical());
    dateControlsHorizontalSizer->Add(
        pToDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateControlsHorizontalSizer->Add(
        pRightDateButton, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());

    dateRangeStaticBoxSizer->Add(
        pDateRangeSelectionRadioBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Presets static box */
    auto presetsStaticBox = new wxStaticBox(this, wxID_ANY, "Presets");
    auto presetsStaticBoxSizer = new wxStaticBoxSizer(presetsStaticBox, wxHORIZONTAL);
    mainSizer->Add(presetsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    auto presetsChoiceLabel = new wxStaticText(presetsStaticBox, wxID_ANY, "Preset");
    pPresetsChoiceCtrl = new wxChoice(presetsStaticBox, tksIDC_PRESET_CHOICE_CTRL);
    pPresetsChoiceCtrl->SetToolTip("Select an existing export preset to apply (if any)");

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

void QuickExportToFormatDialog::FillControls()
{
    pExportFormatRadioBoxCtrl->SetSelection(0);
    int excelRadioButtonIndex = static_cast<int>(ExportFormat::Excel) - 1;
    if (!mIsExcelInstalled()) {
        pExportFormatRadioBoxCtrl->Enable(excelRadioButtonIndex, false);
    }

    /* Export File Controls */
    auto saveToFile = fmt::format("{0}\\taskies-export-{1}.csv",
        pCfg->GetExportPath(),
        pDateStore->FormatDate(pDateStore->TodayDate));
    pSaveToFileTextCtrl->ChangeValue(saveToFile);
    pSaveToFileTextCtrl->SetToolTip(saveToFile);

    /* Date Controls */
    SetDateControlsValue();

    pDateRangeSelectionRadioBoxCtrl->SetSelection(0);
    mDateRangeOption =
        static_cast<ExportDateRangeOption>(pDateRangeSelectionRadioBoxCtrl->GetSelection() + 1);

    /* Presets controls */
    pPresetsChoiceCtrl->Append("(none)", new ClientData<std::string>(""));
    pPresetsChoiceCtrl->SetSelection(0);

    const auto& presets = pCfg->GetPresets();
    int presetIndexToSet = 0;
    for (size_t i = 0; i < presets.size(); i++) {
        pPresetsChoiceCtrl->Append(presets[i].Name, new ClientData<std::string>(presets[i].Uuid));

        if (presets[i].IsDefault) {
            presetIndexToSet = i + 1;
            ApplyPreset(presets[i]);
        }
    }

    pPresetsChoiceCtrl->SetSelection(presetIndexToSet);
}

// clang-format off
void QuickExportToFormatDialog::ConfigureEventBindings()
{
    pExportFormatRadioBoxCtrl->Bind(
        wxEVT_RADIOBOX,
        &QuickExportToFormatDialog::OnExportFormatRadioButtonClick,
        this,
        tksIDC_EXPORTFORMATRADIOBOXCTRL
    );

    pExportToClipboardCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &QuickExportToFormatDialog::OnExportToClipboardCheck,
        this,
        tksIDC_COPY_TO_CLIPBOARD_CTRL
    );

    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToFormatDialog::OnOpenDirectoryForSaveToFileLocation,
        this,
        tksIDC_BROWSE_EXPORT_PATH_CTRL
    );

    pLeftDateButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToFormatDialog::OnLeftDateButtonClick,
        this
    );

    pFromDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &QuickExportToFormatDialog::OnFromDateSelection,
        this,
        tksIDC_DATE_FROM_CTRL
    );

    pToDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &QuickExportToFormatDialog::OnToDateSelection,
        this,
        tksIDC_DATE_TO_CTRL
    );

    pRightDateButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToFormatDialog::OnRightDateButtonClick,
        this
    );

    pDateRangeSelectionRadioBoxCtrl->Bind(
        wxEVT_RADIOBOX,
        &QuickExportToFormatDialog::OnDateRangeRadioBoxSelection,
        this
    );

    pPresetsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &QuickExportToFormatDialog::OnPresetChoiceSelection,
        this,
        tksIDC_PRESET_CHOICE_CTRL
    );

    pOKButton->Bind(
        wxEVT_BUTTON,
        &QuickExportToFormatDialog::OnOK,
        this,
        wxID_OK
    );
}
// clang-format on

void QuickExportToFormatDialog::OnExportFormatRadioButtonClick(wxCommandEvent& event)
{
    int selection = event.GetSelection();

    mExportFormat = static_cast<ExportFormat>(selection + 1);
    SPDLOG_LOGGER_TRACE(pLogger,
        "Export format selected: \"{0}\"",
        mExportFormat == ExportFormat::Csv ? "CSV" : "XLSX");

    std::string fileExtension = mExportFormat == ExportFormat::Csv ? "csv" : "xlsx";
    auto saveToFile = fmt::format("{0}\\taskies-export-{1}.{2}",
        pCfg->GetExportPath(),
        pDateStore->FormatDate(pDateStore->TodayDate),
        fileExtension);
    pSaveToFileTextCtrl->ChangeValue(saveToFile);
    pSaveToFileTextCtrl->SetToolTip(saveToFile);

    if (mExportFormat == ExportFormat::Excel) {
        bExportToClipboard = false;
        pExportToClipboardCheckBoxCtrl->Disable();
    } else {
        pExportToClipboardCheckBoxCtrl->Enable();
    }
}

void QuickExportToFormatDialog::OnExportToClipboardCheck(wxCommandEvent& event)
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

void QuickExportToFormatDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event)
{
    auto openDirDialog = new wxDirDialog(this,
        "Select a directory to export the data to",
        pCfg->GetExportPath(),
        wxDD_DEFAULT_STYLE,
        wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        std::string fileExtension = mExportFormat == ExportFormat::Csv ? "csv" : "xlsx";
        std::string selectedExportPath = openDirDialog->GetPath().ToStdString();
        std::string saveToFile = fmt::format("{0}\\taskies-export-{1}.{2}",
            selectedExportPath,
            pDateStore->FormatDate(pDateStore->TodayDate),
            fileExtension);

        pSaveToFileTextCtrl->SetValue(saveToFile);
        pSaveToFileTextCtrl->SetToolTip(saveToFile);
    }

    openDirDialog->Destroy();
}

void QuickExportToFormatDialog::OnLeftDateButtonClick(wxCommandEvent& event)
{
    switch (mDateRangeOption) {
    case ExportDateRangeOption::Day: {
        auto newDate = mFromDate - date::days{ 1 };

        SetDateControlsValue(newDate);
        break;
    }
    case ExportDateRangeOption::Week: {
        auto newFromDate = mFromDate - date::weeks{ 1 };
        auto newToDate = mToDate - date::weeks{ 1 };

        SetDateControlsValue(newFromDate, newToDate);
        break;
    }
    case ExportDateRangeOption::Month: {
        date::year_month_day ymdFromDate = mFromDate;
        date::year_month_day ymdToDate = mToDate;

        auto newFromDate = ymdFromDate - date::months{ 1 };
        auto newToDate = ymdToDate - date::months{ 1 };

        newFromDate = newFromDate.year() / newFromDate.month() / 1;
        newToDate = newToDate.year() / newToDate.month() / date::last;

        SetDateControlsValue(date::sys_days(newFromDate), date::sys_days(newToDate));
        break;
    }
    case ExportDateRangeOption::Custom:
    default:
        break;
    }
}

void QuickExportToFormatDialog::OnFromDateSelection(wxDateEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    wxDateTime eventDate = wxDateTime(event.GetDate());
    wxDateTime eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > pToDatePickerCtrl->GetValue()) {
        SetFromDateControlValue(false);
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot go past \"to\" date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pFromDatePickerCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newFromDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    SPDLOG_LOGGER_TRACE(pLogger, "New from date value \"{0}\"", date::format("%F", newFromDate));

    mFromDate = newFromDate;
}

void QuickExportToFormatDialog::OnToDateSelection(wxDateEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Received date (wxDateTime) event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    wxDateTime eventDate = wxDateTime(event.GetDate());
    wxDateTime eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc < pFromDatePickerCtrl->GetValue()) {
        SetToDateControlValue(false);
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot go past \"from\" date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pToDatePickerCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newToDate =
        date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    SPDLOG_LOGGER_TRACE(pLogger, "New to date value \"{0}\"", date::format("%F", newToDate));

    mToDate = newToDate;
}

void QuickExportToFormatDialog::OnRightDateButtonClick(wxCommandEvent& event)
{
    switch (mDateRangeOption) {
    case ExportDateRangeOption::Day: {
        auto newDate = mToDate + date::days{ 1 };

        SetDateControlsValue(newDate);
        break;
    }
    case ExportDateRangeOption::Week: {
        auto newFromDate = mFromDate + date::weeks{ 1 };
        auto newToDate = mToDate + date::weeks{ 1 };

        SetDateControlsValue(newFromDate, newToDate);
        break;
    }
    case ExportDateRangeOption::Month: {
        date::year_month_day ymdFromDate = mFromDate;
        date::year_month_day ymdToDate = mToDate;

        auto newFromDate = ymdFromDate + date::months{ 1 };
        auto newToDate = ymdToDate + date::months{ 1 };

        newFromDate = newFromDate.year() / newFromDate.month() / 1;
        newToDate = newToDate.year() / newToDate.month() / date::last;

        SetDateControlsValue(date::sys_days(newFromDate), date::sys_days(newToDate));
        break;
    }
    case ExportDateRangeOption::Custom:
    default:
        break;
    }
}

void QuickExportToFormatDialog::OnDateRangeRadioBoxSelection(wxCommandEvent& event)
{
    mDateRangeOption = static_cast<ExportDateRangeOption>(event.GetSelection() + 1);

    SPDLOG_LOGGER_TRACE(pLogger, "Radio box selection changed \"{0}\"", event.GetSelection());

    switch (mDateRangeOption) {
    case ExportDateRangeOption::Day: {
        SetDateControlsValue(pDateStore->TodayDate);
        break;
    }
    case ExportDateRangeOption::Week: {
        SetDateControlsValue(pDateStore->MondayDate, pDateStore->SundayDate);
        break;
    }
    case ExportDateRangeOption::Month: {
        SetDateControlsValue(pDateStore->FirstDayOfMonth, pDateStore->LastDayOfMonth);
        break;
    }
    case ExportDateRangeOption::Custom: {
        SetDateControlsCustomValue(pDateStore->TodayDate, pDateStore->TodayDate);
        break;
    }
    default:
        break;
    }
}

void QuickExportToFormatDialog::OnPresetChoiceSelection(wxCommandEvent& event)
{
    int presetIndex = pPresetsChoiceCtrl->GetSelection();
    ClientData<std::string>* presetData = reinterpret_cast<ClientData<std::string>*>(
        pPresetsChoiceCtrl->GetClientObject(presetIndex));

    if (presetData->GetValue().empty()) {
        return;
    }

    auto presetUuid = presetData->GetValue();

    SPDLOG_LOGGER_TRACE(pLogger, "Applying selected preset uuid \"{0}\"", presetUuid);

    auto presets = pCfg->GetPresets();
    const auto& selectedPresetToApplyIterator = std::find_if(presets.begin(),
        presets.end(),
        [&](const Core::Settings::PresetSetting& preset) { return preset.Uuid == presetUuid; });

    if (selectedPresetToApplyIterator == presets.end()) {
        pLogger->warn("Could not find preset uuid \"{0}\" in config", presetUuid);
        return;
    }

    auto& selectedPresetToApply = *selectedPresetToApplyIterator;
    ApplyPreset(selectedPresetToApply);
}

void QuickExportToFormatDialog::OnOK(wxCommandEvent& event)
{
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

    SPDLOG_LOGGER_TRACE(pLogger, "Get selected preset uuid \"{0}\"", presetUuid);

    auto presets = pCfg->GetPresets();
    const auto& selectedPresetIterator = std::find_if(presets.begin(),
        presets.end(),
        [&](const Core::Settings::PresetSetting& preset) { return preset.Uuid == presetUuid; });

    auto& selectedPreset = *selectedPresetIterator;

    const auto& columnsToExport = selectedPreset.Columns;
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("No columns to export in selected preset",
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

    const std::string fromDate = pDateStore->FormatDate(mFromDate);
    const std::string toDate = pDateStore->FormatDate(mToDate);

    SPDLOG_LOGGER_TRACE(pLogger, "Export date range: [\"{0}\", \"{1}\"]", fromDate, toDate);

    ExportResult result{};
    std::string message = "";

    wxBusyCursor busy;

    {
        if (mExportFormat == ExportFormat::Csv) {
            Services::Export::CsvExporterService csvExporter(
                pLogger, mExportOptions, mDatabaseFilePath, false);

            std::string exportedData = "";
            result = csvExporter.ExportToCsv(
                projections, joinProjections, fromDate, toDate, exportedData);

            if (result.Success) {
                if (bExportToClipboard) {
                    auto canOpen = wxTheClipboard->Open();
                    if (canOpen) {
                        auto textData = new wxTextDataObject(exportedData);
                        wxTheClipboard->SetData(textData);
                        wxTheClipboard->Close();
                    }
                } else {
                    std::ofstream exportFile;
                    exportFile.open(
                        pSaveToFileTextCtrl->GetValue().ToStdString(), std::ios_base::out);
                    if (!exportFile.is_open()) {
                        pLogger->error("Failed to open export file at path \"{0}\"",
                            pSaveToFileTextCtrl->GetValue().ToStdString());
                        return;
                    }

                    exportFile << exportedData;

                    exportFile.close();
                }
                message = bExportToClipboard ? "Successfully exported data to clipboard"
                                             : "Successfully exported data to file";
            }
        } else if (mExportFormat == ExportFormat::Excel) {
            Services::Export::ExcelExporterService excelExporterService(pLogger,
                mDatabaseFilePath,
                mExportOptions.IncludeAttributes,
                mExportOptions.NewLinesHandler,
                mExportOptions.BooleanHandler);

            const std::string& saveLocation = pSaveToFileTextCtrl->GetValue().ToStdString();
            result = excelExporterService.ExportToExcel(
                projections, joinProjections, fromDate, toDate, saveLocation);

            message = "Successfully exported data to Excel";
        } else {
            pLogger->error("Unmatched or invalid enum value of ExportFormat");
            result = ExportResult::Fail("Invalid export format selected");
        }
    }

    if (!result.Success) {
        wxMessageBox(result.ErrorMessage, Common::GetProgramName(), wxICON_ERROR | wxOK_DEFAULT);

        return;
    }

    wxMessageBox(message, Common::GetProgramName(), wxICON_INFORMATION | wxOK_DEFAULT);

    EndModal(wxID_OK);
}

void QuickExportToFormatDialog::SetDateControlsValue()
{
    SetFromDateControlValue();
    SetToDateControlValue();
}

void QuickExportToFormatDialog::SetFromDateControlValue(bool disable)
{
    mFromDate = pDateStore->TodayDate;

    pFromDatePickerCtrl->SetValue(pDateStore->TodayDateSeconds);
    if (disable) {
        pFromDatePickerCtrl->Disable();
    }
}

void QuickExportToFormatDialog::SetToDateControlValue(bool disable)
{
    mToDate = pDateStore->TodayDate;

    pToDatePickerCtrl->SetValue(pDateStore->TodayDateSeconds);
    if (disable) {
        pToDatePickerCtrl->Disable();
    }
}

void QuickExportToFormatDialog::SetDateControlsValue(const date::sys_days& newDate)
{
    SetFromDateControlValue(newDate);
    SetToDateControlValue(newDate);

    pLeftDateButton->Enable();
    pRightDateButton->Enable();
}

void QuickExportToFormatDialog::SetFromDateControlValue(const date::sys_days& newDate)
{
    mFromDate = newDate;

    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(newDate.time_since_epoch()).count();
    pFromDatePickerCtrl->SetValue(seconds);

    if (pFromDatePickerCtrl->IsEnabled()) {
        pFromDatePickerCtrl->Disable();
    }
}

void QuickExportToFormatDialog::SetDateControlsValue(const date::sys_days& fromDate,
    const date::sys_days& toDate)
{
    SetFromDateControlValue(fromDate);
    SetToDateControlValue(toDate);

    pLeftDateButton->Enable();
    pRightDateButton->Enable();
}

void QuickExportToFormatDialog::SetToDateControlValue(const date::sys_days& newDate)
{
    mToDate = newDate;

    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(newDate.time_since_epoch()).count();
    pToDatePickerCtrl->SetValue(seconds);

    if (pToDatePickerCtrl->IsEnabled()) {
        pToDatePickerCtrl->Disable();
    }
}

void QuickExportToFormatDialog::SetDateControlsCustomValue(const date::sys_days& fromDate,
    const date::sys_days& toDate)
{
    SetFromDateControlCustomValue(fromDate);
    SetToDateControlCustomValue(toDate);

    pLeftDateButton->Disable();
    pRightDateButton->Disable();
}

void QuickExportToFormatDialog::SetFromDateControlCustomValue(const date::sys_days& newDate)
{
    mFromDate = newDate;

    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(newDate.time_since_epoch()).count();
    pFromDatePickerCtrl->SetValue(seconds);

    if (!pFromDatePickerCtrl->IsEnabled()) {
        pFromDatePickerCtrl->Enable();
    }
}

void QuickExportToFormatDialog::SetToDateControlCustomValue(const date::sys_days& newDate)
{
    mToDate = newDate;

    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(newDate.time_since_epoch()).count();
    pToDatePickerCtrl->SetValue(seconds);

    if (!pToDatePickerCtrl->IsEnabled()) {
        pToDatePickerCtrl->Enable();
    }
}

void QuickExportToFormatDialog::ApplyPreset(const Core::Settings::PresetSetting& presetSetting)
{
    mExportOptions.Delimiter = presetSetting.Delimiter;
    mExportOptions.TextQualifier = presetSetting.TextQualifier;
    mExportOptions.EmptyValuesHandler = presetSetting.EmptyValuesHandler;
    mExportOptions.NewLinesHandler = presetSetting.NewLinesHandler;
    mExportOptions.BooleanHandler = presetSetting.BooleanHandler;

    mExportOptions.ExcludeHeaders = presetSetting.ExcludeHeaders;
    mExportOptions.IncludeAttributes = presetSetting.IncludeAttributes;
}
} // namespace tks::UI::dlg
