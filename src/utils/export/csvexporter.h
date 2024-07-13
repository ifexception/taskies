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
#include <unordered_map>

#include <spdlog/logger.h>

namespace tks::Utils
{
class SQLiteExportQueryBuilder;

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

struct ColumnProjection {
    std::string databaseColumn;
    std::string userColumn;
    std::string columnTableName;
    std::string identifier;

    ColumnProjection();
    ColumnProjection(std::string databaseColumn, std::string userColumn, std::string columnTableName);

    void SetIdentifier(const std::string value);
};

struct Projection {
    int orderIndex;
    ColumnProjection columnProjection;

    Projection();
    Projection(int orderIndex, ColumnProjection columnProjection);
};

class CsvExporter
{
public:
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(std::shared_ptr<spdlog::logger> logger, CsvExportOptions options);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    void GeneratePreview(const std::vector<Projection>& projections,
        const std::string& fromDate,
        const std::string& toDate);

private:
    std::shared_ptr<spdlog::logger> pLogger;

    std::unique_ptr<SQLiteExportQueryBuilder> pQueryBuilder;

    CsvExportOptions mOptions;
};

class SQLiteExportQueryBuilder final
{
public:
    SQLiteExportQueryBuilder() = delete;
    SQLiteExportQueryBuilder(const SQLiteExportQueryBuilder&) = delete;
    explicit SQLiteExportQueryBuilder(bool isPreview = false);
    ~SQLiteExportQueryBuilder() = default;

    const SQLiteExportQueryBuilder& operator=(const SQLiteExportQueryBuilder&) = delete;

    const bool IsPreview() const;
    void IsPreview(const bool preview);

    const std::string GetFromDate() const;
    void SetFromDate(const std::string& date);

    const std::string GetToDate() const;
    void SetToDate(const std::string& date);

    std::string Build(const std::vector<Projection>& projections);

private:
    std::string BuildQuery();
    std::string BuildQueryString();
    std::vector<std::string> ComputeProjection(const std::vector<Projection>& projections);
    std::string ComputeSingleProjection(const Projection& projection);

    bool bIsPreview;
    std::string mFromDate;
    std::string mToDate;
};
} // namespace tks::Utils
