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

#include <memory>
#include <string>
#include <vector>

#include <spdlog/logger.h>

#include "../../common/enums.h"

#include "columnjoinprojection.h"
#include "projection.h"
#include "sqliteexportquerybuilder.h"
#include "csvexportoptions.h"
#include "csvexportprocessor.h"

namespace tks::Services::Export
{
class CsvExporter
{
public:
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    std ::vector<std::string> ComputeHeaderModel(const std::vector<Projection>& projections);

    bool GeneratePreview(CsvExportOptions options,
        const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedDataPreview);

    bool Generate(CsvExportOptions options,
        const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedDataPreview);

private:
    bool GenerateExport(CsvExportOptions options,
        const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedDataPreview);

    std::shared_ptr<spdlog::logger> pLogger;

    std::unique_ptr<SQLiteExportQueryBuilder> pQueryBuilder;

    CsvExportOptions mOptions;

    std::string mDatabaseFilePath;
};
} // namespace tks::Services::Export
