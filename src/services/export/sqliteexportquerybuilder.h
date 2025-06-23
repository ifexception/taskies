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

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "projection.h"
#include "columnjoinprojection.h"

namespace tks::Services::Export
{
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

    std::string BuildQuery(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate);

    std::string BuildAttributesQuery(const std::string& fromDate,
        const std::string& toDate,
        const std::int64_t taskId);

private:
    std::string BuildQueryInternal(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate);

    std::string BuildAttributesQueryInternal(const std::string& fromDate,
        const std::string& toDate,
        const std::int64_t taskId);

    std::string BuildQueryString(const std::vector<std::string>& columns,
        const std::vector<std::string>& firstLevelJoins,
        const std::vector<std::string>& secondLevelJoins,
        const std::string& where);

    std::string BuildAttributeQueryString(const std::string& where, const std::int64_t taskId);

    std::vector<std::string> ComputeFirstLevelJoinProjections(
        const std::vector<ColumnJoinProjection>& joinProjections);
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
} // namespace tks::Services::Export
