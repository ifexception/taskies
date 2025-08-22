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

#include "exporttoexceldlg.h"

#include <algorithm>

#include <wx/dirdlg.h>
#include <wx/persist/toplevel.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../../common/common.h"
#include "../../../common/constants.h"
#include "../../../common/enums.h"

#include "../../../services/export/availablecolumns.h"
#include "../../../services/export/exportoptions.h"
#include "../../../services/export/columnexportmodel.h"
#include "../../../services/export/columnjoinprojection.h"
#include "../../../services/export/projection.h"
#include "../../../services/export/projectionbuilder.h"
#include "../../../services/export/excelexporterservice.h"

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
ExportToExcelDialog::ExportToExcelDialog(wxWindow* parent,
    std::shared_ptr<Core::Configuration> cfg,
    std::shared_ptr<spdlog::logger> logger,
    const std::string& databasePath,
    const wxString& name)
    : wxDialog(parent,
          wxID_ANY,
          "Export to Excel",
          wxDefaultPosition,
          wxDefaultSize,
          wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER,
          name)
    , pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pDateStore(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pCloseDialogAfterExportingCheckBoxCtrl(nullptr)
    , pOpenExplorerInExportDirectoryCheckBoxCtrl(nullptr)
    , pNewLinesHandlerChoiceCtrl(nullptr)
    , pBooleanHanderChoiceCtrl(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , pExportTodaysTasksCheckBoxCtrl(nullptr)
    , pWorkWeekRangeCheckBoxCtrl(nullptr)
    , pPresetNameTextCtrl(nullptr)
    , pPresetIsDefaultCheckBoxCtrl(nullptr)
    , pPresetSaveButton(nullptr)
    , pPresetResetButton(nullptr)
    , pPresetsChoiceCtrl(nullptr)
    , pAvailableColumnsListView(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pDataViewCtrl(nullptr)
    , pExportColumnListModel(nullptr)
    , pUpButton(nullptr)
    , pDownButton(nullptr)
    , pIncludeAttributesCheckBoxCtrl(nullptr)
    , pExportButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
    , mSelectedItemIndexes()
    , mItemToSort()
    , bOpenExplorerInExportDirectory(false)
    , bExportTodaysTasksOnly(false)
    , bIncludeAttributes(false)
    , mNewLinesOption(NewLines::None)
    , mBooleanOption(BooleanHandler::OneZero)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    Create();
    if (!wxPersistenceManager::Get().RegisterAndRestore(this)) {
        SetSize(FromDIP(wxSize(480, 600)));
    }

    wxIconBundle iconBundle(Common::GetProgramIconBundleName(), 0);
    SetIcons(iconBundle);
}
void ExportToExcelDialog::Create()
{
    CreateControls();
    FillControls();
    ConfigureEventBindings();
}

void ExportToExcelDialog::CreateControls()
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

    /* Save to file text control */
    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVETOFILETEXTCTRL, wxEmptyString);

    pBrowseExportPathButton =
        new wxButton(outputStaticBox, tksIDC_BROWSEEXPORTPATHBUTTON, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set the directory to save the exported data to");

    /* Close dialog after export check box control */
    pCloseDialogAfterExportingCheckBoxCtrl = new wxCheckBox(outputStaticBox,
        tksIDC_CLOSEDIALOGAFTEREXPORTINGCHECKBOXCTRL,
        "Close dialog after exporting");
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
    pPresetNameTextCtrl = new wxTextCtrl(presetsStaticBox, tksIDC_PRESETNAMETEXTCTRL, "");
    pPresetNameTextCtrl->SetHint("Preset name");
    pPresetNameTextCtrl->SetToolTip("Name of the preset");

    /* Preset is default checkbox control */
    pPresetIsDefaultCheckBoxCtrl =
        new wxCheckBox(presetsStaticBox, tksIDC_PRESETISDEFAULTCHECKBOXCTRL, "Is Default");
    pPresetIsDefaultCheckBoxCtrl->SetToolTip(
        "A default preset will be selected and applied automatically");

    pPresetSaveButton = new wxButton(presetsStaticBox, tksIDC_PRESETSAVEBUTTON, "Save");
    pPresetSaveButton->SetToolTip("Create new or update existing preset");

    pPresetResetButton = new wxButton(presetsStaticBox, tksIDC_PRESETRESETBUTTON, "Reset");
    pPresetResetButton->SetToolTip("Reset all options to their defaults");

    /* Presets selection controls */
    auto presetsChoiceLabel = new wxStaticText(presetsStaticBox, wxID_ANY, "Preset");
    pPresetsChoiceCtrl = new wxChoice(presetsStaticBox, tksIDC_PRESETCHOICECTRL);

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

    /* New lines choice control */
    auto newLinesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "New Lines");
    pNewLinesHandlerChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_NEW_LINES_HANDLER_CTRL);
    pNewLinesHandlerChoiceCtrl->SetToolTip("Set how to handle multiline field values");

    /* Boolean handler control */
    auto booleanHandlerLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Booleans");
    pBooleanHanderChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_BOOLEAN_HANDLER_CTRL);
    pBooleanHanderChoiceCtrl->SetToolTip("Set how to handle boolean field values");

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
    pFromDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_FROMDATEPICKERCTRL);
    pFromDatePickerCtrl->SetToolTip("Set the earliest inclusive date to export the data from");

    /* To date control */
    auto toDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "To: ");
    pToDatePickerCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_TODATEPICKERCTRL);
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

    /* Horizontal Line */
    auto line1 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line1, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(4)).Expand());

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
        tksIDC_AVAILABLECOLUMNSLISTVIEW,
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
        dataToExportStaticBox, tksIDC_RIGHTCHEVRONBUTTON, ">", wxDefaultPosition, wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a header to include in Excel");
    pLeftChevronButton = new wxButton(
        dataToExportStaticBox, tksIDC_LEFTCHEVRONBUTTON, "<", wxDefaultPosition, wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a header to exclude from Excel");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Export Headers data view list control */
    pDataViewCtrl = new wxDataViewCtrl(dataToExportStaticBox,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES);
    pDataViewCtrl->SetToolTip("Headers to export to Excel file");
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
    pUpButton = new wxButton(dataToExportStaticBox, tksIDC_UPBUTTON, "Up");
    pUpButton->SetToolTip("Move the selected header up");
    pDownButton = new wxButton(dataToExportStaticBox, tksIDC_DOWNBUTTON, "Down");
    pDownButton->SetToolTip("Move the selected header down");

    upDownButtonSizer->Add(pUpButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    upDownButtonSizer->Add(pDownButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Export checkbox options */
    pIncludeAttributesCheckBoxCtrl = new wxCheckBox(
        dataToExportStaticBox, tksIDC_INCLUDEATTRIBUTESCHECKBOXCTRL, "Include Attributes");
    pIncludeAttributesCheckBoxCtrl->SetToolTip("Include task attributes in the CSV export");
    dataToExportStaticBoxSizer->Add(
        pIncludeAttributesCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line2 = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line2, wxSizerFlags().Border(wxTOP | wxLEFT | wxRIGHT, FromDIP(4)).Expand());

    /* Export|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pExportButton = new wxButton(this, tksIDC_EXPORTBUTTON, "Export");
    pExportButton->SetDefault();
    pExportButton->SetFocus();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Close");

    buttonsSizer->Add(pExportButton, wxSizerFlags().Border(wxALL, FromDIP(2)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(2)));

    SetSizerAndFit(sizer);
}

void ExportToExcelDialog::FillControls()
{
    /* Export File Controls */
    auto saveToFile = fmt::format(
        "{0}\\taskies-export-{1}.xlsx", pCfg->GetExportPath(), pDateStore->PrintTodayDate);
    pSaveToFileTextCtrl->ChangeValue(saveToFile);
    pSaveToFileTextCtrl->SetToolTip(saveToFile);

    /* Dialog options */
    pCloseDialogAfterExportingCheckBoxCtrl->SetValue(pCfg->CloseExportDialogAfterExporting());

    /* Date Controls */
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();

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
void ExportToExcelDialog::ConfigureEventBindings()
{
    pCloseDialogAfterExportingCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToExcelDialog::OnCloseDialogAfterExportingCheck,
        this,
        tksIDC_CLOSEDIALOGAFTEREXPORTINGCHECKBOXCTRL
    );

    pOpenExplorerInExportDirectoryCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToExcelDialog::OnOpenExplorerInExportDirectoryCheck,
        this,
        TKSIDC_OPENEXPLORERINEXPORTDIRECTORYCHECKBOXCTRL
    );

    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnOpenDirectoryForSaveToFileLocation,
        this,
        tksIDC_BROWSEEXPORTPATHBUTTON
    );

    pNewLinesHandlerChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToExcelDialog::OnNewLinesHandlerChoiceSelection,
        this
    );

    pBooleanHanderChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToExcelDialog::OnBooleanHandlerChoiceSelection,
        this
    );

    pFromDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToExcelDialog::OnFromDateSelection,
        this,
        tksIDC_FROMDATEPICKERCTRL
    );

    pToDatePickerCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToExcelDialog::OnToDateSelection,
        this,
        tksIDC_TODATEPICKERCTRL
    );

    pExportTodaysTasksCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToExcelDialog::OnExportTodaysTasksOnlyCheck,
        this,
        tksIDC_EXPORTTODAYSTASKSCHECKBOXCTRL
    );

    pWorkWeekRangeCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToExcelDialog::OnWorkWeekRangeCheck,
        this,
        tksIDC_WORKWEEKRANGECHECKBOXCTRL
    );

    pPresetSaveButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnSavePreset,
        this,
        tksIDC_PRESETSAVEBUTTON
    );

    pPresetResetButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnResetPreset,
        this,
        tksIDC_PRESETRESETBUTTON
    );

    pPresetsChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToExcelDialog::OnPresetChoice,
        this,
        tksIDC_PRESETCHOICECTRL
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &ExportToExcelDialog::OnAvailableColumnItemCheck,
        this,
        tksIDC_AVAILABLECOLUMNSLISTVIEW
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_UNCHECKED,
        &ExportToExcelDialog::OnAvailableColumnItemUncheck,
        this,
        tksIDC_AVAILABLECOLUMNSLISTVIEW
    );

    pRightChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnAddAvailableColumnToExportColumnListView,
        this,
        tksIDC_RIGHTCHEVRONBUTTON
    );

    pLeftChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnRemoveExportColumnToAvailableColumnList,
        this,
        tksIDC_LEFTCHEVRONBUTTON
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_STARTED,
        &ExportToExcelDialog::OnExportColumnEditingStart,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_DONE,
        &ExportToExcelDialog::OnExportColumnEditingDone,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_SELECTION_CHANGED,
        &ExportToExcelDialog::OnExportColumnSelectionChanged,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pUpButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnUpButtonSort,
        this,
        tksIDC_UPBUTTON
    );

    pDownButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnDownButtonSort,
        this,
        tksIDC_DOWNBUTTON
    );

    pIncludeAttributesCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToExcelDialog::OnIncludeAttributesCheck,
        this,
        tksIDC_INCLUDEATTRIBUTESCHECKBOXCTRL
    );

    pExportButton->Bind(
        wxEVT_BUTTON,
        &ExportToExcelDialog::OnExport,
        this,
        tksIDC_EXPORTBUTTON
    );
}
// clang-format on

void ExportToExcelDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event)
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

void ExportToExcelDialog::OnCloseDialogAfterExportingCheck(wxCommandEvent& event)
{
    pCfg->CloseExportDialogAfterExporting(event.IsChecked());
    pCfg->Save();
}

void ExportToExcelDialog::OnOpenExplorerInExportDirectoryCheck(wxCommandEvent& event)
{
    bOpenExplorerInExportDirectory = event.IsChecked();
}

void ExportToExcelDialog::OnNewLinesHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int newLinesIndex = pNewLinesHandlerChoiceCtrl->GetSelection();
    ClientData<int>* newLinesData = reinterpret_cast<ClientData<int>*>(
        pNewLinesHandlerChoiceCtrl->GetClientObject(newLinesIndex));

    mNewLinesOption = static_cast<NewLines>(newLinesData->GetValue());
}

void ExportToExcelDialog::OnBooleanHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();
    int booleanHandlerIndex = pBooleanHanderChoiceCtrl->GetSelection();
    ClientData<int>* booleanHandlerData = reinterpret_cast<ClientData<int>*>(
        pBooleanHanderChoiceCtrl->GetClientObject(booleanHandlerIndex));

    mBooleanOption = static_cast<BooleanHandler>(booleanHandlerData->GetValue());
}

void ExportToExcelDialog::OnFromDateSelection(wxDateEvent& event)
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

void ExportToExcelDialog::OnToDateSelection(wxDateEvent& event)
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

void ExportToExcelDialog::OnExportTodaysTasksOnlyCheck(wxCommandEvent& event)
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

void ExportToExcelDialog::OnWorkWeekRangeCheck(wxCommandEvent& event)
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

void ExportToExcelDialog::OnResetPreset(wxCommandEvent& event)
{
    pPresetIsDefaultCheckBoxCtrl->SetValue(false);
    pPresetsChoiceCtrl->SetSelection(0);
    pPresetNameTextCtrl->ChangeValue("");

    pNewLinesHandlerChoiceCtrl->SetSelection(0);
    pBooleanHanderChoiceCtrl->SetSelection(0);

    auto columns = pExportColumnListModel->GetColumns();

    for (const auto& column : columns) {
        pAvailableColumnsListView->InsertItem(0, column.OriginalColumn);
    }

    pExportColumnListModel->Clear();

    pIncludeAttributesCheckBoxCtrl->SetValue(false);
}

void ExportToExcelDialog::OnSavePreset(wxCommandEvent& event)
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

    const auto& presets = pCfg->GetPresets();
    auto presetIterator = std::find_if(
        presets.begin(), presets.end(), [&](const Core::Configuration::PresetSettings cfgPreset) {
            return cfgPreset.Uuid == preset.Uuid;
        });

    // even though this preset is loaded for Excel
    // presets are shared between CSV and Excel dialogs so we need to preserve
    // CSV options regardless and if not found, reset to the defaults
    if (presetIterator == presets.end()) {
        Services::Export::ExportOptions exportOptions;

        preset.Delimiter = exportOptions.Delimiter;
        preset.TextQualifier = exportOptions.TextQualifier;
        preset.EmptyValuesHandler = exportOptions.EmptyValuesHandler;
        preset.NewLinesHandler = exportOptions.NewLinesHandler;
        preset.BooleanHandler = exportOptions.BooleanHandler;

        preset.ExcludeHeaders = false;
    } else {
        preset.Delimiter = presetIterator->Delimiter;
        preset.TextQualifier = presetIterator->TextQualifier;
        preset.EmptyValuesHandler = presetIterator->EmptyValuesHandler;
        preset.NewLinesHandler = mNewLinesOption;
        preset.BooleanHandler = mBooleanOption;

        preset.ExcludeHeaders = presetIterator->ExcludeHeaders;
    }

    preset.Name = pPresetNameTextCtrl->GetValue().ToStdString();
    preset.IsDefault = pPresetIsDefaultCheckBoxCtrl->GetValue();

    std::vector<Common::PresetColumn> columns;

    for (const auto& selectedColumn : pExportColumnListModel->GetColumns()) {
        Common::PresetColumn presetColumn;
        presetColumn.Column = selectedColumn.Column;
        presetColumn.OriginalColumn = selectedColumn.OriginalColumn;
        presetColumn.Order = selectedColumn.Order;
        columns.push_back(presetColumn);
    }

    preset.Columns = columns;

    preset.IncludeAttributes = bIncludeAttributes;

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

void ExportToExcelDialog::OnPresetChoice(wxCommandEvent& event)
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

void ExportToExcelDialog::OnAvailableColumnItemCheck(wxListEvent& event)
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

void ExportToExcelDialog::OnAvailableColumnItemUncheck(wxListEvent& event)
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

void ExportToExcelDialog::OnAddAvailableColumnToExportColumnListView(
    wxCommandEvent& WXUNUSED(event))
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

void ExportToExcelDialog::OnRemoveExportColumnToAvailableColumnList(wxCommandEvent& WXUNUSED(event))
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

void ExportToExcelDialog::OnExportColumnEditingStart(wxDataViewEvent& event)
{
    const wxDataViewModel* model = event.GetModel();

    wxVariant value;
    model->GetValue(value, event.GetItem(), event.GetColumn());

    SPDLOG_LOGGER_TRACE(
        pLogger, "Editing started on export column \"{0}\"", value.GetString().ToStdString());
}

void ExportToExcelDialog::OnExportColumnEditingDone(wxDataViewEvent& event)
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

void ExportToExcelDialog::OnExportColumnSelectionChanged(wxDataViewEvent& event)
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

void ExportToExcelDialog::OnUpButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Ordering selected header up");
        pExportColumnListModel->MoveItem(mItemToSort);

        mItemToSort.Unset();
    }
}

void ExportToExcelDialog::OnDownButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        SPDLOG_LOGGER_TRACE(pLogger, "Ordering selected header down");
        pExportColumnListModel->MoveItem(mItemToSort, false);

        mItemToSort.Unset();
    }
}

void ExportToExcelDialog::OnIncludeAttributesCheck(wxCommandEvent& event)
{
    bIncludeAttributes = event.IsChecked();
}

void ExportToExcelDialog::OnExport(wxCommandEvent& event)
{
    SPDLOG_LOGGER_TRACE(pLogger, "Begin export");

    const auto& columnsToExport = pExportColumnListModel->GetColumns();
    SPDLOG_LOGGER_TRACE(pLogger, "Count of columns to export: \"{0}\"", columnsToExport.size());

    if (columnsToExport.size() == 0) {
        wxMessageBox("Please select at least one column to export.",
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

    Services::Export::ExcelExporterService excelExporterService(
        pLogger, mDatabaseFilePath, bIncludeAttributes, mNewLinesOption, mBooleanOption);

    const std::string& saveLocation = pSaveToFileTextCtrl->GetValue().ToStdString();
    bool success = excelExporterService.ExportToExcel(
        projections, joinProjections, fromDate, toDate, saveLocation);

    if (!success) {
        std::string message = "Failed to export data to Excel";
        wxCommandEvent* addNotificationEvent = new wxCommandEvent(tksEVT_ADDNOTIFICATION);
        NotificationClientData* clientData =
            new NotificationClientData(NotificationType::Error, message);
        addNotificationEvent->SetClientObject(clientData);

        wxQueueEvent(pParent, addNotificationEvent);

        return;
    }

    std::string message = "Successfully exported data to Excel";

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

void ExportToExcelDialog::SetFromAndToDatePickerRanges()
{
    pFromDatePickerCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDatePickerCtrl->SetRange(MakeMaximumFromDate(), latestPossibleDatePlusOneDay);

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void ExportToExcelDialog::SetFromDateAndDatePicker()
{
    pFromDatePickerCtrl->SetValue(pDateStore->MondayDateSeconds);

    mFromCtrlDate = pDateStore->MondayDateSeconds;
}

void ExportToExcelDialog::SetToDateAndDatePicker()
{
    pToDatePickerCtrl->SetValue(pDateStore->SundayDateSeconds);

    mToCtrlDate = pDateStore->SundayDateSeconds;
}

void ExportToExcelDialog::ApplyPreset(const Core::Configuration::PresetSettings& presetSettings)
{
    pPresetNameTextCtrl->ChangeValue(presetSettings.Name);
    pPresetIsDefaultCheckBoxCtrl->SetValue(presetSettings.IsDefault);

    pNewLinesHandlerChoiceCtrl->SetSelection(static_cast<int>(presetSettings.NewLinesHandler));
    pBooleanHanderChoiceCtrl->SetSelection(static_cast<int>(presetSettings.BooleanHandler));

    mNewLinesOption = presetSettings.NewLinesHandler;
    mBooleanOption = presetSettings.BooleanHandler;

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

    pIncludeAttributesCheckBoxCtrl->SetValue(presetSettings.IncludeAttributes);
    bIncludeAttributes = presetSettings.IncludeAttributes;
}
} // namespace tks::UI::dlg
