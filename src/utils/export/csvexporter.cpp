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

#include <algorithm>
#include <cctype>

#include "../../common/constants.h"

#include "../../dao/exportdao.h"

#include "../utils.h"

namespace tks::Utils
{
CsvExporter::CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
{
    pQueryBuilder = std::make_unique<Services::Export::SQLiteExportQueryBuilder>(false);
}

std::vector<std::string> CsvExporter::ComputeProjectionModel(
    const std::vector<Services::Export::Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionMap;

    for (const auto& projection : projections) {
        const auto& columnProjection = projection.columnProjection.UserColumn;

        projectionMap.push_back(columnProjection);
    }

    return projectionMap;
}

bool CsvExporter::GeneratePreview(Services::Export::CsvExportOptions options,
    const std::vector<Services::Export::Projection>& projections,
    const std::vector<Services::Export::ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    int rc = -1;
    std::vector<std ::vector<std::pair<std::string, std::string>>> projectionModel;

    mOptions = options;

    pQueryBuilder->IsPreview(true);
    const auto& sql = pQueryBuilder->Build(projections, joinProjections, fromDate, toDate);

    const auto& computedProjectionModel = ComputeProjectionModel(projections);

    DAO::ExportDao exportDao(mDatabaseFilePath, pLogger);
    rc = exportDao.FilterExportCsvData(sql, computedProjectionModel, projectionModel);
    if (rc != 0) {
        return false;
    }

    Services::Export::CsvExportProcessor exportProcessor(mOptions);

    std::stringstream exportedData;

    if (!mOptions.ExcludeHeaders) {
        for (auto i = 0; i < computedProjectionModel.size(); i++) {
            exportedData << computedProjectionModel[i];
            if (i < computedProjectionModel.size() - 1) {
                exportedData << mOptions.Delimiter;
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
                exportedData << mOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    exportedDataPreview = exportedData.str();

    return true;
}
} // namespace tks::Utils
