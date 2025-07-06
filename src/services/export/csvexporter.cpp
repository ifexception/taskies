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
{
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

std::vector<std::string> CsvExporter::ComputeHeaderModel(const std::vector<Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionMap;

    for (const auto& projection : projections) {
        const auto& columnProjection = projection.ColumnProjection.UserColumn;

        projectionMap.push_back(columnProjection);
    }

    return projectionMap;
}

bool CsvExporter::GeneratePreview(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
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
    const auto& sql = pQueryBuilder->BuildQuery(projections, joinProjections, fromDate, toDate);

    std::vector<std::string> computedHeadersModel = ComputeHeaderModel(projections);

    std::unordered_map<std::int64_t, ValuesModel> unorderedMapValueModels;

    Persistence::ExportPersistence exportPersistence(mDatabaseFilePath, pLogger);

    int rc =
        exportPersistence.FilterExportCsvData(sql, computedHeadersModel, unorderedMapValueModels);

    if (rc != 0) {
        return false;
    }

    Data exportData;

    // step 1: build out computed header list
    for (auto& computedHeader : computedHeadersModel) {
        exportData.Headers.push_back(computedHeader);
    }

    // step 2: build out row values
    for (auto& valueModel : unorderedMapValueModels) {
        Row row;
        for (size_t i = 0; i < valueModel.second.ColumnValueModels.size(); i++) {
            row.Values.push_back(valueModel.second.ColumnValueModels[i].Value);
        }

        exportData.Rows[valueModel.first] = row;
    }

    if (mOptions.IncludeAttributes) {
        std::int64_t taskId = -1;
        if (pQueryBuilder->IsPreview()) {
            assert(unorderedMapValueModels.size() == 1);
            for (auto& valueModel : unorderedMapValueModels) {
                taskId = valueModel.first;
            }
        }

        const std::string& attributeSql =
            pQueryBuilder->BuildAttributesQuery(fromDate, toDate, taskId);

        std::unordered_map<std::int64_t, AttributeValueModel> attributeValueModels;
        rc = exportPersistence.FilterExportCsvAttributesData(attributeSql, attributeValueModels);

        if (rc != 0) {
            return false;
        }

        std::vector<std::string> attributeHeaders;

        rc = exportPersistence.GetAttributeHeaderNames(fromDate,
            toDate,
            std::make_optional(taskId),
            pQueryBuilder->IsPreview(),
            attributeHeaders);
        if (rc != 0) {
            return false;
        }
        std::vector<AttributeHeaderValueModel> attributeRows;

        // step 3: build out attribute header list (ordered by SQLite ASC by name)
        for (auto& attributeHeader : attributeHeaders) {
            exportData.Headers.push_back(attributeHeader);
        }

        // step 4: attach attributes to their index in attributeHeaders list

        for (auto& [taskIdKey, rowValue] : exportData.Rows) {
            auto& hvm = attributeValueModels[taskIdKey].HeaderValueModels;

            if (false) { // attempt 1
                std::vector<std::pair<bool, std::string>> indicesToInsert(attributeHeaders.size());
                for (auto& h : hvm) {
                    for (size_t i = 0; i < attributeHeaders.size(); i++) {
                        if (h.Header == attributeHeaders[i]) {
                            indicesToInsert[i] = std::make_pair(true, h.Value);
                            break;
                        }
                    }
                }

                for (size_t i = 0; i < indicesToInsert.size(); i++) {
                    if (indicesToInsert[i].first) {
                        rowValue.Values.push_back(indicesToInsert[i].second);
                    } else {
                        rowValue.Values.push_back("");
                    }
                }
            }
            if (true) { // attempt 2
                // swap out lists again (attributeHeaders list first) and use break statement
                for (size_t i = 0; i < attributeHeaders.size(); i++) {
                    std::string v = "";
                    for (auto& hv : hvm) {
                        if (hv.Header == attributeHeaders[i]) {
                            v = hv.Value;
                            break;
                        }
                    }
                    if (!v.empty()) {
                        rowValue.Values.push_back(v);
                    } else {
                        rowValue.Values.push_back("");
                    }
                }
            }
        }
    }

    CsvMappedOptions mappedOptions(mOptions);

    CsvExportProcessor exportProcessor(mOptions, mappedOptions);

    std::stringstream exportedData;

    if (!mOptions.ExcludeHeaders) {
        for (auto i = 0; i < computedHeadersModel.size(); i++) {
            exportedData << computedHeadersModel[i];
            if (i < computedHeadersModel.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }
        exportedData << "\n";
    }

    for (const auto& valueModel : unorderedMapValueModels) {
        for (auto i = 0; i < valueModel.second.ColumnValueModels.size(); i++) {
            auto& rowValue = valueModel.second.ColumnValueModels[i];
            std::string value = rowValue.Value;

            exportProcessor.ProcessData(exportedData, value);

            if (i < valueModel.second.ColumnValueModels.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    exportedDataPreview = exportedData.str();
    return true;
}
} // namespace tks::Services::Export
