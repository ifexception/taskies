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

#include <fmt/format.h>

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
    , pDataGenerator(nullptr)
    , mExportDataProcessor(newLinesOption, booleanHandlerOption)
{
    pDataGenerator =
        std::make_unique<DataGenerator>(pLogger, mDatabaseFilePath, false, bIncludeAttributes);
}

ExportResult ExcelExporterService::ExportToExcel(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    const std::string& saveLocation)
{
    /* `SData` is our main struct to store the headers and rows */
    SData exportData;

    auto result =
        pDataGenerator->FillData(projections, joinProjections, fromDate, toDate, exportData);
    if (!result.Success) {
        pLogger->error("Failed to generate export data. See earlier logs for detail");
        return result;
    }

    /* begin OLE interaction, can take a while to start so wrap operation in a wxBusyCursor */
    wxBusyCursor busyCursor;

    wxAutomationObject excelInstance;
    if (!excelInstance.GetInstance("Excel.Application")) {
        pLogger->error("Could not create Excel object");
        return ExportResult::Fail("Failed to open Excel application");
    }

    if (!excelInstance.PutProperty("DisplayAlerts", false)) {
        excelInstance.CallMethod("Quit");
        pLogger->error("Could not set \"DisplayAlerts\" property");
        return ExportResult::Fail("Failed to toggle off \"DisplayAlert\" property");
    }

#if TKS_DEBUG
    if (!excelInstance.PutProperty("Visible", true)) {
        // couldn't show it, we don't want to leave the function with hidden instance of Excel still
        // running, so we close it
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Succeeded to start Excel, but failed to show it");
    }
#endif // TKS_DEBUG

    wxAutomationObject workbooks;
    if (!excelInstance.GetObject(workbooks, "Workbooks")) {
        pLogger->error("Could not obtain Workbooks object");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to obtain Excel \"Workbooks\" object");
    }

    const wxVariant workbooksCountVariant = workbooks.GetProperty("Count");
    if (workbooksCountVariant.IsNull()) {
        pLogger->error("Could not get workbooks count");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to get count of workbooks");
    }

    const wxVariant workbookVariant = workbooks.CallMethod("Add");
    if (workbookVariant.IsNull()) {
        pLogger->error("Could not create new Workbook");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to create new \"Workbook\"");
    }

    wxAutomationObject workbook;
    if (!VariantToObject(workbookVariant, workbook)) {
        pLogger->error("Could not convert variant to workbook object");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Conversion error occurred");
    }

    wxAutomationObject worksheets;
    if (!workbook.GetObject(worksheets, "Worksheets")) {
        pLogger->error("Could not obtain Worksheets object");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to obtain Excel \"Worksheets\" object");
    }

    wxAutomationObject worksheet;
    wxVariant v1 = 1L;
    if (!worksheets.GetObject(worksheet, "Item", 1, &v1)) {
        pLogger->error("Could not obtain the first Worksheet object");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to obtain first \"Worksheet\" object");
    }

    /* write a 2D array */
    std::vector<std::vector<std::string>> excelData;

    // Add headers as first row
    std::vector<std::string> headerRow;
    for (const auto& header : exportData.Headers) {
        headerRow.push_back(header);
    }
    excelData.push_back(headerRow);

    // Add values
    for (const auto& [taskId, row] : exportData.Rows) {
        std::vector<std::string> excelRow;
        for (const auto& value : row.Values) {
            std::string processedValue = value;
            mExportDataProcessor.TryApplyOptionsAndProcessExportData(processedValue);
            excelRow.push_back(processedValue);
        }
        excelData.push_back(excelRow);
    }

    for (size_t i = 0; i < excelData.size(); ++i) {
        for (size_t j = 0; j < excelData[i].size(); ++j) {
            SPDLOG_LOGGER_TRACE(pLogger, "Cells[{0}][{1}]", (long) i + 1, (long) j + 1);
            wxVariant iVar = (long) i + 1;
            wxVariant jVar = (long) j + 1;

            wxAutomationObject cellObject;
            wxVariant cellVar = worksheet.GetProperty("Cells", iVar, jVar);
            if (!VariantToObject(cellVar, cellObject)) {
                pLogger->error("Could not get property Cells");
                excelInstance.CallMethod("Quit");
                return ExportResult::Fail("Failed to get Cells property of Worksheet");
            }

            if (!cellObject.PutProperty("Value", excelData[i][j])) {
                pLogger->error("Failed to set property Cells(x,y).Value");
                excelInstance.CallMethod("Quit");
                return ExportResult::Fail(
                    fmt::format("Failed to set property Cells[{0}][{1}].Value with value \"{3}\"",
                        i,
                        j,
                        excelData[i][j]));
            }
        }
    }

    /* save the excel to specified location */
    wxVariant filename = saveLocation;
    // https://learn.microsoft.com/en-us/office/vba/api/excel.xlfileformat
    wxVariant fileFormat = xlWorkbookDefaultSaveAs;
    if (!excelInstance.CallMethod("ActiveWorkbook.SaveAs", filename, fileFormat)) {
        pLogger->error("Failed to call 'SaveAs' method");
        excelInstance.CallMethod("Quit");
        return ExportResult::Fail("Failed to save Excel file in \"" + saveLocation + "\"");
    }

#if !defined(TKS_DEBUG)
    excelInstance.CallMethod("Quit");
#endif // DEBUG

    return ExportResult::OK();
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
