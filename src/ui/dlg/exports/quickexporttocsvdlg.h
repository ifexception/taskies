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

#pragma once

#include <chrono>
#include <memory>

#include <date/date.h>

#include <spdlog/logger.h>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/datectrl.h>
#include <wx/dateevt.h>

#include "../../../common/common.h"

#include "../../../core/configuration.h"

#include "../../../services/export/csvexportoptions.h"

#include "../../../utils/datestore.h"

namespace tks::UI::dlg
{
class QuickExportToCsvDialog : public wxDialog
{
public:
    QuickExportToCsvDialog() = delete;
    QuickExportToCsvDialog(const QuickExportToCsvDialog&) = delete;
    QuickExportToCsvDialog(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath,
        const wxString& name = "quickexporttocsvdlg");

    const QuickExportToCsvDialog& operator=(const QuickExportToCsvDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();

    void OnExportToClipboardCheck(wxCommandEvent& event);
    void OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event);

    void OnFromDateSelection(wxDateEvent& event);
    void OnToDateSelection(wxDateEvent& event);
    void OnExportTodaysTasksCheck(wxCommandEvent& event);
    void OnWorkWeekRangeCheck(wxCommandEvent& event);

    void OnPresetChoiceSelection(wxCommandEvent& event);

    void OnOK(wxCommandEvent& event);

    void SetFromAndToDatePickerRanges();
    void SetFromDateAndDatePicker();
    void SetToDateAndDatePicker();

    void ApplyPreset(Core::Configuration::PresetSettings& presetSettings);

    wxWindow* pParent;

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    std::unique_ptr<DateStore> pDateStore;

    wxCheckBox* pExportToClipboardCheckBoxCtrl;
    wxTextCtrl* pSaveToFileTextCtrl;
    wxButton* pBrowseExportPathButton;

    wxDatePickerCtrl* pFromDatePickerCtrl;
    wxDatePickerCtrl* pToDatePickerCtrl;
    wxCheckBox* pExportTodaysTasksCheckBoxCtrl;
    wxCheckBox* pWorkWeekRangeCheckBoxCtrl;

    wxDateTime mFromCtrlDate;
    wxDateTime mToCtrlDate;
    wxDateTime mToLatestPossibleDate;

    wxChoice* pPresetsChoiceCtrl;

    wxButton* pOKButton;
    wxButton* pCancelButton;

    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate;

    bool bExportToClipboard;
    bool bExportTodaysTasksOnly;

    Services::Export::CsvExportOptions mCsvOptions;

    enum {
        tksIDC_COPY_TO_CLIPBOARD_CTRL = wxID_HIGHEST + 100,
        tksIDC_SAVE_TO_FILE_CTRL,
        tksIDC_BROWSE_EXPORT_PATH_CTRL,
        tksIDC_DATE_FROM_CTRL,
        tksIDC_DATE_TO_CTRL,
        tksIDC_EXPORTTODAYSTASKSCHECKBOXCTRL,
        tksIDC_WORKWEEKRANGECHECKBOXCTRL,
        tksIDC_PRESET_CHOICE_CTRL,
    };
};
} // namespace tks::UI::dlg
