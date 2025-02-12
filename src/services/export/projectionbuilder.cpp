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

#include "projectionbuilder.h"

#include <algorithm>

namespace tks::Services::Export
{
ProjectionBuilder::ProjectionBuilder(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mAvailableColumns(MakeAvailableColumns())
{
}

std::vector<Projection> ProjectionBuilder::BuildProjections(
    const std::vector<ColumnExportModel>& columns)
{
    std::vector<Projection> projections;

    for (const auto& column : columns) {
        // clang-format off
        const auto& availableColumnIterator = std::find_if(
            mAvailableColumns.begin(),
            mAvailableColumns.end(),
            [=](const AvailableColumn& availableColumn) {
                return availableColumn.UserColumn == column.OriginalColumn;
            });
        // clang-format on

        if (availableColumnIterator != mAvailableColumns.end()) {
            const auto& availableColumn = *availableColumnIterator;
            pLogger->info("ProjectionBuilder::BuildProjections - Matched column \"{0}\" with available column \"{1}\"",
                column.OriginalColumn,
                availableColumn.DatabaseColumn);

            Projection projection = BuildProjection(column, availableColumn);

            projections.push_back(projection);
        }
    }

    SortProjectionsByOrderDescending(projections);

    return projections;
}

std::vector<ColumnJoinProjection> ProjectionBuilder::BuildJoinProjections(
    const std::vector<ColumnExportModel>& columns)
{
    std::vector<ColumnJoinProjection> joinProjections;

    ColumnJoinProjection projectTableJoinProjection = BuildRequiredProjectTableJoinProjection();
    joinProjections.push_back(projectTableJoinProjection);

    for (const auto& column : columns) {
        // clang-format off
        const auto& availableColumnIterator = std::find_if(
            mAvailableColumns.begin(),
            mAvailableColumns.end(),
            [=](const AvailableColumn& availableColumn) {
                return availableColumn.UserColumn == column.OriginalColumn &&
                    availableColumn.Join != JoinType::None &&
                    availableColumn.TableName != "projects";
            });
        // clang-format on

        if (availableColumnIterator != mAvailableColumns.end()) {
            const auto& availableColumn = *availableColumnIterator;
            pLogger->info("ProjectionBuilder::BuildProjections - Matched column \"{0}\" with available column \"{1}\"",
                column.OriginalColumn,
                availableColumn.DatabaseColumn);

            ColumnJoinProjection columnJoinProjection = BuildJoinProjection(column, availableColumn);

            joinProjections.push_back(columnJoinProjection);
        }
    }
    return joinProjections;
}

Projection ProjectionBuilder::BuildProjection(const ColumnExportModel& column,
    const AvailableColumn& availableColumn)
{
    SColumnProjection cp(availableColumn.DatabaseColumn,
        column.Column,
        availableColumn.IdColumn,
        availableColumn.TableName,
        availableColumn.Field);

    if (availableColumn.DatabaseColumn == "*time*") {
        cp.SpecialIdentifierForDurationColumns = "*time*";
    }

    Projection projection(column.Order, cp);
    return projection;
}

void ProjectionBuilder::SortProjectionsByOrderDescending(std::vector<Projection>& projections)
{
    pLogger->info("ProjectionBuilder::SortProjectionsByOrder - Sort projections by order index ascending");

    // clang-format off
    std::sort(
        projections.begin(),
        projections.end(),
        [](const Projection& lhs, const Projection& rhs) {
            return lhs.Order < rhs.Order;
        }
    );
    // clang-format on
}

ColumnJoinProjection ProjectionBuilder::BuildRequiredProjectTableJoinProjection()
{
    /* Insert projects regardless as a catch-all scenario for first and second level table joins */
    // clang-format off
    const auto& projectColumnIterator = std::find_if(
        mAvailableColumns.begin(),
        mAvailableColumns.end(),
        [=](const AvailableColumn& column) {
            return column.TableName == "projects";
        });
    // clang-format on

    if (projectColumnIterator != mAvailableColumns.end()) {
        const auto& projectColumn = *projectColumnIterator;
        ColumnJoinProjection cjp(projectColumn.TableName, projectColumn.IdColumn, projectColumn.Join);
        pLogger->info("ProjectionBuilder::BuildJoinProjections - Insert projects table to join on");

        return cjp;
    }

    return ColumnJoinProjection();
}

ColumnJoinProjection ProjectionBuilder::BuildJoinProjection(const ColumnExportModel& column,
    const AvailableColumn& availableColumn)
{
    if (availableColumn.TableName == "categories") {
        ColumnJoinProjection cjp(availableColumn.TableName, availableColumn.IdColumn, JoinType::InnerJoin);

        pLogger->info("ProjectionBuilder::BuildJoinProjection - First level join on \"{0}\" with join \"{1}\"",
            availableColumn.TableName,
            "INNER");

        return cjp;
    }

    if (availableColumn.TableName == "employers" || availableColumn.TableName == "clients") {
        ColumnJoinProjection cjp(availableColumn.TableName, availableColumn.IdColumn, true);

        if (availableColumn.TableName == "clients") {
            cjp.Join = JoinType::LeftJoin;
        } else {
            cjp.Join = JoinType::InnerJoin;
        }

        pLogger->info("ProjectionBuilder::BuildJoinProjection - Second level join on \"{0}\" with join \"{1}\"",
            availableColumn.TableName,
            cjp.Join == JoinType::InnerJoin ? "INNER" : "LEFT");

        return cjp;
    }

    return ColumnJoinProjection();
}
} // namespace tks::Services::Export
