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

#include "projectionkeyvaluepairmodel.h"

#include "../../persistence/exportpersistence.h"

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
    auto success = GenerateExport(
        mOptions, projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::Generate(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    mOptions = options;

    pQueryBuilder->IsPreview(false);
    auto success = GenerateExport(
        mOptions, projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::GenerateExport(CsvExportOptions options,
    const std::vector<Projection>& projections,
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

    Persistence::ExportPersistence exportPersistence(mDatabaseFilePath, pLogger);

    rc = exportPersistence.FilterExportCsvData(sql, headers.size(), rows);
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

    //if (mOptions.IncludeAttributes) {
    //    std::int64_t taskId = -1;
    //    if (bIsPreview) {
    //        assert(unorderedMapValueModels.size() == 1);
    //        for (auto& valueModel : unorderedMapValueModels) {
    //            taskId = valueModel.first;
    //        }
    //    }

    //    const std::string& attributeSql =
    //        pQueryBuilder->BuildAttributesQuery(fromDate, toDate, taskId);

    //    std::unordered_map<std::int64_t, AttributeValueModel> attributeValueModels;
    //    rc = exportPersistence.FilterExportCsvAttributesData(attributeSql, attributeValueModels);

    //    if (rc != 0) {
    //        return false;
    //    }

    //    std::vector<std::string> attributeHeaders;

    //    rc = exportPersistence.GetAttributeHeaderNames(fromDate,
    //        toDate,
    //        std::make_optional(taskId),
    //        pQueryBuilder->IsPreview(),
    //        attributeHeaders);
    //    if (rc != 0) {
    //        return false;
    //    }
    //    std::vector<AttributeHeaderValueModel> attributeRows;

    //    // step 3: build out attribute header list (ordered by SQLite ASC by name)
    //    for (auto& attributeHeader : attributeHeaders) {
    //        exportData.Headers.push_back(attributeHeader);
    //    }

    //    // step 4: attach attributes to their index in attributeHeaders list

    //    for (auto& [taskIdKey, rowValue] : exportData.Rows) {
    //        auto& hvm = attributeValueModels[taskIdKey].HeaderValueModels;

    //        if (false) { // attempt 1
    //            std::vector<std::pair<bool, std::string>> indicesToInsert(attributeHeaders.size());
    //            for (auto& h : hvm) {
    //                for (size_t i = 0; i < attributeHeaders.size(); i++) {
    //                    if (h.Header == attributeHeaders[i]) {
    //                        indicesToInsert[i] = std::make_pair(true, h.Value);
    //                        break;
    //                    }
    //                }
    //            }

    //            for (size_t i = 0; i < indicesToInsert.size(); i++) {
    //                if (indicesToInsert[i].first) {
    //                    rowValue.Values.push_back(indicesToInsert[i].second);
    //                } else {
    //                    rowValue.Values.push_back("");
    //                }
    //            }
    //        }
    //        if (true) { // attempt 2
    //            // swap out lists again (attributeHeaders list first) and use break statement
    //            for (size_t i = 0; i < attributeHeaders.size(); i++) {
    //                std::string v = "";
    //                for (auto& hv : hvm) {
    //                    if (hv.Header == attributeHeaders[i]) {
    //                        v = hv.Value;
    //                        break;
    //                    }
    //                }
    //                if (!v.empty()) {
    //                    rowValue.Values.push_back(v);
    //                } else {
    //                    rowValue.Values.push_back("");
    //                }
    //            }
    //        }
    //    }
    //}

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
