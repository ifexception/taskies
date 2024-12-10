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

#include "csvexporter.h"

#include "../../persistence/exportpersistence.h"

namespace tks::Services::Export
{
CsvExporter::CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
{
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

std::vector<std::string> CsvExporter::ComputeProjectionModel(const std::vector<Projection>& projections)
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
    auto success = GenerateExport(mOptions, projections, joinProjections, fromDate, toDate, exportedDataPreview);

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
    auto success = GenerateExport(mOptions, projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::GenerateExport(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    std::vector<std::vector<std::pair<std::string, std::string>>> projectionModel;

    const auto& sql = pQueryBuilder->Build(projections, joinProjections, fromDate, toDate);

    const auto& computedProjectionModel = ComputeProjectionModel(projections);

    Persistence::ExportPersistence exportPersistence(mDatabaseFilePath, pLogger);
    int rc = exportPersistence.FilterExportCsvData(sql, computedProjectionModel, projectionModel);
    if (rc != 0) {
        return false;
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

    for (const auto& rowModel : projectionModel) {
        for (auto i = 0; i < rowModel.size(); i++) {
            auto& rowValue = rowModel[i];
            std::string value = rowValue.second;

            exportProcessor.ProcessData(exportedData, value);

            if (i < rowModel.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    exportedDataPreview = exportedData.str();
    return true;
}
} // namespace tks::Services::Export
