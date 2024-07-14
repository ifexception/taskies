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

#include "exporttocsvdlg.h"

#include <algorithm>

#include <date/date.h>

#include <fmt/format.h>

#include <wx/dirdlg.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>

#include "../../common/common.h"
#include "../../common/enums.h"

#include "../../core/environment.h"
#include "../../core/configuration.h"
#include "../clientdata.h"

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
std::vector<AvailableColumn> AvailableColumns()
{
    AvailableColumn employer{ "name", "Employer", "employers", "employer_id" };
    AvailableColumn client{ "name", "Client", "clients", "client_id" };
    AvailableColumn project{ "name", "Project", "projects", "project_id" };
    AvailableColumn projectDisplayName{ "display_name", "Display Name", "projects", "project_id" };
    AvailableColumn category{ "name", "Category", "categories", "cateogory_id" };
    AvailableColumn date{ "date", "Date", "workdays", "workday_id" };
    AvailableColumn description{ "description", "Description", "tasks" };
    AvailableColumn billable{ "billable", "Billable", "tasks" };
    AvailableColumn uid{ "unique_identifier", "Unique ID", "tasks" };
    AvailableColumn time{ "*time*", "Duration", "tasks" }; // *time* special identifier to select two columns into one

    return std::vector<AvailableColumn>{
        employer, client, project, projectDisplayName, category, date, description, billable, uid, time
    };
}

ExportToCsvDialog::ExportToCsvDialog(wxWindow* parent,
    std::shared_ptr<Core::Environment> env,
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
    , pEnv(env)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pDateStore(nullptr)
    , pDelimiterChoiceCtrl(nullptr)
    , pTextQualifierChoiceCtrl(nullptr)
    , pEolTerminatorChoiceCtrl(nullptr)
    , pEmptyValueHandlerChoiceCtrl(nullptr)
    , pNewLinesHandlerChoiceCtrl(nullptr)
    , pRemoveCommasCheckBoxCtrl(nullptr)
    , pExportToClipboardCheckBoxCtrl(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pFromDateCtrl(nullptr)
    , pToDateCtrl(nullptr)
    , pAvailableColumnsListView(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pDataViewCtrl(nullptr)
    , pUpButton(nullptr)
    , pDownButton(nullptr)
    , pExcludeHeadersCheckBoxCtrl(nullptr)
    , pDataExportPreviewTextCtrl(nullptr)
    , pShowPreviewButton(nullptr)
    , pExportButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
    , mCsvOptions()
    , mCsvExporter(pLogger, mCsvOptions)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    Create();

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

    /* Sizer for Options and Output controls */
    auto horizontalBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(horizontalBoxSizer, wxSizerFlags().Expand());

    /* Options static box (left) */
    auto optionsStaticBox = new wxStaticBox(this, wxID_ANY, "Options");
    auto optionsStaticBoxSizer = new wxStaticBoxSizer(optionsStaticBox, wxVERTICAL);
    horizontalBoxSizer->Add(optionsStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* Flex grid sizer for option choices */
    auto optionsFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    optionsStaticBoxSizer->Add(optionsFlexGridSizer, wxSizerFlags().Expand().Proportion(1));

    /* Delimiter choice control */
    auto delimiterLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Delimiter");
    pDelimiterChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_DELIMITER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pDelimiterChoiceCtrl->SetToolTip("Set the field separator character");

    /* Text qualifiers choice control */
    auto textQualifierLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Text Qualifier");
    pTextQualifierChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_TEXT_QUALIFIER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pTextQualifierChoiceCtrl->SetToolTip("Set the text qualifier for text values");

    /* End of line choice control */
    auto eolLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "End of Line");
    pEolTerminatorChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_EOL_TERMINATOR_CTRL, wxDefaultPosition, wxSize(128, -1));
    pEolTerminatorChoiceCtrl->SetToolTip("Set the end of line qualifier for each row");

    /* Empty values choice control */
    auto emptyValuesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Empty Values");
    pEmptyValueHandlerChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_EMPTY_VALUE_HANDLER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pEmptyValueHandlerChoiceCtrl->SetToolTip("Set how to handle empty or blank field values");

    /* New lines choice control */
    auto newLinesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "New Lines");
    pNewLinesHandlerChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_NEW_LINES_HANDLER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pNewLinesHandlerChoiceCtrl->SetToolTip("Set how to handle multiline field values");

    /* Remove commans checkbox control */
    pRemoveCommasCheckBoxCtrl = new wxCheckBox(optionsStaticBox, tksIDC_REMOVE_COMMAS_CTRL, "Remove Commas");
    pRemoveCommasCheckBoxCtrl->SetToolTip("Remove commas from text to be exported");

    optionsFlexGridSizer->Add(delimiterLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(pDelimiterChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    optionsFlexGridSizer->Add(textQualifierLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(pTextQualifierChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    optionsFlexGridSizer->Add(eolLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(pEolTerminatorChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    optionsFlexGridSizer->Add(emptyValuesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(pEmptyValueHandlerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    optionsFlexGridSizer->Add(newLinesLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    optionsFlexGridSizer->Add(pNewLinesHandlerChoiceCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    optionsFlexGridSizer->Add(0, 0);
    optionsFlexGridSizer->Add(pRemoveCommasCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Output static box (right) */
    auto outputStaticBox = new wxStaticBox(this, wxID_ANY, "Output");
    auto outputStaticBoxSizer = new wxStaticBoxSizer(outputStaticBox, wxVERTICAL);
    horizontalBoxSizer->Add(outputStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Proportion(1));

    auto outputFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    outputStaticBoxSizer->Add(outputFlexGridSizer, wxSizerFlags().Expand());

    /* Export to clipboard checkbox control */
    pExportToClipboardCheckBoxCtrl =
        new wxCheckBox(outputStaticBox, tksIDC_COPY_TO_CLIPBOARD_CTRL, "Copy to Clipboard");
    pExportToClipboardCheckBoxCtrl->SetToolTip("When checked the data will be exported to the clipboard");

    /* Save to file text control */
    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVE_TO_FILE_CTRL, wxEmptyString);

    pBrowseExportPathButton = new wxButton(outputStaticBox, tksIDC_BROWSE_EXPORT_PATH_CTRL, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set where to the save the exported data to");

    outputFlexGridSizer->AddGrowableCol(1, 1);

    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(pExportToClipboardCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    outputFlexGridSizer->Add(saveToFileLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    outputFlexGridSizer->Add(pSaveToFileTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));
    outputFlexGridSizer->Add(0, 0);
    outputFlexGridSizer->Add(pBrowseExportPathButton, wxSizerFlags().Border(wxALL, FromDIP(2)).Right());

    /* Sizer for date range */
    auto dateRangeStaticBox = new wxStaticBox(this, wxID_ANY, "Date Range");
    auto dateRangeStaticBoxSizer = new wxStaticBoxSizer(dateRangeStaticBox, wxHORIZONTAL);
    sizer->Add(dateRangeStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    /* From date control */
    auto fromDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "From: ");
    pFromDateCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_FROM_CTRL);
    pFromDateCtrl->SetToolTip("Set the earliest inclusive date to export the data from");

    /* To date control */
    auto toDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "To: ");
    pToDateCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_TO_CTRL);
    pToDateCtrl->SetToolTip("Set the latest inclusive date to export the data from");

    dateRangeStaticBoxSizer->Add(fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pFromDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pToDateCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Header/Data to Export Controls sizer */
    auto dataToExportStaticBox = new wxStaticBox(this, wxID_ANY, "Data to Export");
    auto dataToExportStaticBoxSizer = new wxStaticBoxSizer(dataToExportStaticBox, wxVERTICAL);
    sizer->Add(dataToExportStaticBoxSizer, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    auto headerControlsHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
    dataToExportStaticBoxSizer->Add(headerControlsHorizontalSizer, wxSizerFlags().Expand().Proportion(1));

    /* Default headers list view controls */
    pAvailableColumnsListView = new wxListView(dataToExportStaticBox,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pAvailableColumnsListView->EnableCheckBoxes();
    pAvailableColumnsListView->SetToolTip(
        "Available headers (columns) that can be exported (use the controls to select which headers to export)");
    headerControlsHorizontalSizer->Add(pAvailableColumnsListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    int columnIndex = 0;

    wxListItem availableColumn;
    availableColumn.SetId(columnIndex);
    availableColumn.SetText("Available Headers");
    availableColumn.SetWidth(180);
    pAvailableColumnsListView->InsertColumn(columnIndex++, availableColumn);

    /* Chevrons buttons */
    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    headerControlsHorizontalSizer->Add(chevronButtonSizer, wxSizerFlags());

    pRightChevronButton =
        new wxButton(dataToExportStaticBox, tksIDC_RIGHT_CHEV_CTRL, ">", wxDefaultPosition, wxSize(32, -1));
    pRightChevronButton->SetToolTip("Select a header to be included in the export");
    pLeftChevronButton =
        new wxButton(dataToExportStaticBox, tksIDC_LEFT_CHEV_CTRL, "<", wxDefaultPosition, wxSize(32, -1));
    pLeftChevronButton->SetToolTip("Select a header to be excluded in the export (if any)");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    /* Export Headers data view list control */
    pDataViewCtrl = new wxDataViewCtrl(dataToExportStaticBox,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxDV_SINGLE | wxDV_ROW_LINES);
    pDataViewCtrl->SetToolTip("Headers to be exported to file or clipboard");
    headerControlsHorizontalSizer->Add(pDataViewCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand().Proportion(1));

    /* Model */
    pExportColumnListModel = new ExportHeadersListModel(pLogger);
    pDataViewCtrl->AssociateModel(pExportColumnListModel.get());

    /* Toggled Column */
    pDataViewCtrl->AppendToggleColumn("", ExportHeadersListModel::Col_Toggled, wxDATAVIEW_CELL_ACTIVATABLE);

    /* Header Column */
    auto* textRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);
    wxDataViewColumn* headerEditableColumn = new wxDataViewColumn("Headers",
        textRenderer,
        ExportHeadersListModel::Col_Header,
        wxCOL_WIDTH_AUTOSIZE,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_RESIZABLE);
    headerEditableColumn->SetMinWidth(120);
    pDataViewCtrl->AppendColumn(headerEditableColumn);

    /* OrderIndex Column */
    auto* orderRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);
    auto* orderColumn = new wxDataViewColumn("Order",
        orderRenderer,
        ExportHeadersListModel::Col_OrderIndex,
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
    pDownButton = new wxButton(dataToExportStaticBox, tksIDC_DOWN_BUTTON, "Down");

    upDownButtonSizer->Add(pUpButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());
    upDownButtonSizer->Add(pDownButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Center());

    pExcludeHeadersCheckBoxCtrl = new wxCheckBox(dataToExportStaticBox, tksIDC_EXCLUDE_HEADERS_CTRL, "Exclude Headers");
    dataToExportStaticBoxSizer->Add(pExcludeHeadersCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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
    dataPreviewStaticBoxSizer->Add(pDataExportPreviewTextCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    pShowPreviewButton = new wxButton(dataPreviewStaticBox, tksIDC_SHOW_PREVIEW_BUTTON, "Show Preview");
    pShowPreviewButton->SetToolTip("Show a preview of the data to be exported (if all fields are selected correctly)");

    dataPreviewStaticBoxSizer->Add(pShowPreviewButton, wxSizerFlags().Border(wxALL, FromDIP(4)).Right());

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* Export|Cancel buttons */
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(buttonsSizer, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    buttonsSizer->AddStretchSpacer();

    pExportButton = new wxButton(this, tksIDC_EXPORT_BUTTON, "Export");
    pExportButton->SetDefault();
    pExportButton->SetFocus();

    pCancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

    buttonsSizer->Add(pExportButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    buttonsSizer->Add(pCancelButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    SetSizerAndFit(sizer);
}

void ExportToCsvDialog::FillControls()
{
    pDelimiterChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pDelimiterChoiceCtrl->SetSelection(0);

    auto delimiters = Common::Static::DelimiterList();
    for (auto i = 0; i < delimiters.size(); i++) {
        pDelimiterChoiceCtrl->Append(delimiters[i], new ClientData<int>(i));
    }

    pTextQualifierChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pTextQualifierChoiceCtrl->SetSelection(0);

    auto textQualifiers = Common::Static::TextQualifierList();
    for (auto i = 0; i < textQualifiers.size(); i++) {
        pTextQualifierChoiceCtrl->Append(textQualifiers[i], new ClientData<int>(i));
    }

    pEolTerminatorChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pEolTerminatorChoiceCtrl->SetSelection(0);

    auto eolQualifiers = Common::Static::EndOfLineList();
    for (auto i = 0; i < eolQualifiers.size(); i++) {
        pEolTerminatorChoiceCtrl->Append(eolQualifiers[i], new ClientData<int>(i));
    }

    pEmptyValueHandlerChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pEmptyValueHandlerChoiceCtrl->SetSelection(0);

    auto emptyValueHandlers = Common::Static::EmptyValueHandlerList();
    for (auto i = 0; i < emptyValueHandlers.size(); i++) {
        pEmptyValueHandlerChoiceCtrl->Append(emptyValueHandlers[i], new ClientData<int>(i));
    }

    pNewLinesHandlerChoiceCtrl->Append("(default)", new ClientData<int>(-1));
    pNewLinesHandlerChoiceCtrl->SetSelection(0);

    auto newLineHandlers = Common::Static::NewLinesHandlerList();
    for (auto i = 0; i < newLineHandlers.size(); i++) {
        pNewLinesHandlerChoiceCtrl->Append(newLineHandlers[i], new ClientData<int>(i));
    }

    pRemoveCommasCheckBoxCtrl->SetValue(true);

    /* Export File Controls */
    if (true /*!pCfg->GetExportDirectory()*/) {
        auto defaultFileName = fmt::format("taskies-tasks-export-{0}.csv", pDateStore->PrintTodayDate);
        auto defaultSaveToFile = pEnv->GetExportPath() / defaultFileName;
        pSaveToFileTextCtrl->ChangeValue(defaultSaveToFile.string());
        pSaveToFileTextCtrl->SetToolTip(defaultSaveToFile.string());
    }

    /* Date Controls */
    SetFromAndToDatePickerRanges();

    SetFromDateAndDatePicker();
    SetToDateAndDatePicker();

    /* Available Columns */
    for (auto& column : AvailableColumns()) {
        pAvailableColumnsListView->InsertItem(0, column.Display);
    }
}

// clang-format off
void ExportToCsvDialog::ConfigureEventBindings()
{
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

    pEolTerminatorChoiceCtrl->Bind(
        wxEVT_CHOICE,
        &ExportToCsvDialog::OnEolTerminatorChoiceSelection,
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

    pRemoveCommasCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnRemoveCommasCheck,
        this
    );

    pExportToClipboardCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnExportToClipboardCheck,
        this
    );

    pBrowseExportPathButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation,
        this,
        tksIDC_BROWSE_EXPORT_PATH_CTRL
    );

    pFromDateCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToCsvDialog::OnFromDateSelection,
        this,
        tksIDC_DATE_FROM_CTRL
    );

    pToDateCtrl->Bind(
        wxEVT_DATE_CHANGED,
        &ExportToCsvDialog::OnToDateSelection,
        this,
        tksIDC_DATE_TO_CTRL
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &ExportToCsvDialog::OnAvailableHeaderItemCheck,
        this,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL
    );

    pAvailableColumnsListView->Bind(
        wxEVT_LIST_ITEM_UNCHECKED,
        &ExportToCsvDialog::OnAvailableHeaderItemUncheck,
        this,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL
    );

    pRightChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnAddAvailableHeaderToExportHeaderList,
        this,
        tksIDC_RIGHT_CHEV_CTRL
    );

    pLeftChevronButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnRemoveExportHeaderToAvailableHeaderList,
        this,
        tksIDC_LEFT_CHEV_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_STARTED,
        &ExportToCsvDialog::OnExportHeaderEditingStart,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_ITEM_EDITING_DONE,
        &ExportToCsvDialog::OnExportHeaderEditingDone,
        this,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL
    );

    pDataViewCtrl->Bind(
        wxEVT_DATAVIEW_SELECTION_CHANGED,
        &ExportToCsvDialog::OnExportHeaderSelectionChanged,
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

    pShowPreviewButton->Bind(
        wxEVT_BUTTON,
        &ExportToCsvDialog::OnShowPreview,
        this,
        tksIDC_SHOW_PREVIEW_BUTTON
    );
}
// clang-format on

void ExportToCsvDialog::OnDelimiterChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    pLogger->info("ExportToCsvDialog::OnDelimiterChoiceSelection - Selected delimiter \"{0}\"", choice.ToStdString());
}

void ExportToCsvDialog::OnTextQualifierChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    pLogger->info(
        "ExportToCsvDialog::OnTextQualifierChoiceSelection - Selected text qualifier \"{0}\"", choice.ToStdString());
}

void ExportToCsvDialog::OnEolTerminatorChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    pLogger->info(
        "ExportToCsvDialog::OnEolTerminatorChoiceSelection - Selected EOL terminator \"{0}\"", choice.ToStdString());
}

void ExportToCsvDialog::OnEmptyValueHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    pLogger->info("ExportToCsvDialog::OnEmptyValueHandlerChoiceSelection - Selected empty value handler \"{0}\"",
        choice.ToStdString());
}

void ExportToCsvDialog::OnNewLinesHandlerChoiceSelection(wxCommandEvent& event)
{
    auto choice = event.GetString();

    pLogger->info("ExportToCsvDialog::OnNewLinesHandlerChoiceSelection - Selected new lines handler \"{0}\"",
        choice.ToStdString());
}

void ExportToCsvDialog::OnRemoveCommasCheck(wxCommandEvent& event)
{
    auto choice = event.IsChecked();

    pLogger->info("ExportToCsvDialog::OnRemoveCommasCheck - Checked \"{0}\"", choice);
}

void ExportToCsvDialog::OnExportToClipboardCheck(wxCommandEvent& event)
{
    if (event.IsChecked()) {
        pSaveToFileTextCtrl->Disable();
        pBrowseExportPathButton->Disable();
    } else {
        pSaveToFileTextCtrl->Enable();
        pBrowseExportPathButton->Enable();
    }
}

void ExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event)
{
    std::string pathDirectoryToOpenOn;
    if (true /*!pCfg->GetExportPath().empty()*/) {
        pathDirectoryToOpenOn = pEnv->GetExportPath().string();
    } else {
        // pathDirectoryToOpenOn = pCfg->GetExportPath();
    }

    auto openDirDialog = new wxDirDialog(
        this, "Select a directory to export the data to", pathDirectoryToOpenOn, wxDD_DEFAULT_STYLE, wxDefaultPosition);
    int res = openDirDialog->ShowModal();

    if (res == wxID_OK) {
        auto defaultFileName = fmt::format("taskies-tasks-export-{0}.csv", pDateStore->PrintTodayDate);
        auto selectedExportPath = openDirDialog->GetPath().ToStdString();
        auto finalPath = selectedExportPath + "\\" + defaultFileName;

        pSaveToFileTextCtrl->SetValue(finalPath);
        pSaveToFileTextCtrl->SetToolTip(finalPath);
    }

    openDirDialog->Destroy();
}

void ExportToCsvDialog::OnFromDateSelection(wxDateEvent& event)
{
    pLogger->info("ExportToCsvDialog::OnFromDateSelection - Received date (wxDateTime) with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());
    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > mToCtrlDate) {
        SetFromDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot exceed to date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pFromDateCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newFromDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    pLogger->info("ExportToCsvDialog::OnFromDateSelection - New date value \"{0}\"", date::format("%F", newFromDate));

    mFromCtrlDate = eventDateUtc;
    mFromDate = newFromDate;
}

void ExportToCsvDialog::OnToDateSelection(wxDateEvent& event)
{
    pLogger->info("ExportToCsvDialog::OnToDateSelection - Received date (wxDateTime) event with value \"{0}\"",
        event.GetDate().FormatISODate().ToStdString());

    auto eventDate = wxDateTime(event.GetDate());

    auto eventDateUtc = eventDate.MakeFromTimezone(wxDateTime::UTC);

    if (eventDateUtc > mToLatestPossibleDate) {
        SetToDateAndDatePicker();
        return;
    }

    if (eventDateUtc < mFromCtrlDate) {
        SetFromDateAndDatePicker();
        wxRichToolTip toolTip("Invalid Date", "Selected date cannot go past from date");
        toolTip.SetIcon(wxICON_WARNING);
        toolTip.ShowFor(pToDateCtrl);
        return;
    }

    auto eventDateUtcTicks = eventDateUtc.GetTicks();
    auto newToDate = date::floor<date::days>(std::chrono::system_clock::from_time_t(eventDateUtcTicks));
    pLogger->info("ExportToCsvDialog::OnToDateSelection - New date value \"{0}\"", date::format("%F", newToDate));

    mToCtrlDate = eventDateUtc;
    mToDate = newToDate;
}

void ExportToCsvDialog::OnAvailableHeaderItemCheck(wxListEvent& event)
{
    long index = event.GetIndex();
    mSelectedItemIndexes.push_back(index);

    std::string name;

    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pAvailableColumnsListView->GetItem(item);

    name = item.GetText().ToStdString();

    pLogger->info("ExportToCsvDialog::OnAvailableHeaderItemCheck - Selected header name \"{0}\"", name);
}

void ExportToCsvDialog::OnAvailableHeaderItemUncheck(wxListEvent& event)
{
    long index = event.GetIndex();
    mSelectedItemIndexes.erase(
        std::remove(mSelectedItemIndexes.begin(), mSelectedItemIndexes.end(), index), mSelectedItemIndexes.end());

    std::string name;

    wxListItem item;
    item.m_itemId = index;
    item.m_col = 0;
    item.m_mask = wxLIST_MASK_TEXT;
    pAvailableColumnsListView->GetItem(item);

    name = item.GetText().ToStdString();

    pLogger->info("ExportToCsvDialog::OnAvailableHeaderItemUncheck - Unselected header name \"{0}\"", name);
}

void ExportToCsvDialog::OnAddAvailableHeaderToExportHeaderList(wxCommandEvent& WXUNUSED(event))
{
    if (mSelectedItemIndexes.size() == 0) {
        pLogger->info(
            "ExportToCsvDialog::OnAddAvailableHeadertoExportHeaderList - No items (headers) selected to move");
        return;
    }

    // Sort the item indexes by ascending order so the
    // subsequent for loop correctly iterates over the entries in reverse
    std::sort(mSelectedItemIndexes.begin(), mSelectedItemIndexes.end(), std::less{});

    int orderIndex = 0;
    int columnIndex = 0;

    for (long i = (mSelectedItemIndexes.size() - 1); 0 <= i; i--) {
        // Extract the header name text from item index
        std::string name;
        wxListItem item;
        item.m_itemId = mSelectedItemIndexes[i];
        item.m_col = columnIndex;
        item.m_mask = wxLIST_MASK_TEXT;
        pAvailableColumnsListView->GetItem(item);

        name = item.GetText().ToStdString();

        /* Add export header in data view control and update */
        pExportColumnListModel->Append(name, orderIndex++);

        /* Remove header from available header list control */
        pAvailableColumnsListView->DeleteItem(mSelectedItemIndexes[i]);

        mSelectedItemIndexes.erase(mSelectedItemIndexes.begin() + i);

        pLogger->info(
            "ExportToCsvDialog::OnAddAvailableHeadertoExportHeaderList - Header \"{0}\" removed from available", name);
    }
}

void ExportToCsvDialog::OnRemoveExportHeaderToAvailableHeaderList(wxCommandEvent& WXUNUSED(event))
{
    auto headersToRemove = pExportColumnListModel->GetSelectedHeaders();
    wxDataViewItemArray items;
    auto selections = pDataViewCtrl->GetSelections(items);
    if (selections > 0) {
        pExportColumnListModel->DeleteItems(items);

        for (const auto& header : headersToRemove) {
            pAvailableColumnsListView->InsertItem(0, header);
        }
    }
}

void ExportToCsvDialog::OnExportHeaderEditingStart(wxDataViewEvent& event)
{
    const wxDataViewModel* model = event.GetModel();

    wxVariant value;
    model->GetValue(value, event.GetItem(), event.GetColumn());

    pLogger->info("ExportToCsvDialog::OnExportHeaderEditingStart - Editing started on export header - {0}",
        value.GetString().ToStdString());
}

void ExportToCsvDialog::OnExportHeaderEditingDone(wxDataViewEvent& event)
{
    if (event.IsEditCancelled()) {
        pLogger->info("ExportToCsvDialog::OnExportHeaderEditingDone - Edit was cancelled");
    } else {
        pLogger->info("ExportToCsvDialog::OnExportHeaderEditingDone - Edit completed with new value: \"{0}\"",
            event.GetValue().GetString().ToStdString());

        pExportColumnListModel->ChangeItem(event.GetItem(), event.GetValue().GetString().ToStdString());
    }
}

void ExportToCsvDialog::OnExportHeaderSelectionChanged(wxDataViewEvent& event)
{
    auto item = event.GetItem();
    if (!item.IsOk()) {
        return;
    }

    mItemToSort = item;

    const wxDataViewModel* model = event.GetModel();

    wxVariant value;
    model->GetValue(value, event.GetItem(), ExportHeadersListModel::Col_Header);

    pLogger->info("ExportToCsvDialog::OnExportHeaderSelectionChanged - Selected item header: \"{0}\"",
        value.GetString().ToStdString());
}

void ExportToCsvDialog::OnUpButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        pLogger->info("ExportToCsvDialog::OnUpButtonSort - Begin ordering selected header up");
        pExportColumnListModel->MoveItem(mItemToSort);

        mItemToSort.Unset();
    }
}

void ExportToCsvDialog::OnDownButtonSort(wxCommandEvent& event)
{
    if (mItemToSort.IsOk()) {
        pLogger->info("ExportToCsvDialog::OnDownButtonSort - Begin ordering selected header down");
        pExportColumnListModel->MoveItem(mItemToSort, false);

        mItemToSort.Unset();
    }
}

void ExportToCsvDialog::OnShowPreview(wxCommandEvent& WXUNUSED(event))
{
    pLogger->info("ExportToCsvDialog::OnShowPreview - Begin show preview");

    const auto& columnsToExport = pExportColumnListModel->GetHeadersToExport();
    pLogger->info("ExportToCsvDialog::OnShowPreview - Count of columns to export: \"{0}\"", columnsToExport.size());

    const auto& availableColumnsList = AvailableColumns();
    bool isProjectsTableSelected = false;

    std::vector<Utils::Projection> projections;
    std::vector<Utils::FirstLevelJoinTable> firstLevelTablesToJoinOn;
    std::vector<Utils::SecondLevelJoinTable> secondLevelTablesToJoinOn;

    for (const auto& columnToExport : columnsToExport) {
        const auto& availableColumnIterator = std::find_if(availableColumnsList.begin(),
            availableColumnsList.end(),
            [=](const AvailableColumn& column) { return column.Display == columnToExport.OriginalHeader; });

        if (availableColumnIterator != availableColumnsList.end()) {
            const auto& availableColumn = *availableColumnIterator;
            pLogger->info(
                "ExportToCsvDialog::OnShowPreview - Matched export column \"{0}\" with available column \"{1}\"",
                columnToExport.OriginalHeader,
                availableColumn.DatabaseColumn);

            Utils::ColumnProjection cp(
                availableColumn.DatabaseColumn, columnToExport.Header, availableColumn.TableName);

            if (availableColumn.DatabaseColumn == "*time*") {
                cp.SetIdentifier("*time*");
            }

            Utils::Projection projection(columnToExport.OrderIndex, cp);

            projections.push_back(projection);

            if (availableColumn.TableName == "projects" || availableColumn.TableName == "categories") {
                Utils::FirstLevelJoinTable jt;

                if (availableColumn.TableName == "projects") {
                    isProjectsTableSelected = true;
                }

                jt.tableName = availableColumn.TableName;
                jt.idColumn = availableColumn.IdColumn;
                jt.joinType = Utils::JoinType::InnerJoin;

                pLogger->info(
                    "ExportToCsvDialog::OnShowPreview - Insert first level table to join on \"{0}\" with join \"{1}\"",
                    availableColumn.TableName,
                    "INNER");

                firstLevelTablesToJoinOn.push_back(jt);
            }

            if (availableColumn.TableName == "employers" || availableColumn.TableName == "clients") {
                Utils::SecondLevelJoinTable jt;

                jt.tableName = availableColumn.TableName;
                jt.idColumn = availableColumn.IdColumn;

                if (isProjectsTableSelected) {
                    Utils::SecondLevelJoinTable jtProjects;
                    jtProjects.isProjectsSelected = true;
                    secondLevelTablesToJoinOn.push_back(jtProjects);
                }

                if (availableColumn.TableName == "clients") {
                    jt.joinType = Utils::JoinType::LeftJoin;
                } else {
                    jt.joinType = Utils::JoinType::InnerJoin;
                }

                pLogger->info(
                    "ExportToCsvDialog::OnShowPreview - Insert second level table to join on \"{0}\" with join \"{1}\"",
                    availableColumn.TableName,
                    jt.joinType == Utils::JoinType::InnerJoin ? "INNER" : "LEFT");

                secondLevelTablesToJoinOn.push_back(jt);
            }
        }
    }

    pLogger->info("ExportToCsvDialog::OnShowPreview - Sort projections by order index ascending");
    std::sort(projections.begin(), projections.end(), [](const Utils::Projection& lhs, const Utils::Projection& rhs) {
        return lhs.orderIndex < rhs.orderIndex;
    });

    const std::string fromDate = date::format("%F", mFromDate);
    const std::string toDate = date::format("%F", mToDate);

    pLogger->info("ExportToCsvDialog::OnShowPreview - Export date range: [\"{0}\", \"{1}\"]", fromDate, toDate);
    mCsvExporter.GeneratePreview(projections, firstLevelTablesToJoinOn, secondLevelTablesToJoinOn, fromDate, toDate);
}

void ExportToCsvDialog::SetFromAndToDatePickerRanges()
{
    pFromDateCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateTime fromFromDate = wxDateTime::Now(), toFromDate = wxDateTime::Now();

    if (pFromDateCtrl->GetRange(&fromFromDate, &toFromDate)) {
        pLogger->info("ExportToCsvDialog::SetFromAndToDatePickerRanges - pFromDateCtrl range is [{0} - {1}]",
            fromFromDate.FormatISODate().ToStdString(),
            toFromDate.FormatISODate().ToStdString());
    }

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDateCtrl->SetRange(wxDateTime(pDateStore->MondayDateSeconds), latestPossibleDatePlusOneDay);

    wxDateTime toFromDate2 = wxDateTime::Now(), toToDate = wxDateTime::Now();

    if (pToDateCtrl->GetRange(&toFromDate2, &toToDate)) {
        pLogger->info("ExportToCsvDialog::SetFromAndToDatePickerRanges - pToDateCtrl range is [{0} - {1})",
            toFromDate2.FormatISODate().ToStdString(),
            toToDate.FormatISODate().ToStdString());
    }

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);
}

void ExportToCsvDialog::SetFromDateAndDatePicker()
{
    pFromDateCtrl->SetValue(pDateStore->MondayDateSeconds);

    pLogger->info("ExportToCsvDialog::SetFromDateAndDatePicker - Reset pFromDateCtrl to: {0}",
        pFromDateCtrl->GetValue().FormatISODate().ToStdString());

    mFromCtrlDate = pDateStore->MondayDateSeconds;

    pLogger->info("ExportToCsvDialog::SetFromDateAndDatePicker - Reset mFromCtrlDate to: {0}",
        mFromCtrlDate.FormatISODate().ToStdString());
}

void ExportToCsvDialog::SetToDateAndDatePicker()
{
    pToDateCtrl->SetValue(pDateStore->SundayDateSeconds);

    pLogger->info("ExportToCsvDialog::SetToDateAndDatePicker - Reset pToDateCtrl to: {0}",
        pToDateCtrl->GetValue().FormatISODate().ToStdString());

    mToCtrlDate = pDateStore->SundayDateSeconds;

    pLogger->info("ExportToCsvDialog::SetToDateAndDatePicker - Reset mToCtrlDate to: {0}",
        mToCtrlDate.FormatISODate().ToStdString());
}
} // namespace tks::UI::dlg
