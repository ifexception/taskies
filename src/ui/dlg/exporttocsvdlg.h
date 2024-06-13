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

#include "../dataview/exportheaderslistmodel.h"

#include "../../utils/datestore.h"

namespace tks
{
namespace Core
{
class Environment;
class Configuration;
} // namespace Core
namespace UI::dlg
{
class ExportToCsvDialog final : public wxDialog
{
public:
    ExportToCsvDialog() = delete;
    ExportToCsvDialog(const ExportToCsvDialog&) = delete;
    ExportToCsvDialog(wxWindow* parent,
        std::shared_ptr<Core::Environment> env,
        std::shared_ptr<Core::Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger,
        const std::string& databasePath,
        const wxString& name = "exporttocsvdlg");

    const ExportToCsvDialog& operator=(const ExportToCsvDialog&) = delete;

private:
    void Create();

    void CreateControls();
    void FillControls();
    void ConfigureEventBindings();

    void OnExportToClipboardCheck(wxCommandEvent& event);
    void OnOpenDirectoryForSaveToFileLocation(wxCommandEvent& event);

    void OnFromDateSelection(wxDateEvent& event);
    void OnToDateSelection(wxDateEvent& event);

    void OnAvailableHeaderItemCheck(wxListEvent& event);
    void OnAvailableHeaderItemUncheck(wxListEvent& event);
    void OnAddAvailableHeaderToExportHeaderList(wxCommandEvent& event);
    void OnRemoveExportHeaderToAvailableHeaderList(wxCommandEvent& event);

    void OnExportHeaderEditingStart(wxDataViewEvent& event);
    void OnExportHeaderEditingDone(wxDataViewEvent& event);

    void SetFromAndToDatePickerRanges();
    void SetFromDateAndDatePicker();
    void SetToDateAndDatePicker();

    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<Core::Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;

    std::unique_ptr<DateStore> pDateStore;

    wxWindow* pParent;

    wxChoice* pDelimiterChoiceCtrl;
    wxChoice* pTextQualifierChoiceCtrl;
    wxChoice* pEolTerminatorChoiceCtrl;
    wxChoice* pEmptyValueHandlerChoiceCtrl;
    wxChoice* pNewLinesHandlerChoiceCtrl;
    wxCheckBox* pRemoveCommasCheckBoxCtrl;

    wxCheckBox* pExportToClipboardCheckBoxCtrl;
    wxTextCtrl* pSaveToFileTextCtrl;
    wxButton* pBrowseExportPathButton;

    wxDatePickerCtrl* pFromDateCtrl;
    wxDatePickerCtrl* pToDateCtrl;

    wxDateTime mFromCtrlDate;
    wxDateTime mToCtrlDate;
    wxDateTime mToLatestPossibleDate;

    wxListView* pDefaultHeadersListView;
    wxButton* pRightChevronButton;
    wxButton* pLeftChevronButton;

    wxDataViewCtrl* pDataViewCtrl;
    wxObjectDataPtr<ExportHeadersListModel> pExportHeaderListModel;

    wxButton* pUpButton;
    wxButton* pDownButton;

    wxCheckBox* pExcludeHeadersCheckBoxCtrl;

    wxTextCtrl* pDataExportPreviewTextCtrl;

    wxButton* pExportButton;
    wxButton* pCancelButton;

    std::chrono::time_point<std::chrono::system_clock, date::days> mFromDate;
    std::chrono::time_point<std::chrono::system_clock, date::days> mToDate;

    std::vector<long> mSelectedItemIndexes;

    enum {
        tksIDC_DELIMITER_CTRL = wxID_HIGHEST + 100,
        tksIDC_TEXT_QUALIFIER_CTRL,
        tksIDC_EOL_TERMINATOR_CTRL,
        tksIDC_EMPTY_VALUE_HANDLER_CTRL,
        tksIDC_NEW_LINES_HANDLER_CTRL,
        tksIDC_REMOVE_COMMAS_CTRL,
        tksIDC_COPY_TO_CLIPBOARD_CTRL,
        tksIDC_SAVE_TO_FILE_CTRL,
        tksIDC_BROWSE_EXPORT_PATH_CTRL,
        tksIDC_DATE_FROM_CTRL,
        tksIDC_DATE_TO_CTRL,
        tksIDC_DEFAULT_HEADERS_LISTVIEW_CTRL,
        tksIDC_RIGHT_CHEV_CTRL,
        tksIDC_LEFT_CHEV_CTRL,
        tksIDC_EXPORT_HEADERS_DATAVIEW_CTRL,
        tksIDC_UP_BUTTON,
        tksIDC_DOWN_BUTTON,
        tksIDC_EXCLUDE_HEADERS_CTRL,
        tksIDC_DATA_EXPORT_PREVIEW_CTRL,
        tksIDC_EXPORT_BUTTON,
    };
};
} // namespace UI::dlg
} // namespace tks
