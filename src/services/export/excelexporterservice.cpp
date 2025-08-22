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

#include "excelexporterservice.h"

#include <wx/string.h>

#include "../../utils/utils.h"

namespace tks::Services::Export
{
ExcelExporterService::ExcelExporterService(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool includeAttributes,
    NewLines newLinesOption,
    BooleanHandler booleanHandlerOption)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIncludeAttributes(includeAttributes)
    , pDataExportGenerator(nullptr)
    , mExportDataProcessor(newLinesOption, booleanHandlerOption)
{
    pDataExportGenerator = std::make_unique<DataExportGenerator>(
        pLogger, mDatabaseFilePath, false, bIncludeAttributes);
}

bool ExcelExporterService::ExportToExcel(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    const std::string& saveLocation)
{
    /* `SData` is our main struct to store the headers and rows */
    SData exportData;

    bool success = pDataExportGenerator->GenerateExportData(
        projections, joinProjections, fromDate, toDate, exportData);
    if (!success) {
        pLogger->error("Failed to generate export data. See earlier logs for detail");
        return false;
    }

    /* begin OLE interaction, can take a while to start so wrap operation in a wxBusyCursor */
    wxBusyCursor busyCursor;

    wxAutomationObject excelInstance;
    if (!excelInstance.GetInstance("Excel.Application")) {
        pLogger->error("Could not create Excel object");
        return false;
    }

    if (!excelInstance.PutProperty("DisplayAlerts", false)) {
        pLogger->error("Could not set \"DisplayAlerts\" property");
        return false;
    }

#if TKS_DEBUG
    if (!excelInstance.PutProperty("Visible", true)) {
        // couldn't show it, we don't want to leave the function with hidden instance of Excel still
        // running, so we close it
        excelInstance.CallMethod("Quit");
        wxMessageBox("Succeeded to start Excel, but failed to show it");
        return false;
    }
#endif // TKS_DEBUG

    wxAutomationObject workbooks;
    if (!excelInstance.GetObject(workbooks, "Workbooks")) {
        pLogger->error("Could not obtain Workbooks object");
        excelInstance.CallMethod("Quit");
        return false;
    }

    const wxVariant workbooksCountVariant = workbooks.GetProperty("Count");
    if (workbooksCountVariant.IsNull()) {
        pLogger->error("Could not get workbooks count");
        excelInstance.CallMethod("Quit");
        return false;
    }

    const wxVariant workbookVariant = workbooks.CallMethod("Add");
    if (workbookVariant.IsNull()) {
        pLogger->error("Could not create new Workbook");
        excelInstance.CallMethod("Quit");
        return false;
    }

    wxAutomationObject workbook;
    if (!VariantToObject(workbookVariant, workbook)) {
        pLogger->error("Could not convert variant to workbook object");
        excelInstance.CallMethod("Quit");
        return false;
    }

    wxAutomationObject worksheets;
    if (!workbook.GetObject(worksheets, "Worksheets")) {
        pLogger->error("Could not obtain Worksheets object");
        excelInstance.CallMethod("Quit");
        return false;
    }

    wxAutomationObject worksheet;
    wxVariant v1 = 1L;
    if (!worksheets.GetObject(worksheet, "Item", 1, &v1)) {
        pLogger->error("Could not obtain the first Worksheet object");
        excelInstance.CallMethod("Quit");
        return false;
    }

    /* insert the headers into the first row from our `SData` struct */
    wxAutomationObject rangeObject;
    wxVariant range;
    int headerRowIndex = 1;
    for (size_t i = 0; i < exportData.Headers.size(); i++) {
        std::string excelColumn = Utils::ToExcelColumnName(i + 1);
        range = excelColumn + std::to_string(headerRowIndex);

        if (!worksheet.GetObject(rangeObject, "Range", 1, &range)) {
            pLogger->error("Could not obtain the Range object");
            return false;
        }

        if (!rangeObject.PutProperty("Value", exportData.Headers[i])) {
            pLogger->error("Failed to call PutProperty 'Value'");
            return false;
        }
    }

    /* insert the rows data from our `SData` struct */
    int rowIndex = 2;
    for (auto& [taskId, row] : exportData.Rows) {
        for (size_t i = 0; i < row.Values.size(); i++) {
            std::string excelColumn = Utils::ToExcelColumnName(i + 1);
            range = excelColumn + std::to_string(rowIndex);

            if (!worksheet.GetObject(rangeObject, "Range", 1, &range)) {
                pLogger->error("Could not obtain the Range object");
                return false;
            }

            std::string value = row.Values[i];
            mExportDataProcessor.TryApplyOptionsAndProcessExportData(value);

            if (!rangeObject.PutProperty("Value", value)) {
                pLogger->error("Failed to call PutProperty 'Value'");
                return false;
            }
        }

        rowIndex++;
    }

    /* save the excel to specified location */
    wxVariant filename = saveLocation;
    // https://learn.microsoft.com/en-us/office/vba/api/excel.xlfileformat
    wxVariant fileFormat = xlWorkbookDefaultSaveAs;
    if (!excelInstance.CallMethod("ActiveWorkbook.SaveAs", filename, fileFormat)) {
        pLogger->error("Failed to call 'SaveAs' method");
        excelInstance.CallMethod("Quit");
        return false;
    }

#if !defined(TKS_DEBUG)
    excelInstance.CallMethod("Quit");
#endif // DEBUG

    return true;
}

bool ExcelExporterService::VariantToObject(const wxVariant& v, wxAutomationObject& o)
{
    wxCHECK_MSG(!o.GetDispatchPtr(), false, "o already contains an object");

    if (!v.GetVoidPtr())
        return false;

    o.SetDispatchPtr(v.GetVoidPtr());
    return true;
}
} // namespace tks::Services::Export
