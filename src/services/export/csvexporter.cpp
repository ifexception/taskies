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

#include "csvexporter.h"

#include <cassert>
#include <unordered_map>

#include "exportsservice.h"

namespace tks::Services::Export
{
CsvExporter::CsvExporter(const std::string& databaseFilePath,
    std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsPreview(false)
{
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

bool CsvExporter::GeneratePreview(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    bIsPreview = true;
    mOptions = options;

    pQueryBuilder->IsPreview(true);

    bool success =
        GenerateExport(projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::Generate(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    bIsPreview = false;
    mOptions = options;

    pQueryBuilder->IsPreview(false);

    bool success =
        GenerateExport(projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::GenerateExport(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    std::unordered_map<std::int64_t, Row> rows;
    int rc = -1;

    std::vector<std::string> headers = GetHeadersFromProjections(projections);
    if (headers.empty()) {
        return false;
    }

    const auto& sql = pQueryBuilder->BuildQuery(projections, joinProjections, fromDate, toDate);

    ExportsService exportsService(mDatabaseFilePath, pLogger);

    rc = exportsService.FilterExportCsvData(sql, headers.size(), rows);
    if (rc != 0) {
        return false;
    }

    Data exportData;

    // step 1: build out computed header list
    for (auto& header : headers) {
        exportData.Headers.push_back(header);
    }

    // step 2: attach "main" rows to 'Data' struct
    exportData.Rows = rows;

    if (mOptions.IncludeAttributes) {
        bool attributeAddedSuccessfully = GenerateAttributes(fromDate, toDate, exportData);
        if (!attributeAddedSuccessfully) {
            return false;
        }
    }

    CsvMappedOptions mappedOptions(mOptions);

    CsvExportProcessor exportProcessor(mOptions, mappedOptions);

    std::stringstream exportedData;

    if (!mOptions.ExcludeHeaders) {
        for (auto i = 0; i < exportData.Headers.size(); i++) {
            exportedData << exportData.Headers[i];
            if (i < exportData.Headers.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }
        exportedData << "\n";
    }

    for (const auto& row : exportData.Rows) {
        const auto& rowValues = row.second.Values;
        for (size_t i = 0; i < rowValues.size(); i++) {
            std::string value = rowValues[i];
            exportProcessor.ProcessData(value);
            exportedData << value;

            if (i < rowValues.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    if (!exportedData.good()) {
        return false;
    }

    exportedDataPreview = exportedData.str();
    return true;
}

bool CsvExporter::GenerateAttributes(const std::string& fromDate,
    const std::string& toDate,
    Data& data)
{
    std::optional<std::int64_t> taskId;
    ExportsService exportsService(mDatabaseFilePath, pLogger);
    int rc = -1;
    std::vector<std::string> attributeNames;

    if (bIsPreview) {
        assert(data.Rows.size() == 1);
        for (auto& row : data.Rows) {
            taskId = std::make_optional(row.first);
        }
    }

    const std::string& attributeSql = pQueryBuilder->BuildAttributesQuery(fromDate, toDate, taskId);

    std::unordered_map<std::int64_t, HeaderValueRow> attributeHeaderValueRows;
    rc = exportsService.FilterExportCsvAttributesData(attributeSql, attributeHeaderValueRows);

    if (rc != 0) {
        return false;
    }

    rc = exportsService.GetAttributeNames(fromDate, toDate, taskId, bIsPreview, attributeNames);
    if (rc != 0) {
        return false;
    }
    if (attributeNames.empty()) {
        return true;
    }

    // step 3: append attribute header (name) list to main struct
    // (ordered by ASC name by SQLite)
    for (auto& attributeName : attributeNames) {
        data.Headers.push_back(attributeName);
    }

    // step 4: attach attributes to their index in attributeNames list
    for (auto& [taskIdKey, row] : data.Rows) {
        auto& headerValuePair = attributeHeaderValueRows[taskIdKey].HeaderValuePairs;

        for (size_t i = 0; i < attributeNames.size(); i++) {
            std::string value = "";

            for (auto& headerValue : headerValuePair) {
                if (headerValue.Header == attributeNames[i]) {
                    value = headerValue.Value;
                    break;
                }
            }

            if (!value.empty()) {
                row.Values.push_back(value);
            } else {
                row.Values.push_back("");
            }
        }
    }

    return true;
}

std::vector<std::string> CsvExporter::GetHeadersFromProjections(
    const std::vector<Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> headers;

    for (const auto& projection : projections) {
        const auto& userNamedHeader = projection.ColumnProjection.UserColumn;
        headers.push_back(userNamedHeader);
    }

    return headers;
}
} // namespace tks::Services::Export
