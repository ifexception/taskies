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

#include <spdlog/logger.h>

namespace tks::Utils
{
enum class EndOfLine : int { Windows = 1, Macintosh, Linux };
enum class EmptyValues : int { Blank = 1, Null };
enum class NewLines : int { Preserve = 1, Merge };

struct CsvExportOptions {
    char Delimiter;
    char TextQualifier;
    EndOfLine EolTerminator;
    EmptyValues EmptyValuesHandler;
    NewLines NewLinesHandler;

    CsvExportOptions();
    ~CsvExportOptions() = default;
};

class DatabaseExportQueryBuilder
{
public:
    DatabaseExportQueryBuilder();
    ~DatabaseExportQueryBuilder() = default;

    DatabaseExportQueryBuilder& WithEmployerName();
    DatabaseExportQueryBuilder& WithClientName();
    DatabaseExportQueryBuilder& WithProjectName();
    DatabaseExportQueryBuilder& WithProjectDisplayName();
    DatabaseExportQueryBuilder& WithCategoryName();
    DatabaseExportQueryBuilder& WithDate();
    DatabaseExportQueryBuilder& WithTaskDescription();
    DatabaseExportQueryBuilder& WithBillable();
    DatabaseExportQueryBuilder& WithUniqueId();
    DatabaseExportQueryBuilder& WithTime();
    DatabaseExportQueryBuilder& WithDateRange(const std::string& fromDate, const std::string& toDate);

    std::string Build();

private:
    std::stringstream mSelectQuery;
    std::stringstream mFromQuery;
    std::stringstream mJoinsQuery;
    std::stringstream mWhereQuery;

    const char newline = '\n';
    const char comma = ',';
};

class CsvExporter
{
public:
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(std::shared_ptr<spdlog::logger> logger, CsvExportOptions options);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    void GeneratePreview();

private:
    std::shared_ptr<spdlog::logger> pLogger;

    CsvExportOptions mOptions;
};
} // namespace tks::Utils
