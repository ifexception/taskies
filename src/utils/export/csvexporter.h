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

namespace tks::Utils
{
class SQLiteExportQueryBuilder;

struct CsvExportOptions {
    char Delimiter;
    char TextQualifier;
    EmptyValues EmptyValuesHandler;
    NewLines NewLinesHandler;
    bool ExcludeHeaders;

    CsvExportOptions();
    ~CsvExportOptions() = default;

    void Reset();
};

struct ColumnJoinProjection {
    std::string TableName;
    std::string IdColumn;
    JoinType Join;
    bool IsSecondLevelJoin;

    ColumnJoinProjection();
    ColumnJoinProjection(std::string tableName, std::string idColumn, bool isSecondLevelJoin = false);
    ColumnJoinProjection(std::string tableName, std::string idColumn, JoinType join, bool isSecondLevelJoin = false);
};

struct ColumnProjection {
    std::string DatabaseColumn;
    std::string UserColumn;
    std::string IdColumn;
    std::string TableName;
    std::string SpecialIdentifierForDurationColumns;

    ColumnProjection();
    ColumnProjection(std::string databaseColumn, std::string userColumn, std::string idColumn, std::string tableName);

    void SetSpecialIdentifierForDurationColumns(const std::string value);
};

struct Projection {
    int orderIndex;
    ColumnProjection columnProjection;

    Projection();
    Projection(int orderIndex, ColumnProjection columnProjection);
};

class CsvExportProcessor final
{
public:
    CsvExportProcessor() = delete;
    CsvExportProcessor(const CsvExportProcessor&) = delete;
    CsvExportProcessor(CsvExportOptions options);
    ~CsvExportProcessor() = default;

    const CsvExportProcessor& operator=(const CsvExportProcessor&) = delete;

    void ProcessData(std::stringstream& data, std::string& value);

private:
    void TryProcessNewLines(std::string& value) const;
    void TryProcessEmptyValues(std::string& value) const;
    void TryApplyTextQualifier(std::stringstream& data, std::string& value) const;

    CsvExportOptions mOptions;
};

class CsvExporter
{
public:
    CsvExporter() = delete;
    CsvExporter(const CsvExporter&) = delete;
    CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    std ::vector<std::string> ComputeProjectionModel(const std::vector<Projection>& projections);

    bool GeneratePreview(CsvExportOptions options,
        const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ std::string& exportedDataPreview);

private:
    std::shared_ptr<spdlog::logger> pLogger;

    std::unique_ptr<SQLiteExportQueryBuilder> pQueryBuilder;

    CsvExportOptions mOptions;

    std::string mDatabaseFilePath;
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

    std::string Build(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate);

private:
    std::string BuildQuery(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate);

    std::string BuildQueryString(const std::vector<std::string>& columns,
        const std::vector<std::string>& firstLevelJoins,
        const std::vector<std::string>& secondLevelJoins,
        const std::string& where);

    std::vector<std::string> ComputeFirstLevelJoinProjections(const std::vector<ColumnJoinProjection>& joinProjections);
    std::string ComputeFirstLevelJoinProjection(const ColumnJoinProjection& joinProjection);

    std::vector<std::string> ComputeSecondLevelJoinProjections(
        const std::vector<ColumnJoinProjection>& joinProjections);
    std::string ComputeSecondLevelJoinProjection(const ColumnJoinProjection& joinProjection);

    std::vector<std::string> ComputeProjections(const std::vector<Projection>& projections);
    std::string ComputeSingleProjection(const Projection& projection);

    std::string BuildWhere(const std::string& fromDate, const std::string& toDate);

    void AppendColumns(std::stringstream& query, const std::vector<std::string>& columns);
    void AppendJoins(std::stringstream& query, const std::vector<std::string>& joins);
    void AppendClause(std::stringstream& query, std::string name, std::string clause);

    bool bIsPreview;
};

class ExportDao final
{
public:
    ExportDao() = delete;
    ExportDao(const ExportDao&) = delete;
    explicit ExportDao(const std::string& databaseFilePath, const std::shared_ptr<spdlog::logger> logger);
    ~ExportDao();

    const ExportDao& operator=(const ExportDao&) = delete;

    int FilterExportData(const std::string& sql,
        const std::vector<std::string>& projectionMap,
        /*out*/ std::vector<std ::vector<std::pair<std::string, std::string>>>& projectionModel);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;
};
} // namespace tks::Utils
