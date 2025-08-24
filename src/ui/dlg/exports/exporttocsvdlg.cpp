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

#include "exporttocsvdlg.h"

#include <algorithm>

#include <date/date.h>

#include <fmt/format.h>

#include <wx/clipbrd.h>
#include <wx/dirdlg.h>
#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/enums.h"

#include "../../../services/export/availablecolumns.h"
#include "../../../services/export/csvexporterservice.h"
#include "../../../services/export/columnexportmodel.h"
#include "../../../services/export/columnjoinprojection.h"
#include "../../../services/export/projection.h"
#include "../../../services/export/projectionbuilder.h"

#include "../../../utils/utils.h"

#include "../../events.h"
#include "../../common/clientdata.h"
#include "../../common/notificationclientdata.h"

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
ExportToCsvDialog::ExportToCsvDialog(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Export to CSV",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pDateStore(nullptr)
    , pDelimiterChoiceCtrl(nullptr)
    , pTextQualifierChoiceCtrl(nullptr)
    , pEmptyValueHandlerChoiceCtrl(nullptr)
    , pNewLinesHandlerChoiceCtrl(nullptr)
    , pBooleanHanderChoiceCtrl(nullptr)
    , pExportToClipboardCheckBoxCtrl(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , pPresetNameTextCtrl(nullptr)
    , pPresetIsDefaultCheckBoxCtrl(nullptr)
    , pPresetSaveButton(nullptr)
    , pPresetResetButton(nullptr)
    , pPresetsChoiceCtrl(nullptr)
    , pAvailableColumnsListView(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pDataViewCtrl(nullptr)
    , pUpButton(nullptr)
    , pDownButton(nullptr)
    , pExcludeHeadersCheckBoxCtrl(nullptr)
    , pIncludeAttributesCheckBoxCtrl(nullptr)
    , pDataExportPreviewTextCtrl(nullptr)
    , pShowPreviewButton(nullptr)
    , pExportButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
    , mExportOptions()
    , bExportToClipboard(false)
    , bOpenExplorerInExportDirectory(false)
    , bExportTodaysTasksOnly(false)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    Create();
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(500, 700)));
    }

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}

void ExportToCsvDialog::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void ExportToCsvDialog::CreateControls()
{
    /* Main Window Sizer */
    auto sizer = new wxBoxSizer(wxVERTICAL);

    auto outputAndPresetHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(outputAndPresetHorizontalSizer, wxSizerFlags().Expand());

    /* Output static box (top) */
    auto outputStaticBox = new wxStaticBox(this, wxID_ANY, "Output");
    auto outputStaticBoxSizer = new wxStaticBoxSizer(outputStaticBox, wxVERTICAL);
    outputAndPresetHorizontalSizer->Add(
        outputStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Export to clipboard checkbox control */
    pExportToClipboardCheckBoxCtrl =
        new wxCheckBox(outputStaticBox, tksIDC_COPY_TO_CLIPBOARD_CTRL, "Copy to clipboard");
    pExportToClipboardCheckBoxCtrl->SetToolTip("Exported data will be copied to the clipboard");

    /* Save to file text control */
    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVE_TO_FILE_CTRL, wxEmptyString);

    pBrowseExportPathButton =
        new wxButton(outputStaticBox, tksIDC_BROWSE_EXPORT_PATH_CTRL, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set the directory to save the exported data to");

    /* Close dialog after export check box control */
    pCloseDialogAfterExportingCheckBoxCtrl = new wxCheckBox(
        outputStaticBox, tksIDC_CLOSE_DIALOG_AFTER_EXPORT_CTRL, "Close dialog after exporting");
    pCloseDialogAfterExportingCheckBoxCtrl->SetToolTip(
        "The dialog will close automatically after a successful export");

    /* Open explorer in export directory check box control */
    pOpenExplorerInExportDirectoryCheckBoxCtrl = new wxCheckBox(outputStaticBox,
        TKSIDC_OPENEXPLORERINEXPORTDIRECTORYCHECKBOXCTRL,
        "Open File Explorer after exporting");
    pOpenExplorerInExportDirectoryCheckBoxCtrl->SetToolTip(
        "Open Explorer in export directory after successful export");

    auto outputFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    outputStaticBoxSizer->Add(outputFlexGridSizer, wxSizerFlags().Expand());

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
    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(
        pCloseDialogAfterExportingCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(2)));
    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(
        pOpenExplorerInExportDirectoryCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(2)));

    /* Presets static box */
    auto presetsStaticBox = new wxStaticBox(this, wxID_ANY, "Presets");
    auto presetsStaticBoxSizer = new wxStaticBoxSizer(presetsStaticBox, wxVERTICAL);
    outputAndPresetHorizontalSizer->Add(
        presetsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Preset name static text control */
    auto presetNameLabel = new wxStaticText(presetsStaticBox, wxID_ANY, "Name");

    /* Preset name text control */
    pPresetNameTextCtrl = new wxTextCtrl(presetsStaticBox, tksIDC_PRESET_NAME_TEXT_CTRL, "");
    pPresetNameTextCtrl->SetHint("Preset name");
    pPresetNameTextCtrl->SetToolTip("Name of the preset");

    /* Preset is default checkbox control */
    pPresetIsDefaultCheckBoxCtrl =
        new wxCheckBox(presetsStaticBox, tksIDC_PRESET_IS_DEFAULT_CTRL, "Is Default");
    pPresetIsDefaultCheckBoxCtrl->SetToolTip(
        "A default preset will be selected and applied automatically");

    pPresetSaveButton = new wxButton(presetsStaticBox, tksIDC_PRESET_SAVE_BUTTON, "Save");
    pPresetSaveButton->SetToolTip("Create new or update existing preset");

    pPresetResetButton = new wxButton(presetsStaticBox, tksIDC_PRESET_RESET_BUTTON, "Reset");
    pPresetResetButton->SetToolTip("Reset all options to their defaults");

    /* Presets selection controls */
    auto presetsChoiceLabel = new wxStaticText(presetsStaticBox, wxID_ANY, "Preset");
    pPresetsChoiceCtrl = new wxChoice(presetsStaticBox, tksIDC_PRESET_CHOICE_CTRL);

    /* Presets flex grid sizer */
    auto presetFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    presetsStaticBoxSizer->Add(presetFlexGridSizer, wxSizerFlags().Expand());

    presetFlexGridSizer->AddGrowableCol(1, 1);

    presetFlexGridSizer->Add(
        presetNameLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    presetFlexGridSizer->Add(
        pPresetNameTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    presetFlexGridSizer->Add(0, 0);
    presetFlexGridSizer->Add(
        pPresetIsDefaultCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    presetFlexGridSizer->Add(0, 0);

    auto presetButtonHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    presetButtonHorizontalSizer->AddStretchSpacer(1);
    presetButtonHorizontalSizer->Add(pPresetSaveButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    presetButtonHorizontalSizer->Add(pPresetResetButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    presetFlexGridSizer->Add(presetButtonHorizontalSizer, wxSizerFlags().Expand());

    presetFlexGridSizer->Add(
        presetsChoiceLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    presetFlexGridSizer->Add(
        pPresetsChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Horizontal Line */
    auto line0 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line0, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(4)).Expand());

    /* Horizontal sizer for options and date range controls */
    auto optionsAndDateRangeHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(optionsAndDateRangeHorizontalSizer, wxSizerFlags().Expand());

    /* Options static box */
    auto optionsStaticBox = new wxStaticBox(this, wxID_ANY, "Options");
    auto optionsStaticBoxSizer = new wxStaticBoxSizer(optionsStaticBox, wxVERTICAL);
    optionsAndDateRangeHorizontalSizer->Add(
        optionsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Flex grid sizer for option choices */
    auto optionsFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    optionsStaticBoxSizer->Add(optionsFlexGridSizer, wxSizerFlags().Expand().Proportion(1));

    optionsFlexGridSizer->AddGrowableCol(1, 1);

    /* Delimiter choice control */
    auto delimiterLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Delimiter");
    pDelimiterChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_DELIMITER_CTRL);
    pDelimiterChoiceCtrl->SetToolTip("Set the field separator character");

    /* Text qualifiers choice control */
    auto textQualifierLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Text Qualifier");
    pTextQualifierChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_TEXT_QUALIFIER_CTRL);
    pTextQualifierChoiceCtrl->SetToolTip("Set the text qualifier for field values");

    /* Empty values choice control */
    auto emptyValuesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Empty Values");
    pEmptyValueHandlerChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_EMPTY_VALUE_HANDLER_CTRL);
    pEmptyValueHandlerChoiceCtrl->SetToolTip("Set how to handle empty or blank field values");

    /* New lines choice control */
    auto newLinesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "New Lines");
    pNewLinesHandlerChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_NEW_LINES_HANDLER_CTRL);
    pNewLinesHandlerChoiceCtrl->SetToolTip("Set how to handle multiline field values");

    /* Boolean handler control */
    auto booleanHandlerLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Booleans");
    pBooleanHanderChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_BOOLEAN_HANDLER_CTRL);
    pBooleanHanderChoiceCtrl->SetToolTip("Set how to handle boolean field values");

    optionsFlexGridSizer->Add(
        delimiterLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(
        pDelimiterChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    optionsFlexGridSizer->Add(
        textQualifierLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(
        pTextQualifierChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    optionsFlexGridSizer->Add(
        emptyValuesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(
        pEmptyValueHandlerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    optionsFlexGridSizer->Add(
        newLinesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(
        pNewLinesHandlerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    optionsFlexGridSizer->Add(
        booleanHandlerLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(
        pBooleanHanderChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Date range static box */
    auto dateRangeStaticBox = new wxStaticBox(this, wxID_ANY, "Date Range");
    auto dateRangeStaticBoxSizer = new wxStaticBoxSizer(dateRangeStaticBox, wxVERTICAL);
    optionsAndDateRangeHorizontalSizer->Add(
        dateRangeStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* From date control */
    auto fromDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "From: ");
    pFromDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_FROM_CTRL);
    pFromDatePickerCtrl->SetToolTip("Set the earliest inclusive date to export the data from");

    /* To date control */
    auto toDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "To: ");
    pToDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_TO_CTRL);
    pToDatePickerCtrl->SetToolTip("Set the latest inclusive date to export the data from");

    /* Export todays tasks check box control */
    pExportTodaysTasksCheckBoxCtrl = new wxCheckBox(
        dateRangeStaticBox, tksIDC_EXPORTTODAYSTASKSCHECKBOXCTRL, "Export today's tasks");
    pExportTodaysTasksCheckBoxCtrl->SetToolTip("Export tasks logged during today's date");

    /* Set date range to work week (i.e. Mon - Fri) */
    pWorkWeekRangeCheckBoxCtrl = new wxCheckBox(
        dateRangeStaticBox, tksIDC_WORKWEEKRANGECHECKBOXCTRL, "Export work week tasks");
    pWorkWeekRangeCheckBoxCtrl->SetToolTip("Export only tasks logged during a work week");

    /* Date from and to controls horizontal sizer */
    auto dateControlsHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    dateRangeStaticBoxSizer->Add(dateControlsHorizontalSizer, wxSizerFlags().Expand());

    dateControlsHorizontalSizer->Add(
        fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateControlsHorizontalSizer->Add(pFromDatePickerCtrl,
        wxSizerFlags() /*.Border(wxLEFT, FromDIP(1))*/.Border(
            wxTOP | wxRIGHT | wxBOTTOM, FromDIP(4)));
    dateControlsHorizontalSizer->Add(
        toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateControlsHorizontalSizer->Add(pToDatePickerCtrl,
        wxSizerFlags() /*.Border(wxLEFT, FromDIP(1))*/.Border(
            wxTOP | wxRIGHT | wxBOTTOM, FromDIP(4)));

    dateRangeStaticBoxSizer->Add(
        pExportTodaysTasksCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(
        pWorkWeekRangeCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Header/Columns to Export Controls sizer */
    auto dataToExportStaticBox = new wxStaticBox(this, wxID_ANY, "Data to Export");
    auto dataToExportStaticBoxSizer = new wxStaticBoxSizer(dataToExportStaticBox, wxVERTICAL);
    sizer->Add(dataToExportStaticBoxSizer,
        wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    auto headerControlsHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    dataToExportStaticBoxSizer->Add(
        headerControlsHorizontalSizer, wxSizerFlags().Expand().Proportion(1));

    /* Default headers list view controls */
    pAvailableColumnsListView = new wxListView(dataToExportStaticBox,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pAvailableColumnsListView->EnableCheckBoxes();
    pAvailableColumnsListView->SetToolTip("Available headers that can be exported");
    headerControlsHorizontalSizer->Add(
        pAvailableColumnsListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    int columnIndex = 0;

    wxListItem availableColumn;
    availableColumn.SetId(columnIndex);
    availableColumn.SetText("Available Headers");
    availableColumn.SetWidth(180);
    pAvailableColumnsListView->InsertColumn(columnIndex++, availableColumn);

    /* Chevrons buttons */
    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    headerControlsHorizontalSizer->Add(chevronButtonSizer, wxSizerFlags());

    pRightChevronButton = new wxButton(
        dataToExportStaticBox, tksIDC_RIGHT_CHEV_CTRL, ">", wxDefaultPosition, wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a header to include in the export");
    pLeftChevronButton = new wxButton(
        dataToExportStaticBox, tksIDC_LEFT_CHEV_CTRL, "<", wxDefaultPosition, wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a header to exclude in the export");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Export Headers data view list control */
    pDataViewCtrl = new wxDataViewCtrl(dataToExportStaticBox,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES);
    pDataViewCtrl->SetToolTip("Headers to export to a file or clipboard");
    headerControlsHorizontalSizer->Add(
        pDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Model */
    pExportColumnListModel = new ColumnListModel(pLogger);
    pDataViewCtrl->AssociateModel(pExportColumnListModel.get());

    /* Toggled Column */
    pDataViewCtrl->AppendToggleColumn(
        "", ColumnListModel::Col_Toggled, wxDATAVIEW_CELL_ACTIVATABLE);

    /* Header Column */
    auto* textRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);
    wxDataViewColumn* headerEditableColumn = new wxDataViewColumn("Headers",
        textRenderer,
        ColumnListModel::Col_Column,
        wxCOL_WIDTH_AUTOSIZE,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    headerEditableColumn->SetMinWidth(120);
    pDataViewCtrl->AppendColumn(headerEditableColumn);

    /* OrderIndex Column */
    auto* orderRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);
    auto* orderColumn = new wxDataViewColumn("Order",
        orderRenderer,
        ColumnListModel::Col_Order,
        FromDIP(32),
        wxALIGN_CENTER,
        wxDATAVIEW_COL_HIDDEN | wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE);
    orderColumn->SetSortOrder(true /* ascending */);
    pDataViewCtrl->AppendColumn(orderColumn);

    /* Up|Down Buttons sizer */
    auto upDownButtonSizer = new wxBoxSizer(wxVERTICAL);
    headerControlsHorizontalSizer->Add(upDownButtonSizer, wxSizerFlags());

    /* Up|Down Buttons */
    pUpButton = new wxButton(dataToExportStaticBox, tksIDC_UP_BUTTON, "Up");
    pUpButton->SetToolTip("Move the selected header up");
    pDownButton = new wxButton(dataToExportStaticBox, tksIDC_DOWN_BUTTON, "Down");
    pDownButton->SetToolTip("Move the selected header down");

    upDownButtonSizer->Add(pUpButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    upDownButtonSizer->Add(pDownButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Export checkbox options */
    pExcludeHeadersCheckBoxCtrl =
        new wxCheckBox(dataToExportStaticBox, tksIDC_EXCLUDE_HEADERS_CTRL, "Exclude Headers");
    pExcludeHeadersCheckBoxCtrl->SetToolTip("Headers are excluded from the CSV export");
    dataToExportStaticBoxSizer->Add(
        pExcludeHeadersCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    pIncludeAttributesCheckBoxCtrl = new wxCheckBox(
        dataToExportStaticBox, tksIDC_INCLUDEATTRIBUTESCHECKBOXCTRL, "Include Attributes");
    pIncludeAttributesCheckBoxCtrl->SetToolTip("Include task attributes in the CSV export");
    dataToExportStaticBoxSizer->Add(
        pIncludeAttributesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Data Preview sizer and controls */
    auto dataPreviewStaticBox = new wxStaticBox(this, wxID_ANY, "Preview");
    auto dataPreviewStaticBoxSizer = new wxStaticBoxSizer(dataPreviewStaticBox, wxVERTICAL);
    sizer->Add(dataPreviewStaticBoxSizer, wxSizerFlags().Expand().Border(wxALL, FromDIP(4)));

    /* Data export preview text control */
    pDataExportPreviewTextCtrl = new wxTextCtrl(dataPreviewStaticBox,
        tksIDC_DATA_EXPORT_PREVIEW_CTRL,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_READONLY | wxTE_MULTILINE);
    dataPreviewStaticBoxSizer->Add(
        pDataExportPreviewTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pShowPreviewButton =
        new wxButton(dataPreviewStaticBox, tksIDC_SHOW_PREVIEW_BUTTON, "Show Preview");
    pShowPreviewButton->SetToolTip("Show a preview of the data to be exported");

    dataPreviewStaticBoxSizer->Add(
        pShowPreviewButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    /* Horizontal Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line1, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(4)).Expand());

    /* Export|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pExportButton = new wxButton(this, tksIDC_EXPORT_BUTTON, "Export");
    pExportButton->SetDefault();
    pExportButton->SetFocus();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Close");

    buttonsSizer->Add(pExportButton, wxSizerFlags().Border(wxALL, FromDIP(2)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(2)));

    SetSizerAndFit(sizer);
}

void ExportToCsvDialog::FillControls()
{
    /* Export File Controls */
    auto saveToFile = fmt::format(
        "{0}\\taskies-export-{1}.csv", pCfg->GetExportPath(), pDateStore->PrintTodayDate);
    pSaveToFileTextCtrl->ChangeValue(saveToFile);
    pSaveToFileTextCtrl->SetToolTip(saveToFile);

    pDelimiterChoiceCtrl->Append("Please select", new ClientData<int>(-1));
    pDelimiterChoiceCtrl->SetSelection(0);

    auto delimiters = Common::Static::DelimiterList();
    for (auto i = 0; i < delimiters.size(); i++) {
        pDelimiterChoiceCtrl->Append(
            delimiters[i].first, new ClientData<DelimiterType>(delimiters[i].second));
    }

    pTextQualifierChoiceCtrl->Append("Please select", new ClientData<int>(-1));
    pTextQualifierChoiceCtrl->SetSelection(0);

    auto textQualifiers = Common::Static::TextQualifierList();
    for (auto i = 0; i < textQualifiers.size(); i++) {
        pTextQualifierChoiceCtrl->Append(
            textQualifiers[i].first, new ClientData<TextQualifierType>(textQualifiers[i].second));
    }

    pEmptyValueHandlerChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pEmptyValueHandlerChoiceCtrl->SetSelection(0);

    auto emptyValueHandlers = Common::Static::EmptyValueHandlerList();
    for (auto i = 0; i < emptyValueHandlers.size(); i++) {
        pEmptyValueHandlerChoiceCtrl->Append(emptyValueHandlers[i], new ClientData<int>(i + 1));
    }

    pNewLinesHandlerChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pNewLinesHandlerChoiceCtrl->SetSelection(0);

    auto newLineHandlers = Common::Static::NewLinesHandlerList();
    for (auto i = 0; i < newLineHandlers.size(); i++) {
        pNewLinesHandlerChoiceCtrl->Append(newLineHandlers[i], new ClientData<int>(i + 1));
    }

    pBooleanHanderChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pBooleanHanderChoiceCtrl->SetSelection(0);

    auto booleanHandlers = Common::Static::BooleanHandlerList();
    for (auto i = 0; i < booleanHandlers.size(); i++) {
        pBooleanHanderChoiceCtrl->Append(booleanHandlers[i], new ClientData<int>(i + 1));
    }

    /* Dialog options */
    pCloseDialogAfterExportingCheckBoxCtrl->SetValue(pCfg->CloseExportDialogAfterExporting());

    /* Date Controls */
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();

    /* Available Columns */
    for (auto& column : Services::Export::MakeAvailableColumns()) {
        pAvailableColumnsListView->InsertItem(0, column.UserColumn);
    }

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
void ExportToCsvDialog::ConfigureEventBindings()
{
    pExportToClipboardCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnExportToClipboardCheck,
        this,
        tksIDC_COPY_TO_CLIPBOARD_CTRL
    );

    pCloseDialogAfterExportingCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnCloseDialogAfterExportingCheck,
        this,
        tksIDC_CLOSE_DIALOG_AFTER_EXPORT_CTRL
    );

    pOpenExplorerInExportDirectoryCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnOpenExplorerInExportDirectoryCheck,
        this,
        TKSIDC_OPENEXPLORERINEXPORTDIRECTORYCHECKBOXCTRL
    );

    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation,
        this,
        tksIDC_BROWSE_EXPORT_PATH_CTRL
    );

    pDelimiterChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnDelimiterChoiceSelection,
        this
    );

    pTextQualifierChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnTextQualifierChoiceSelection,
        this
    );

    pEmptyValueHandlerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnEmptyValueHandlerChoiceSelection,
        this
    );

    pNewLinesHandlerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnNewLinesHandlerChoiceSelection,
        this
    );

    pBooleanHanderChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnBooleanHandlerChoiceSelection,
        this
    );

    pFromDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToCsvDialog::OnFromDateSelection,
        this,
        tksIDC_DATE_FROM_CTRL
    );

    pToDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToCsvDialog::OnToDateSelection,
        this,
        tksIDC_DATE_TO_CTRL
    );

    pExportTodaysTasksCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnExportTodaysTasksOnlyCheck,
        this,
        tksIDC_EXPORTTODAYSTASKSCHECKBOXCTRL
    );

    pWorkWeekRangeCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnWorkWeekRangeCheck,
        this,
        tksIDC_WORKWEEKRANGECHECKBOXCTRL
    );

    pPresetSaveButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnSavePreset,
        this,
        tksIDC_PRESET_SAVE_BUTTON
    );

    pPresetResetButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnResetPreset,
        this,
        tksIDC_PRESET_RESET_BUTTON
    );

    pPresetsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnPresetChoice,
        this,
        tksIDC_PRESET_CHOICE_CTRL
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &ExportToCsvDialog::OnAvailableColumnItemCheck,
        this,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_UNCHECKED,
        &ExportToCsvDialog::OnAvailableColumnItemUncheck,
        this,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL
    );

    pRightChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnAddAvailableColumnToExportColumnListView,
        this,
        tksIDC_RIGHT_CHEV_CTRL
    );

    pLeftChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnRemoveExportColumnToAvailableColumnList,
        this,
        tksIDC_LEFT_CHEV_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_STARTED,
        &ExportToCsvDialog::OnExportColumnEditingStart,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_DONE,
        &ExportToCsvDialog::OnExportColumnEditingDone,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_SELECTION_CHANGED,
        &ExportToCsvDialog::OnExportColumnSelectionChanged,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pUpButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnUpButtonSort,
        this,
        tksIDC_UP_BUTTON
    );

    pDownButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnDownButtonSort,
        this,
        tksIDC_DOWN_BUTTON
    );

    pExcludeHeadersCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnExcludeHeadersCheck,
        this,
        tksIDC_EXCLUDE_HEADERS_CTRL
    );

    pIncludeAttributesCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnIncludeAttributesCheck,
        this,
        tksIDC_INCLUDEATTRIBUTESCHECKBOXCTRL
    );

    pShowPreviewButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnShowPreview,
        this,
        tksIDC_SHOW_PREVIEW_BUTTON
    );

    pExportButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnExport,
        this,
        tksIDC_EXPORT_BUTTON
    );
}
// clang-format on

void ExportToCsvDialog::OnDelimiterChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int delimiterIndex = pDelimiterChoiceCtrl->GetSelection();
    ClientData<DelimiterType>* delimiterData = reinterpret_cast<ClientData<DelimiterType>*>(
        pDelimiterChoiceCtrl->GetClientObject(delimiterIndex));

    mExportOptions.Delimiter = delimiterData->GetValue();
}

void ExportToCsvDialog::OnTextQualifierChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int textQualifierIndex = pTextQualifierChoiceCtrl->GetSelection();
    ClientData<TextQualifierType>* textQualifierData =
        reinterpret_cast<ClientData<TextQualifierType>*>(
            pTextQualifierChoiceCtrl->GetClientObject(textQualifierIndex));

    mExportOptions.TextQualifier = textQualifierData->GetValue();
}

void ExportToCsvDialog::OnEmptyValueHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    int emptyValueIndex = pEmptyValueHandlerChoiceCtrl->GetSelection();
    ClientData<int>* emptyValueData = reinterpret_cast<ClientData<int>*>(
        pEmptyValueHandlerChoiceCtrl->GetClientObject(emptyValueIndex));

    mExportOptions.EmptyValuesHandler = static_cast<EmptyValues>(emptyValueData->GetValue());
}

void ExportToCsvDialog::OnNewLinesHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int newLinesIndex = pNewLinesHandlerChoiceCtrl->GetSelection();
    ClientData<int>* newLinesData = reinterpret_cast<ClientData<int>*>(
        pNewLinesHandlerChoiceCtrl->GetClientObject(newLinesIndex));

    mExportOptions.NewLinesHandler = static_cast<NewLines>(newLinesData->GetValue());
}

void ExportToCsvDialog::OnBooleanHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int booleanHandlerIndex = pBooleanHanderChoiceCtrl->GetSelection();
    ClientData<int>* booleanHandlerData = reinterpret_cast<ClientData<int>*>(
        pBooleanHanderChoiceCtrl->GetClientObject(booleanHandlerIndex));

    mExportOptions.BooleanHandler = static_cast<BooleanHandler>(booleanHandlerData->GetValue());
}

void ExportToCsvDialog::OnExportToClipboardCheck(wxCommandEvent& event)
{
    bExportToClipboard = event.IsChecked();

    if (bExportToClipboard) {
        pSaveToFileTextCtrl->Disable();
        pBrowseExportPathButton->Disable();
    } else {
        pSaveToFileTextCtrl->Enable();
        pBrowseExportPathButton->Enable();
    }
}

void ExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event)
{
    auto openDirDialog = new wxDirDialog(this,
        "Select a directory to export the data to",
        pCfg->GetExportPath(),
        wxDD_DEFAULT_STYLE,
        wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        auto selectedExportPath = openDirDialog->GetPath().ToStdString();
        auto saveToFile = fmt::format(
            "{0}\\taskies-export-{1}.csv", selectedExportPath, pDateStore->PrintTodayDate);

        pSaveToFileTextCtrl->SetValue(saveToFile);
        pSaveToFileTextCtrl->SetToolTip(saveToFile);
    }

    openDirDialog->Destroy();
}

void ExportToCsvDialog::OnCloseDialogAfterExportingCheck(wxCommandEvent& event)
{
    pCfg->CloseExportDialogAfterExporting(event.IsChecked());
    pCfg->Save();
}

void ExportToCsvDialog::OnOpenExplorerInExportDirectoryCheck(wxCommandEvent& event)
{
    bOpenExplorerInExportDirectory = event.IsChecked();
}

void ExportToCsvDialog::OnFromDateSelection(wxDateEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    wxDateTime eventDate = wxDateTime(event.GetDate());
    wxDateTime eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

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
    SPDLOG_LOGGER_TRACE(pLogger, "New from date value \"{0}\"", date::format("%F", newFromDate));

    mFromCtrlDate = eventDateUtc;
    mFromDate = newFromDate;
}

void ExportToCsvDialog::OnToDateSelection(wxDateEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Received date (wxDateTime) event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    wxDateTime eventDate = wxDateTime(event.GetDate());
    wxDateTime eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

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
    SPDLOG_LOGGER_TRACE(pLogger, "New to date value \"{0}\"", date::format("%F", newToDate));

    mToCtrlDate = eventDateUtc;
    mToDate = newToDate;
}

void ExportToCsvDialog::OnExportTodaysTasksOnlyCheck(wxCommandEvent& event)
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

void ExportToCsvDialog::OnWorkWeekRangeCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        auto fridayDate = pDateStore->MondayDate + (pDateStore->MondayDate - date::Thursday);
        auto fridayTimestamp = fridayDate.time_since_epoch();
        auto fridaySeconds =
            std::chrono::duration_cast<std::chrono::seconds>(fridayTimestamp).count();

        pFromDatePickerCtrl->SetValue(pDateStore->MondayDateSeconds);
        mFromCtrlDate = pDateStore->MondayDateSeconds;

        pToDatePickerCtrl->SetValue(fridaySeconds);
        mToCtrlDate = fridaySeconds;

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

void ExportToCsvDialog::OnResetPreset(wxCommandEvent& event)
{
    mExportOptions.Reset();

    pDelimiterChoiceCtrl->SetSelection(0);
    pTextQualifierChoiceCtrl->SetSelection(0);
    pEmptyValueHandlerChoiceCtrl->SetSelection(0);
    pNewLinesHandlerChoiceCtrl->SetSelection(0);
    pBooleanHanderChoiceCtrl->SetSelection(0);

    pPresetIsDefaultCheckBoxCtrl->SetValue(false);
    pPresetsChoiceCtrl->SetSelection(0);
    pPresetNameTextCtrl->ChangeValue("");

    auto columns = pExportColumnListModel->GetColumns();

    for (const auto& column : columns) {
        pAvailableColumnsListView->InsertItem(0, column.OriginalColumn);
    }

    pExportColumnListModel->Clear();

    pExcludeHeadersCheckBoxCtrl->SetValue(false);
    pIncludeAttributesCheckBoxCtrl->SetValue(false);
}

void ExportToCsvDialog::OnSavePreset(wxCommandEvent& event)
{
    if (pCfg->GetPresetCount() == MAX_PRESET_COUNT) {
        auto valMsg = "Limit of 5 presets has been exceeded";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pPresetSaveButton);
        return;
    }

    if (pExportColumnListModel->GetColumns().empty()) {
        auto valMsg = "At least one column selection is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pPresetSaveButton);
        return;
    }

    if (pPresetNameTextCtrl->GetValue().ToStdString().empty()) {
        auto valMsg = "A preset name is required";
        wxRichToolTip tooltip("Validation", valMsg);
        tooltip.SetIcon(wxICON_WARNING);
        tooltip.ShowFor(pPresetNameTextCtrl);
        return;
    }

    int presetIndex = pPresetsChoiceCtrl->GetSelection();
    ClientData<std::string>* presetData = reinterpret_cast<ClientData<std::string>*>(
        pPresetsChoiceCtrl->GetClientObject(presetIndex));

    Common::Preset preset;
    if (presetData->GetValue().empty()) {
        preset.Uuid = Utils::Uuid();
    } else {
        preset.Uuid = presetData->GetValue();
    }

    preset.Name = pPresetNameTextCtrl->GetValue().ToStdString();
    preset.IsDefault = pPresetIsDefaultCheckBoxCtrl->GetValue();
    preset.Delimiter = mExportOptions.Delimiter;
    preset.TextQualifier = mExportOptions.TextQualifier;
    preset.EmptyValuesHandler = mExportOptions.EmptyValuesHandler;
    preset.NewLinesHandler = mExportOptions.NewLinesHandler;
    preset.BooleanHandler = mExportOptions.BooleanHandler;

    std::vector<Common::PresetColumn> columns;

    for (const auto& selectedColumn : pExportColumnListModel->GetColumns()) {
        Common::PresetColumn presetColumn;
        presetColumn.Column = selectedColumn.Column;
        presetColumn.OriginalColumn = selectedColumn.OriginalColumn;
        presetColumn.Order = selectedColumn.Order;
        columns.push_back(presetColumn);
    }

    preset.Columns = columns;

    preset.ExcludeHeaders = mExportOptions.ExcludeHeaders;
    preset.IncludeAttributes = mExportOptions.IncludeAttributes;

    bool success = pCfg->TryUnsetDefaultPreset();
    if (!success) {
        pLogger->warn("Failed to unset default preset on preset save");
    }

    if (presetData->GetValue().empty()) {
        // save preset
        pCfg->SaveExportPreset(preset);

        // set as the active preset
        int selection =
            pPresetsChoiceCtrl->Append(preset.Name, new ClientData<std::string>(preset.Uuid));
        pPresetsChoiceCtrl->SetSelection(selection);
    } else {
        // update preset
        pCfg->UpdateExportPreset(preset);
    }
}

void ExportToCsvDialog::OnPresetChoice(wxCommandEvent& event)
{
    int presetIndex = event.GetSelection();
    ClientData<std::string>* presetData = reinterpret_cast<ClientData<std::string>*>(
        pPresetsChoiceCtrl->GetClientObject(presetIndex));

    if (presetData->GetValue().empty()) {
        return;
    }

    auto presetUuid = presetData->GetValue();

    auto presets = pCfg->GetPresets();
    const auto& selectedPresetToApplyIterator = std::find_if(
        presets.begin(), presets.end(), [&](const Core::Configuration::PresetSettings& preset) {
            return preset.Uuid == presetUuid;
        });

    if (selectedPresetToApplyIterator == presets.end()) {
        pLogger->warn("Could not find preset with uuid \"{1}\" in config", presetUuid);
        return;
    }

    auto& selectedPresetToApply = *selectedPresetToApplyIterator;

    ApplyPreset(selectedPresetToApply);
}

void ExportToCsvDialog::OnAvailableColumnItemCheck(wxListEvent& event)
{
    long index = event.GetIndex();

    mSelectedItemIndexes.push_back(index);

    // This code is purely just for logging purposes
    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pAvailableColumnsListView->GetItem(item);

    std::string name = item.GetText().ToStdString();

    SPDLOG_LOGGER_TRACE(pLogger, "Selected column name \"{0}\"", name);
}

void ExportToCsvDialog::OnAvailableColumnItemUncheck(wxListEvent& event)
{
    long index = event.GetIndex();

    // clang-format off
    mSelectedItemIndexes.erase(
        std::remove(
            mSelectedItemIndexes.begin(),
            mSelectedItemIndexes.end(),
            index),
        mSelectedItemIndexes.end()
    );
    // clang-format on

    // This code is purely just for logging purposes
    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pAvailableColumnsListView->GetItem(item);

    std::string name = item.GetText().ToStdString();

    SPDLOG_LOGGER_TRACE(pLogger, "Unselected column name \"{0}\"", name);
}

void ExportToCsvDialog::OnAddAvailableColumnToExportColumnListView(wxCommandEvent& WXUNUSED(event))
{
    if (mSelectedItemIndexes.size() == 0) {
        return;
    }

    // sort the item indexes by ascending order so the
    // subsequent for loop correctly iterates over the entries in reverse
    std::sort(mSelectedItemIndexes.begin(), mSelectedItemIndexes.end(), std::less{});

    int columnIndex = 0;

    for (long i = (mSelectedItemIndexes.size() - 1); 0 <= i; i--) {
        // Extract the column name text from item index
        wxListItem item;
        item.m_itemId = mSelectedItemIndexes[i];
        item.m_col = columnIndex;
        item.m_mask = wxLIST_MASK_TEXT;
        pAvailableColumnsListView->GetItem(item);

        std::string name = item.GetText().ToStdString();

        /* Add export column in data view control and update */
        pExportColumnListModel->Append(name);

        /* Remove column from available column list control */
        pAvailableColumnsListView->DeleteItem(mSelectedItemIndexes[i]);

        mSelectedItemIndexes.erase(mSelectedItemIndexes.begin() + i);

        SPDLOG_LOGGER_TRACE(pLogger, "Column \"{0}\" removed from available list", name);
    }
}

void ExportToCsvDialog::OnRemoveExportColumnToAvailableColumnList(wxCommandEvent& WXUNUSED(event))
{
    auto columnsToRemove = pExportColumnListModel->GetSelectedColumns();

    wxDataViewItemArray items;
    auto selections = pDataViewCtrl->GetSelections(items);
    if (selections > 0) {
        pExportColumnListModel->DeleteItems(items);

        for (const auto& column : columnsToRemove) {
            int i = pAvailableColumnsListView->InsertItem(0, column.OriginalColumn);
        }
        SPDLOG_LOGGER_TRACE(
            pLogger, " \"{0}\" columns removed from export list", columnsToRemove.size());
    }
}

void ExportToCsvDialog::OnExportColumnEditingStart(wxDataViewEvent& event)
{
    const wxDataViewModel* model = event.GetModel();

    wxVariant value;
    model->GetValue(value, event.GetItem(), event.GetColumn());

    SPDLOG_LOGGER_TRACE(
        pLogger, "Editing started on export column \"{0}\"", value.GetString().ToStdString());
}

void ExportToCsvDialog::OnExportColumnEditingDone(wxDataViewEvent& event)
{
    if (event.IsEditCancelled()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Edit was cancelled");
    } else {
        SPDLOG_LOGGER_TRACE(pLogger,
            "Edit completed with new value \"{0}\"",
            event.GetValue().GetString().ToStdString());

        pExportColumnListModel->ChangeItem(
            event.GetItem(), event.GetValue().GetString().ToStdString());
    }
}

void ExportToCsvDialog::OnExportColumnSelectionChanged(wxDataViewEvent& event)
{
    auto item = event.GetItem();
    if (!item.IsOk()) {
        return;
    }

    mItemToSort = item;

    const wxDataViewModel* model = event.GetModel();

    wxVariant value;
    model->GetValue(value, event.GetItem(), ColumnListModel::Col_Column);

    SPDLOG_LOGGER_TRACE(pLogger, "Selected item header: \"{0}\"", value.GetString().ToStdString());
}

void ExportToCsvDialog::OnUpButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Ordering selected header up");
        pExportColumnListModel->MoveItem(mItemToSort);

        mItemToSort.Unset();
    }
}

void ExportToCsvDialog::OnDownButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Ordering selected header down");
        pExportColumnListModel->MoveItem(mItemToSort, false);

        mItemToSort.Unset();
    }
}

void ExportToCsvDialog::OnExcludeHeadersCheck(wxCommandEvent& event)
{
    mExportOptions.ExcludeHeaders = event.IsChecked();
}

void ExportToCsvDialog::OnIncludeAttributesCheck(wxCommandEvent& event)
{
    mExportOptions.IncludeAttributes = event.IsChecked();
}

void ExportToCsvDialog::OnShowPreview(wxCommandEvent& WXUNUSED(event))
{
    SPDLOG_LOGGER_TRACE(pLogger, "Begin show preview");

    const auto& columnsToExport = pExportColumnListModel->GetColumns();
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("Please select at least one column to export",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_INFORMATION);
        return;
    }

    auto columnExportModels = Services::Export::BuildFromList(columnsToExport);

    Services::Export::ProjectionBuilder projectionBuilder(pLogger);

    std::vector<Services::Export::Projection> projections =
        projectionBuilder.BuildProjections(columnExportModels);
    std::vector<Services::Export::ColumnJoinProjection> joinProjections =
        projectionBuilder.BuildJoinProjections(columnExportModels);

    const std::string fromDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mFromDate);
    const std::string toDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mToDate);

    SPDLOG_LOGGER_TRACE(pLogger, "Export date range: [\"{0}\", \"{1}\"]", fromDate, toDate);

    Services::Export::CsvExporterService csvExporter(pLogger, mExportOptions, mDatabaseFilePath, true);

    std::string exportedDataPreview = "";
    bool success = csvExporter.ExportToCsv(
        projections, joinProjections, fromDate, toDate, exportedDataPreview);

    if (!success) {
        std::string message = "Failed to export data";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);
    }

    pDataExportPreviewTextCtrl->ChangeValue(exportedDataPreview);
}

void ExportToCsvDialog::OnExport(wxCommandEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger, "Begin export");

    const auto& columnsToExport = pExportColumnListModel->GetColumns();
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("Please select at least one column to export!",
            Common::GetProgramName(),
            wxOK_DEFAULT | wxICON_INFORMATION);
        return;
    }

    auto columnExportModels = Services::Export::BuildFromList(columnsToExport);

    Services::Export::ProjectionBuilder projectionBuilder(pLogger);

    std::vector<Services::Export::Projection> projections =
        projectionBuilder.BuildProjections(columnExportModels);
    std::vector<Services::Export::ColumnJoinProjection> joinProjections =
        projectionBuilder.BuildJoinProjections(columnExportModels);

    const std::string fromDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mFromDate);
    const std::string toDate =
        bExportTodaysTasksOnly ? pDateStore->PrintTodayDate : date::format("%F", mToDate);

    SPDLOG_LOGGER_TRACE(pLogger, "Export date range: [\"{0}\", \"{1}\"]", fromDate, toDate);

    Services::Export::CsvExporterService csvExporter(pLogger, mExportOptions, mDatabaseFilePath, false);

    std::string exportedData = "";
    bool success =
        csvExporter.ExportToCsv(projections, joinProjections, fromDate, toDate, exportedData);

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
        if (!exportFile.is_open()) {
            pLogger->error("Failed to open export file at path \"{0}\"",
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

    if (bOpenExplorerInExportDirectory) {
        {
            SHELLEXECUTEINFO sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_DEFAULT;
            sei.hwnd = GetHWND();
            sei.lpVerb = L"open";
            sei.lpDirectory = wxPathOnly(pSaveToFileTextCtrl->GetValue()).ToStdWstring().c_str();
            sei.nShow = SW_SHOW;

            ShellExecuteEx(&sei);
        }
    }

    if (pCfg->CloseExportDialogAfterExporting()) {
        EndDialog(wxID_OK);
    }
}

void ExportToCsvDialog::SetFromAndToDatePickerRanges()
{
    pFromDatePickerCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDatePickerCtrl->SetRange(MakeMaximumFromDate(), latestPossibleDatePlusOneDay);

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void ExportToCsvDialog::SetFromDateAndDatePicker()
{
    pFromDatePickerCtrl->SetValue(pDateStore->MondayDateSeconds);

    mFromCtrlDate = pDateStore->MondayDateSeconds;
}

void ExportToCsvDialog::SetToDateAndDatePicker()
{
    pToDatePickerCtrl->SetValue(pDateStore->SundayDateSeconds);

    mToCtrlDate = pDateStore->SundayDateSeconds;
}

void ExportToCsvDialog::ApplyPreset(const Core::Configuration::PresetSettings& presetSettings)
{
    pDelimiterChoiceCtrl->SetSelection(static_cast<int>(presetSettings.Delimiter));
    pTextQualifierChoiceCtrl->SetSelection(static_cast<int>(presetSettings.TextQualifier));
    pEmptyValueHandlerChoiceCtrl->SetSelection(static_cast<int>(presetSettings.EmptyValuesHandler));
    pNewLinesHandlerChoiceCtrl->SetSelection(static_cast<int>(presetSettings.NewLinesHandler));
    pBooleanHanderChoiceCtrl->SetSelection(static_cast<int>(presetSettings.BooleanHandler));

    pPresetNameTextCtrl->ChangeValue(presetSettings.Name);
    pPresetIsDefaultCheckBoxCtrl->SetValue(presetSettings.IsDefault);

    // apply selected columns
    for (long i = (pAvailableColumnsListView->GetItemCount() - 1); 0 <= i; i--) {
        std::string name;
        wxListItem item;
        item.m_itemId = i;
        item.m_col = 0;
        item.m_mask = wxLIST_MASK_TEXT;
        pAvailableColumnsListView->GetItem(item);

        name = item.GetText().ToStdString();

        auto presetOriginalColumnIterator = std::find_if(presetSettings.Columns.begin(),
            presetSettings.Columns.end(),
            [=](const Core::Configuration::PresetColumnSettings& presetColumn) {
                return name == presetColumn.OriginalColumn;
            });

        if (presetOriginalColumnIterator == presetSettings.Columns.end()) {
            continue;
        }

        auto& presetColumn = *presetOriginalColumnIterator;
        /* Add export header in data view control and update */
        pExportColumnListModel->AppendStagingItem(
            presetColumn.Column, presetColumn.OriginalColumn, presetColumn.Order);

        /* Remove header from available header list control */
        pAvailableColumnsListView->DeleteItem(i);
    }

    pExportColumnListModel->AppendFromStaging();

    pExcludeHeadersCheckBoxCtrl->SetValue(presetSettings.ExcludeHeaders);
    pIncludeAttributesCheckBoxCtrl->SetValue(presetSettings.IncludeAttributes);

    mExportOptions.Delimiter = presetSettings.Delimiter;
    mExportOptions.TextQualifier = presetSettings.TextQualifier;
    mExportOptions.EmptyValuesHandler = presetSettings.EmptyValuesHandler;
    mExportOptions.NewLinesHandler = presetSettings.NewLinesHandler;
    mExportOptions.BooleanHandler = presetSettings.BooleanHandler;

    mExportOptions.ExcludeHeaders = presetSettings.ExcludeHeaders;
    mExportOptions.IncludeAttributes = presetSettings.IncludeAttributes;
}
} // namespace tks::UI::dlg
