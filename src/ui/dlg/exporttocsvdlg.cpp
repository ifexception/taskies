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
    , pDefaultHeadersListView(nullptr)
    , pRightChevronButton(nullptr)
    , pLeftChevronButton(nullptr)
    , pDataViewCtrl(nullptr)
    , pUpButton(nullptr)
    , pDownButton(nullptr)
    , pExcludeHeadersCheckBoxCtrl(nullptr)
    , pExportButton(nullptr)
    , pCancelButton(nullptr)
    , mFromDate()
    , mToDate()
{
    pDateStore = std::make_unique<DateStore>(pLogger);

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
    pDefaultHeadersListView = new wxListView(dataToExportStaticBox,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_SINGLE_SEL | wxLC_REPORT | wxLC_HRULES);
    pDefaultHeadersListView->EnableCheckBoxes();
    pDefaultHeadersListView->SetToolTip(
        "Available headers that can be exported (use the controls to select which headers to export)");
    headerControlsHorizontalSizer->Add(pDefaultHeadersListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    wxListItem defaultHeader;
    defaultHeader.SetId(0);
    defaultHeader.SetText("Available Headers");
    defaultHeader.SetWidth(180);
    pDefaultHeadersListView->InsertColumn(0, defaultHeader);

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
    pExportHeaderListModel = new ExportHeadersListModel(pLogger);
    pDataViewCtrl->AssociateModel(pExportHeaderListModel.get());

    /* Toggled Column */
    pDataViewCtrl->AppendToggleColumn("", ExportHeadersListModel::Col_Toggled, wxDATAVIEW_CELL_ACTIVATABLE);

    /* Header Column */
    auto* textRenderer = new wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);
    wxDataViewColumn* headerEditableColumn = new wxDataViewColumn("Headers",
        textRenderer,
        ExportHeadersListModel::Col_Header,
        wxCOL_WIDTH_AUTOSIZE,
        wxALIGN_LEFT,
        wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE);
    headerEditableColumn->SetMinWidth(120);
    pDataViewCtrl->AppendColumn(headerEditableColumn);

    /* OrderIndex Column */
    auto* orderRenderer = new wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT);
    auto* orderColumn = new wxDataViewColumn("Order",
        orderRenderer,
        ExportHeadersListModel::Col_OrderIndex,
        FromDIP(32),
        wxALIGN_CENTER,
        wxDATAVIEW_COL_HIDDEN);
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

    /* Available Headers */
    int columnIndex = 0;

    for (auto& header : Common::Static::AvailableHeaderList()) {
        pDefaultHeadersListView->InsertItem(0, header);
    }
}

// clang-format off
void ExportToCsvDialog::ConfigureEventBindings()
{
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

    pDefaultHeadersListView->Bind(
        wxEVT_LIST_ITEM_CHECKED,
        &ExportToCsvDialog::OnAvailableHeaderItemCheck,
        this,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL
    );

    pDefaultHeadersListView->Bind(
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
}
// clang-format on

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
    pDefaultHeadersListView->GetItem(item);

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
    pDefaultHeadersListView->GetItem(item);

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

    int orderIndex = 0;
    int columnIndex = 0;

    for (long i = (mSelectedItemIndexes.size() - 1); 0 <= i; i--) {
        // Extract the header name text from item index
        std::string name;
        wxListItem item;
        item.m_itemId = mSelectedItemIndexes[i];
        item.m_col = columnIndex;
        item.m_mask = wxLIST_MASK_TEXT;
        pDefaultHeadersListView->GetItem(item);

        name = item.GetText().ToStdString();

        /* Uncheck the item (not sure if this is needed) */
        // pDefaultHeadersListView->CheckItem(index, false);

        /* Add export header in data view control and update */
        pExportHeaderListModel->Append(name, orderIndex++);

        /* Remove header from available header list control */
        pDefaultHeadersListView->DeleteItem(mSelectedItemIndexes[i]);

        pLogger->info("ExportToCsvDialog::OnAddAvailableHeadertoExportHeaderList - Header \"{0}\" moved from available "
                      "to export list",
            name);
    }
}

void ExportToCsvDialog::OnRemoveExportHeaderToAvailableHeaderList(wxCommandEvent& WXUNUSED(event))
{
    auto headersToRemove = pExportHeaderListModel->GetSelectedHeaders();
    wxDataViewItemArray items;
    auto selections = pDataViewCtrl->GetSelections(items);
    if (selections > 0) {
        pExportHeaderListModel->DeleteItems(items);

        for (const auto& header : headersToRemove) {
            pDefaultHeadersListView->InsertItem(0, header);
        }
    }
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
