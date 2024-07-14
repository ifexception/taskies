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

namespace tks::Utils
{
CsvExportOptions::CsvExportOptions()
    : Delimiter(',')
    , TextQualifier('"')
    , EolTerminator(EndOfLine::Windows)
    , EmptyValuesHandler(EmptyValues::Blank)
    , NewLinesHandler(NewLines::Merge)
{
}

CsvExporter::CsvExporter(std::shared_ptr<spdlog::logger> logger, CsvExportOptions options)
    : pLogger(logger)
    , mOptions(options)
{
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

void CsvExporter::GeneratePreview(const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate)
{
    pQueryBuilder->IsPreview(true);
    std::string sql = pQueryBuilder->Build(projections, firstLevelJoinTables, secondLevelJoinTables, fromDate, toDate);
}

// #####################################################################################################################

ColumnProjection::ColumnProjection()
    : databaseColumn("")
    , userColumn("")
    , columnTableName("")
    , identifier("")
{
}

ColumnProjection::ColumnProjection(std::string databaseColumn, std::string userColumn, std::string columnTableName)
    : databaseColumn(databaseColumn)
    , userColumn(userColumn)
    , columnTableName(columnTableName)
{
}

void ColumnProjection::SetIdentifier(const std::string value)
{
    identifier = value;
}

Projection::Projection()
    : orderIndex(-1)
    , columnProjection()
{
}

Projection::Projection(int orderIndex, ColumnProjection columnProjection)
    : orderIndex(orderIndex)
    , columnProjection(columnProjection)
{
}

SQLiteExportQueryBuilder::SQLiteExportQueryBuilder(bool isPreview)
    : bIsPreview(isPreview)
{
}

const bool SQLiteExportQueryBuilder::IsPreview() const
{
    return bIsPreview;
}

void SQLiteExportQueryBuilder::IsPreview(const bool preview)
{
    bIsPreview = preview;
}

std::string SQLiteExportQueryBuilder::Build(const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate)
{
    return BuildQuery(projections, firstLevelJoinTables, secondLevelJoinTables, fromDate, toDate);
}

std::string SQLiteExportQueryBuilder::BuildQuery(const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate)
{
    const auto& columns = ComputeProjections(projections);
    const auto& firstLevelJoins = ComputeFirstLevelJoins(firstLevelJoinTables);
    const auto& secondLevelJoins = ComputeSecondLevelJoins(secondLevelJoinTables);
    const auto& where = BuildWhere(fromDate, toDate);

    std::string query = BuildQueryString(columns, firstLevelJoins, secondLevelJoins, where);
    return query;
}

std::string SQLiteExportQueryBuilder::BuildQueryString(const std::vector<std::string>& columns,
    const std::vector<std::string>& firstLevelJoins,
    const std::vector<std::string>& secondLevelJoins,
    const std::string& where)
{
    std::stringstream query;

    query << "SELECT ";
    AppendColumns(query, columns);

    query << "FROM tasks ";
    if (!firstLevelJoins.empty()) {
        AppendJoins(query, firstLevelJoins);
    }

    if (!secondLevelJoins.empty()) {
        AppendJoins(query, secondLevelJoins);
    }

    AppendClause(query, " WHERE ", where);

    if (bIsPreview) {
        AppendClause(query, " LIMIT ", "1");
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeFirstLevelJoins(
    const std::vector<FirstLevelJoinTable>& joinTables)
{
    if (joinTables.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinTable : joinTables) {
        std::string joinStatement = ComputeFirstLevelJoin(joinTable);
        computedJoins.push_back(joinStatement);
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeFirstLevelJoin(const FirstLevelJoinTable& joinTable)
{
    std::stringstream query;
    if (joinTable.joinType == JoinType::InnerJoin) {
        query << "INNER JOIN " << joinTable.tableName << "ON "
              << "tasks"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeSecondLevelJoins(
    const std::vector<SecondLevelJoinTable>& joinTables)
{
    if (joinTables.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinTable : joinTables) {
        std::string joinStatement = ComputeSecondLevelJoin(joinTable);
        computedJoins.push_back(joinStatement);
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeSecondLevelJoin(const SecondLevelJoinTable& joinTable)
{
    std::stringstream query;
    if (joinTable.joinType == JoinType::InnerJoin) {
        query << "INNER JOIN " << joinTable.tableName << "ON "
              << "projects"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }
    if (joinTable.joinType == JoinType::LeftJoin) {
        query << "LEFT JOIN " << joinTable.tableName << "ON "
              << "projects"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeProjections(const std::vector<Projection>& projections)
{
    if (projections.size() == 0) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionsOut;

    for (const auto& projection : projections) {
        std::string projectionOut = ComputeSingleProjection(projection);
        projectionsOut.push_back(projectionOut);
    }

    return projectionsOut;
}

std::string SQLiteExportQueryBuilder::ComputeSingleProjection(const Projection& projection)
{
    std::stringstream query;
    ColumnProjection cp = projection.columnProjection;

    if (!cp.userColumn.empty() && cp.identifier.empty()) {
        query << cp.columnTableName << "." << cp.databaseColumn << " AS " << cp.userColumn;
    } else if (!cp.userColumn.empty() && !cp.identifier.empty()) {
        query << "(printf('%02d'," << cp.columnTableName << ".hours)"
              << " || "
              << "':'"
              << " || "
              << "(printf('%02d'," << cp.columnTableName << ".minutes)"
              << " AS " << cp.userColumn;
    } else if (cp.userColumn.empty() && !cp.identifier.empty()) {
        query << "(printf('%02d'," << cp.columnTableName << ".hours)"
              << " || "
              << "':'"
              << " || "
              << "(printf('%02d'," << cp.columnTableName << ".minutes)"
              << " AS "
              << "Duration";
    } else {
        query << cp.columnTableName << "." << cp.databaseColumn;
    }

    return query.str();
}
std::string SQLiteExportQueryBuilder::BuildWhere(const std::string& fromDate, const std::string& toDate)
{
    if (fromDate.empty() || toDate.empty()) {
        return std::string();
    }

    std::stringstream whereClause;

    whereClause << "workdays.date"
                << " >= " << fromDate << " AND "
                << "workdays.date"
                << " <= " << toDate << " ";

    return whereClause.str();
}

void SQLiteExportQueryBuilder::AppendColumns(std::stringstream& query, const std::vector<std::string>& columns)
{
    for (const auto& column : columns) {
        if (!column.empty()) {
            query << column << ", ";
        }
    }
    query << " ";
}

void SQLiteExportQueryBuilder::AppendJoins(std::stringstream& query, const std::vector<std::string>& joins)
{
    for (const auto& join : joins) {
        if (!join.empty()) {
            query << join << " ";
        }
    }

    query << " ";
}

void SQLiteExportQueryBuilder::AppendClause(std::stringstream& query, std::string name, std::string clause)
{
    if (!clause.empty()) {
        query << name << clause;
    }
}
} // namespace tks::Utils
