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

#include <wx/statline.h>

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
    : pParent(parent)
    , pCfg(cfg)
    , pLogger(logger)
    , mDatabaseFilePath(databasePath)
    , pExportToClipboardCheckBoxCtrl(nullptr)
    , pSaveToFileTextCtrl(nullptr)
    , pBrowseExportPathButton(nullptr)
    , pFromDatePickerCtrl(nullptr)
    , pToDatePickerCtrl(nullptr)
    , mFromCtrlDate()
    , mToCtrlDate()
    , mToLatestPossibleDate()
    , pOKButton(nullptr)
    , pCancelButton(nullptr)
    , pDateStore(nullptr)
{
    pDateStore = std::make_unique<DateStore>(pLogger);

    mFromDate = pDateStore->MondayDate;
    mToDate = pDateStore->SundayDate;

    Create();

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
        dateRangeStaticBox, tksIDC_EXPORTTODAYSTASKSONLYCHECKBOXCTRL, "Export todays tasks only");
    pExportTodaysTasksOnlyCheckBoxCtrl->SetToolTip(
        "If selected, only tasks logged for todays date will be exported");

    dateRangeStaticBoxSizer->Add(
        fromDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pFromDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(
        toDateLabel, wxSizerFlags().Border(wxALL, FromDIP(4)).CenterVertical());
    dateRangeStaticBoxSizer->Add(pToDatePickerCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));
    dateRangeStaticBoxSizer->Add(
        pExportTodaysTasksOnlyCheckBoxCtrl, wxSizerFlags().Border(wxALL, FromDIP(4)));

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

void QuickExportToCsvDialog::FillControls() {}

void QuickExportToCsvDialog::ConfigureEventBindings() {}

void QuickExportToCsvDialog::OnExportToClipboardCheck(wxCommandEvent& event) {}

void QuickExportToCsvDialog::OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event) {}

void QuickExportToCsvDialog::OnFromDateSelection(wxDateEvent& event) {}

void QuickExportToCsvDialog::OnToDateSelection(wxDateEvent& event) {}

void QuickExportToCsvDialog::OnExportTodaysTasksOnlyCheck(wxCommandEvent& event) {}
} // namespace tks::UI::dlg
