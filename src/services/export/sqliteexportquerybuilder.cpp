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

#include "sqliteexportquerybuilder.h"

namespace tks::Services::Export
{
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
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate)
{
    return BuildQuery(projections, joinProjections, fromDate, toDate);
}

std::string SQLiteExportQueryBuilder::BuildQuery(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate)
{
    const auto& columns = ComputeProjections(projections);
    const auto& firstLevelJoins = ComputeFirstLevelJoinProjections(joinProjections);
    const auto& secondLevelJoins = ComputeSecondLevelJoinProjections(joinProjections);
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
    query << "INNER JOIN workdays ";
    query << "ON tasks.workday_id = workdays.workday_id ";

    if (!firstLevelJoins.empty()) {
        AppendJoins(query, firstLevelJoins);
    }

    if (!secondLevelJoins.empty()) {
        query << " ";
        AppendJoins(query, secondLevelJoins);
    }

    AppendClause(query, " WHERE ", where);

    if (bIsPreview) {
        AppendClause(query, " LIMIT ", "1");
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeFirstLevelJoinProjections(
    const std::vector<ColumnJoinProjection>& joinProjections)
{
    if (joinProjections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinProjection : joinProjections) {
        if (!joinProjection.IsSecondLevelJoin) {
            std::string joinStatement = ComputeFirstLevelJoinProjection(joinProjection);
            computedJoins.push_back(joinStatement);
        }
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeFirstLevelJoinProjection(const ColumnJoinProjection& joinProjection)
{
    std::stringstream query;
    if (joinProjection.Join == JoinType::InnerJoin) {
        // clang-format off
        query
            << "INNER JOIN "
            << joinProjection.TableName
            << " ON "
            << "tasks"
            << "."
            << joinProjection.IdColumn
            << " = "
            << joinProjection.TableName
            << "."
            << joinProjection.IdColumn;
        // clang-format on
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeSecondLevelJoinProjections(
    const std::vector<ColumnJoinProjection>& joinProjections)
{
    if (joinProjections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinTable : joinProjections) {
        if (joinTable.IsSecondLevelJoin) {
            auto joinStatement = ComputeSecondLevelJoinProjection(joinTable);
            computedJoins.push_back(joinStatement);
        }
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeSecondLevelJoinProjection(const ColumnJoinProjection& joinProjection)
{
    std::stringstream query;

    if (joinProjection.Join == JoinType::InnerJoin) {
        query << "INNER JOIN ";
    }
    if (joinProjection.Join == JoinType::LeftJoin) {
        query << "LEFT JOIN ";
    }

    // clang-format off
    query
        << joinProjection.TableName
        << " ON "
        << "projects"
        << "."
        << joinProjection.IdColumn
        << " = " << joinProjection.TableName
        << "."
        << joinProjection.IdColumn;
    // clang-format on

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
    SColumnProjection cp = projection.ColumnProjection;

    if (cp.Field == FieldType::Default) {
        if (!cp.UserColumn.empty()) {
            // clang-format off
            query
                << cp.TableName
                << "."
                << cp.DatabaseColumn
                << " AS "
                << "\""
                << cp.UserColumn
                << "\"";
            // clang-format on
        } else {
            query << cp.TableName << "." << cp.DatabaseColumn;
        }
    } else if (cp.Field == FieldType::Formatted) {
        if (cp.SpecialIdentifierForDurationColumns.empty()) {
            if (!cp.UserColumn.empty()) {
                // clang-format off
                query
                    << "("
                    << "printf('%02d', "
                    << cp.TableName
                    << "."
                    << cp.DatabaseColumn
                    << ")"
                    << ")"
                    << " AS "
                    << "\""
                    << cp.UserColumn
                    << "\"";
                // clang-format on
            }
        } else {
            // clang-format off
            query
                << "("
                << "printf('%02d', "
                << cp.TableName
                << ".hours)"
                <<" || "
                << "':'"
                << " || "
                << "printf('%02d',"
                << cp.TableName
                << ".minutes)"
                << ")"
                << " AS "
                << "\"";
            if (!cp.UserColumn.empty()) {
                query << cp.UserColumn;
            } else {
                query << "Duration";
            }
                query << "\"";
            // clang-format on
        }
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
                << " >= "
                << "'" << fromDate << "'"
                << " AND "
                << "workdays.date"
                << " <= "
                << "'" << toDate << "'";

    return whereClause.str();
}

void SQLiteExportQueryBuilder::AppendColumns(std::stringstream& query, const std::vector<std::string>& columns)
{
    for (auto i = 0; i < columns.size(); i++) {
        const auto& column = columns[i];
        if (!column.empty()) {
            query << column;
            if (i != columns.size() - 1) {
                query << ", ";
            }
        }
    }
    query << " ";
}

void SQLiteExportQueryBuilder::AppendJoins(std::stringstream& query, const std::vector<std::string>& joins)
{
    for (auto i = 0; i < joins.size(); i++) {
        const auto& join = joins[i];
        if (!join.empty()) {
            query << join;
            if (i != joins.size() - 1) {
                query << " ";
            }
        }
    }
}

void SQLiteExportQueryBuilder::AppendClause(std::stringstream& query, std::string name, std::string clause)
{
    if (!clause.empty()) {
        query << name << clause;
    }
}
} // namespace tks::Services::Export
