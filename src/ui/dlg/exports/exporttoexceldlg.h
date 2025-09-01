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
#include <cstdint>
#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/dataview.h>
#include <wx/listctrl.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "../../dataview/columnlistmodel.h"

#include "../../../core/configuration.h"

#include "../../../utils/datestore.h"

namespace tks::UI::dlg
{
class ExportToExcelDialog : public wxDialog
{
public:
    ExportToExcelDialog() = delete;
    ExportToExcelDialog(const ExportToExcelDialog&) = delete;
    ExportToExcelDialog(wxWindow* parent,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath,
        const wxString& name = "exporttoexceldlg");

    const ExportToExcelDialog& operator=(const ExportToExcelDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();

    void OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event);
    void OnCloseDialogAfterExportingCheck(wxCommandEvent& event);
    void OnOpenExplorerInExportDirectoryCheck(wxCommandEvent& event);

    void OnNewLinesHandlerChoiceSelection(wxCommandEvent& event);
    void OnBooleanHandlerChoiceSelection(wxCommandEvent& event);

    void OnFromDateSelection(wxDateEvent& event);
    void OnToDateSelection(wxDateEvent& event);
    void OnExportTodaysTasksOnlyCheck(wxCommandEvent& event);
    void OnWorkWeekRangeCheck(wxCommandEvent& event);

    void OnSavePreset(wxCommandEvent& event);
    void OnResetPreset(wxCommandEvent& event);
    void OnPresetChoice(wxCommandEvent& event);

    void OnAvailableColumnItemCheck(wxListEvent& event);
    void OnAvailableColumnItemUncheck(wxListEvent& event);
    void OnAddAvailableColumnToExportColumnListView(wxCommandEvent& event);
    void OnRemoveExportColumnToAvailableColumnList(wxCommandEvent& event);
    void OnExportColumnEditingStart(wxDataViewEvent& event);
    void OnExportColumnEditingDone(wxDataViewEvent& event);
    void OnExportColumnSelectionChanged(wxDataViewEvent& event);
    void OnUpButtonSort(wxCommandEvent& event);
    void OnDownButtonSort(wxCommandEvent& event);
    void OnExcludeHeadersCheck(wxCommandEvent& event);
    void OnIncludeAttributesCheck(wxCommandEvent& event);

    void OnExport(wxCommandEvent& event);

    void SetFromAndToDatePickerRanges();
    void SetFromDateAndDatePicker();
    void SetToDateAndDatePicker();

    void ApplyPreset(const Core::Configuration::PresetSettings& presetSettings);

    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    std::unique_ptr<DateStore> pDateStore;

    wxWindow* pParent;

    wxTextCtrl* pSaveToFileTextCtrl;
    wxButton* pBrowseExportPathButton;
    wxCheckBox* pCloseDialogAfterExportingCheckBoxCtrl;
    wxCheckBox* pOpenExplorerInExportDirectoryCheckBoxCtrl;

    wxChoice* pNewLinesHandlerChoiceCtrl;
    wxChoice* pBooleanHanderChoiceCtrl;

    wxDatePickerCtrl* pFromDatePickerCtrl;
    wxDatePickerCtrl* pToDatePickerCtrl;
    wxCheckBox* pExportTodaysTasksCheckBoxCtrl;
    wxCheckBox* pWorkWeekRangeCheckBoxCtrl;

    wxDateTime mFromCtrlDate;
    wxDateTime mToCtrlDate;
    wxDateTime mToLatestPossibleDate;

    wxTextCtrl* pPresetNameTextCtrl;
    wxCheckBox* pPresetIsDefaultCheckBoxCtrl;
    wxButton* pPresetSaveButton;
    wxButton* pPresetResetButton;
    wxChoice* pPresetsChoiceCtrl;

    wxListView* pAvailableColumnsListView;
    wxButton* pRightChevronButton;
    wxButton* pLeftChevronButton;

    wxDataViewCtrl* pDataViewCtrl;
    wxObjectDataPtr<ColumnListModel> pExportColumnListModel;

    wxButton* pUpButton;
    wxButton* pDownButton;

    wxCheckBox* pIncludeAttributesCheckBoxCtrl;

    wxButton* pExportButton;
    wxButton* pCancelButton;

    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate;

    std::vector<long> mSelectedItemIndexes;
    wxDataViewItem mItemToSort;

    bool bOpenExplorerInExportDirectory;
    bool bExportTodaysTasksOnly;
    bool bIncludeAttributes;

    NewLines mNewLinesOption;
    BooleanHandler mBooleanOption;

    enum {
        tksIDC_SAVETOFILETEXTCTRL = wxID_HIGHEST + 100,
        tksIDC_BROWSEEXPORTPATHBUTTON,
        tksIDC_CLOSEDIALOGAFTEREXPORTINGCHECKBOXCTRL,
        TKSIDC_OPENEXPLORERINEXPORTDIRECTORYCHECKBOXCTRL,
        tksIDC_NEW_LINES_HANDLER_CTRL,
        tksIDC_BOOLEAN_HANDLER_CTRL,
        tksIDC_FROMDATEPICKERCTRL,
        tksIDC_TODATEPICKERCTRL,
        tksIDC_EXPORTTODAYSTASKSCHECKBOXCTRL,
        tksIDC_WORKWEEKRANGECHECKBOXCTRL,
        tksIDC_PRESETNAMETEXTCTRL,
        tksIDC_PRESETISDEFAULTCHECKBOXCTRL,
        tksIDC_PRESETSAVEBUTTON,
        tksIDC_PRESETRESETBUTTON,
        tksIDC_PRESETCHOICECTRL,
        tksIDC_AVAILABLECOLUMNSLISTVIEW,
        tksIDC_RIGHTCHEVRONBUTTON,
        tksIDC_LEFTCHEVRONBUTTON,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL,
        tksIDC_UPBUTTON,
        tksIDC_DOWNBUTTON,
        tksIDC_INCLUDEATTRIBUTESCHECKBOXCTRL,
        tksIDC_EXPORTBUTTON,
    };
};
} // namespace tks::UI::dlg
