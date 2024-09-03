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

#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../../common/enums.h"
#include "../../services/export/columnjoinprojection.h"
#include "../../services/export/projection.h"
#include "../../services/export/sqliteexportquerybuilder.h"
#include "../../services/export/csvexportoptions.h"

namespace tks::Utils
{
class CsvExportProcessor final
{
public:
    CsvExportProcessor() = delete;
    CsvExportProcessor(const CsvExportProcessor&) = delete;
    CsvExportProcessor(Services::Export::CsvExportOptions options);
    ~CsvExportProcessor() = default;

    const CsvExportProcessor& operator=(const CsvExportProcessor&) = delete;

    void ProcessData(std::stringstream& data, std::string& value);

private:
    void TryProcessNewLines(std::string& value) const;
    void TryProcessEmptyValues(std::string& value) const;
    void TryApplyTextQualifier(std::stringstream& data, std::string& value) const;

    Services::Export::CsvExportOptions mOptions;
};

class CsvExporter
{
public:
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    std ::vector<std::string> ComputeProjectionModel(const std::vector<Services::Export::Projection>& projections);

    bool GeneratePreview(Services::Export::CsvExportOptions options,
        const std::vector<Services::Export::Projection>& projections,
        const std::vector<Services::Export::ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedDataPreview);

private:
    std::shared_ptr<spdlog::logger> pLogger;

    std::unique_ptr<Services::Export::SQLiteExportQueryBuilder> pQueryBuilder;

    Services::Export::CsvExportOptions mOptions;

    std::string mDatabaseFilePath;
};
} // namespace tks::Utils
