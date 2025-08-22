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

#include "columnjoinprojection.h"
#include "projection.h"
#include "exportoptions.h"
#include "csvexportprocessor.h"
#include "dataexportgenerator.h"

namespace tks::Services::Export
{
struct CsvExporter final {
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(std::shared_ptr<spdlog::logger> logger,
        ExportOptions options,
        const std::string& databaseFilePath,
        bool isPreview);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    bool ExportToCsv(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedData);

    std::shared_ptr<spdlog::logger> pLogger;
    ExportOptions mOptions;
    std::string mDatabaseFilePath;
    std::unique_ptr<DataExportGenerator> pDataExporter;

    bool bIsPreview;
};
} // namespace tks::Services::Export
