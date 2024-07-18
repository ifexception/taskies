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
    bool ExcludeHeaders;

    CsvExportOptions();
    ~CsvExportOptions() = default;
};

enum class JoinType { InnerJoin = 1, LeftJoin = 2 };

struct FirstLevelJoinTable {
    std::string tableName;
    JoinType joinType;
    std::string idColumn;
};

struct SecondLevelJoinTable {
    std::string tableName;
    JoinType joinType;
    std::string idColumn;
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
    CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger);
    ~CsvExporter() = default;

    const CsvExporter& operator=(const CsvExporter&) = delete;

    std ::vector<std::string> ComputeProjectionModel(const std::vector<Projection>& projections);

    bool GeneratePreview(CsvExportOptions options, const std::vector<Projection>& projections,
        const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
        const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
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
        const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
        const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
        const std::string& fromDate,
        const std::string& toDate);

private:
    std::string BuildQuery(const std::vector<Projection>& projections,
        const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
        const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
        const std::string& fromDate,
        const std::string& toDate);

    std::string BuildQueryString(const std::vector<std::string>& columns,
        const std::vector<std::string>& firstLevelJoins,
        const std::vector<std::string>& secondLevelJoins,
        const std::string& where);

    std::vector<std::string> ComputeFirstLevelJoins(const std::vector<FirstLevelJoinTable>& joinTables);
    std::string ComputeFirstLevelJoin(const FirstLevelJoinTable& joinTable);

    std::vector<std::string> ComputeSecondLevelJoins(const std::vector<SecondLevelJoinTable>& joinTables);
    std::string ComputeSecondLevelJoin(const SecondLevelJoinTable& joinTable);

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
