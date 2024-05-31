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
    , pExportButton(nullptr)
    , pCancelButton(nullptr)
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

    auto optionsFlexGridSizer = new wxFlexGridSizer(2, FromDIP(4), FromDIP(4));
    optionsStaticBoxSizer->Add(optionsFlexGridSizer, wxSizerFlags().Expand().Proportion(1));

    auto delimiterLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Delimiter");
    pDelimiterChoiceCtrl = new wxChoice(optionsStaticBox, tksIDC_DELIMITER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pDelimiterChoiceCtrl->SetToolTip("Set the field separator character");

    auto textQualifierLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Text Qualifier");
    pTextQualifierChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_TEXT_QUALIFIER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pTextQualifierChoiceCtrl->SetToolTip("Set the text qualifier for text values");

    auto eolLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "End of Line");
    pEolTerminatorChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_EOL_TERMINATOR_CTRL, wxDefaultPosition, wxSize(128, -1));
    pEolTerminatorChoiceCtrl->SetToolTip("Set the end of line qualifier for each row");

    auto emptyValuesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "Empty Values");
    pEmptyValueHandlerChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_EMPTY_VALUE_HANDLER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pEmptyValueHandlerChoiceCtrl->SetToolTip("Set how to handle empty or blank field values");

    auto newLinesLabel = new wxStaticText(optionsStaticBox, wxID_ANY, "New Lines");
    pNewLinesHandlerChoiceCtrl =
        new wxChoice(optionsStaticBox, tksIDC_NEW_LINES_HANDLER_CTRL, wxDefaultPosition, wxSize(128, -1));
    pNewLinesHandlerChoiceCtrl->SetToolTip("Set how to handle multiline field values");

    pRemoveCommasCheckBoxCtrl = new wxCheckBox(optionsStaticBox, tksIDC_REMOVE_COMMAS_CTRL, "Remove Commas");

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

    pExportToClipboardCheckBoxCtrl =
        new wxCheckBox(outputStaticBox, tksIDC_COPY_TO_CLIPBOARD_CTRL, "Copy to Clipboard");
    pExportToClipboardCheckBoxCtrl->SetToolTip("When checked the data will be exported to the clipboard");

    auto saveToFileLabel = new wxStaticText(outputStaticBox, wxID_ANY, "Save to File");
    pSaveToFileTextCtrl = new wxTextCtrl(outputStaticBox, tksIDC_SAVE_TO_FILE_CTRL, wxEmptyString);

    pBrowseExportPathButton = new wxButton(outputStaticBox, tksIDC_BROWSE_EXPORT_PATH_CTRL, "Browse...");
    pBrowseExportPathButton->SetToolTip("Set where to the save the results to");

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

    auto fromDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "From: ");
    pFromDateCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_FROM_CTRL);

    auto toDateLabel = new wxStaticText(dateRangeStaticBox, wxID_ANY, "To: ");
    pToDateCtrl = new wxDatePickerCtrl(dateRangeStaticBox, tksIDC_DATE_TO_CTRL);

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

    pDefaultHeadersListView = new wxListView(dataToExportStaticBox, tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL);
    headerControlsHorizontalSizer->Add(
        pDefaultHeadersListView, wxSizerFlags().Border(wxALL, FromDIP(4)).Expand());

    wxListItem defaultHeader;
    defaultHeader.SetId(0);
    defaultHeader.SetText("Headers");
    defaultHeader.SetWidth(120);
    pDefaultHeadersListView->InsertColumn(0, defaultHeader);

    auto chevronButtonSizer = new wxBoxSizer(wxVERTICAL);
    headerControlsHorizontalSizer->Add(chevronButtonSizer, wxSizerFlags());

    pRightChevronButton = new wxButton(dataToExportStaticBox, tksIDC_RIGHT_CHEV_CTRL, ">");
    pLeftChevronButton = new wxButton(dataToExportStaticBox, tksIDC_LEFT_CHEV_CTRL, "<");

    chevronButtonSizer->Add(pRightChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)));
    chevronButtonSizer->Add(pLeftChevronButton, wxSizerFlags().Border(wxALL, FromDIP(4)));

    /* Horizontal Line */
    auto line = new wxStaticLine(this, wxID_ANY);
    sizer->Add(line, wxSizerFlags().Border(wxALL, FromDIP(2)).Expand());

    /* OK|Cancel buttons */
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

    /* Date Controls */
    pFromDateCtrl->SetRange(MakeMaximumFromDate(), wxDateTime(pDateStore->SundayDateSeconds));

    wxDateTime fromFromDate = wxDateTime::Now(), toFromDate = wxDateTime::Now();

    if (pFromDateCtrl->GetRange(&fromFromDate, &toFromDate)) {
        pLogger->info("ExportToCsvDialog::FillControls - pFromDateCtrl range is [{0} - {1}]",
            fromFromDate.FormatISODate().ToStdString(),
            toFromDate.FormatISODate().ToStdString());
    }

    wxDateSpan oneDay(0, 0, 0, 1);
    auto& latestPossibleDatePlusOneDay = wxDateTime(pDateStore->SundayDateSeconds).Add(oneDay);
    pToDateCtrl->SetRange(wxDateTime(pDateStore->MondayDateSeconds), latestPossibleDatePlusOneDay);

    wxDateTime toFromDate2 = wxDateTime::Now(), toToDate = wxDateTime::Now();

    if (pToDateCtrl->GetRange(&toFromDate2, &toToDate)) {
        pLogger->info("ExportToCsvDialog::FillControls - pToDateCtrl range is [{0} - {1})",
            toFromDate2.FormatISODate().ToStdString(),
            toToDate.FormatISODate().ToStdString());
    }

    mToLatestPossibleDate = wxDateTime(pDateStore->SundayDateSeconds);

    pFromDateCtrl->SetValue(pDateStore->MondayDateSeconds);

    pLogger->info("ExportToCsvDialog::FillControls - Reset pFromDateCtrl to: {0}",
        pFromDateCtrl->GetValue().FormatISODate().ToStdString());

    mFromCtrlDate = pDateStore->MondayDateSeconds;

    pLogger->info(
        "ExportToCsvDialog::FillControls - Reset mFromCtrlDate to: {0}", mFromCtrlDate.FormatISODate().ToStdString());

    pToDateCtrl->SetValue(pDateStore->SundayDateSeconds);

    pLogger->info("ExportToCsvDialog::FillControls - \npToDateCtrl date = {0}\nSundayDateSeconds = {1}",
        pToDateCtrl->GetValue().FormatISOCombined().ToStdString(),
        date::format("%Y-%m-%d %I:%M:%S %p", date::sys_seconds{ std::chrono::seconds(pDateStore->SundayDateSeconds) }));

    pLogger->info("ExportToCsvDialog::FillControls - Reset pToDateCtrl to: {0}",
        pToDateCtrl->GetValue().FormatISODate().ToStdString());

    mToCtrlDate = pDateStore->SundayDateSeconds;

    pLogger->info(
        "ExportToCsvDialog::FillControls - Reset mToCtrlDate to: {0}", mToCtrlDate.FormatISODate().ToStdString());
}

// clang-format off
void ExportToCsvDialog::ConfigureEventBindings()
{
    pExportToClipboardCheckBoxCtrl->Bind(
        wxEVT_CHECKBOX,
        &ExportToCsvDialog::OnExportToClipboardCheck,
        this
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
} // namespace tks::UI::dlg
