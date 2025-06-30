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

std::vector<std::string> CsvExporter::ComputeProjectionModel(
    const std::vector<Projection>& projections)
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

    const auto& computedProjectionModel = ComputeProjectionModel(projections);

    std::vector<ProjectionListModel> projectionListModels;

    Persistence::ExportPersistence exportPersistence(mDatabaseFilePath, pLogger);

    int rc = exportPersistence.FilterExportCsvData(sql, computedProjectionModel, projectionListModels);

    if (rc != 0) {
        return false;
    }

    if (mOptions.IncludeAttributes) {
        const std::string& attributeSql = pQueryBuilder->BuildAttributesQuery(fromDate, toDate, -1);
    }

    CsvMappedOptions mappedOptions(mOptions);

    CsvExportProcessor exportProcessor(mOptions, mappedOptions);

    std::stringstream exportedData;

    if (!mOptions.ExcludeHeaders) {
        for (auto i = 0; i < computedProjectionModel.size(); i++) {
            exportedData << computedProjectionModel[i];
            if (i < computedProjectionModel.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }
        exportedData << "\n";
    }

    for (const auto& listModel : projectionListModels) {
        for (auto i = 0; i < listModel.ProjectionKeyValuePairModels.size(); i++) {
            auto& rowValue = listModel.ProjectionKeyValuePairModels[i];
            std::string value = rowValue.Value;

            exportProcessor.ProcessData(exportedData, value);

            if (i < listModel.ProjectionKeyValuePairModels.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    exportedDataPreview = exportedData.str();
    return true;
}
} // namespace tks::Services::Export
