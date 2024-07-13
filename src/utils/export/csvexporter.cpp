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
    const std::string& fromDate,
    const std::string& toDate)
{
    pQueryBuilder->IsPreview(true);
}

// #####################################################################################################################

ColumnProjection::ColumnProjection()
    : databaseColumn("")
    , userColumn("")
    , columnTableName("")
{
}

ColumnProjection::ColumnProjection(std::string databaseColumn, std::string userColumn, std::string columnTableName)
    : databaseColumn(databaseColumn)
    , userColumn(userColumn)
    , columnTableName(columnTableName)
{
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

const std::string SQLiteExportQueryBuilder::GetFromDate() const
{
    return mFromDate;
}

void SQLiteExportQueryBuilder::SetFromDate(const std::string& date)
{
    mFromDate = date;
}

const std::string SQLiteExportQueryBuilder::GetToDate() const
{
    return mToDate;
}

void SQLiteExportQueryBuilder::SetToDate(const std::string& date)
{
    mToDate = date;
}

std::string SQLiteExportQueryBuilder::Build(const std::vector<Projection>& projections)
{
    return std::string();
}

std::string SQLiteExportQueryBuilder::BuildQuery()
{
    return std::string();
}

std::string SQLiteExportQueryBuilder::BuildQueryString()
{
    return std::string();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeProjection(const std::vector<Projection>& projections)
{
    if (projections.size() != 0) {
        std::vector<std::string> projectionsOut;

        for (const auto& projection : projections) {
            std::string projectionOut = ComputeSingleProjection(projection);
            projectionsOut.push_back(projectionOut);
        }

        return projectionsOut;
    }

    return std::vector<std::string>();
}

std::string SQLiteExportQueryBuilder::ComputeSingleProjection(const Projection& projection)
{
    std::stringstream query;
    ColumnProjection cp = projection.columnProjection;

    if (cp.userColumn.size() != 0) {
        query << cp.columnTableName << "." << cp.databaseColumn << " AS " << cp.userColumn;
    } else {
        query << cp.columnTableName << "." << cp.databaseColumn;
    }

    query << ",";

    return query.str();
}
} // namespace tks::Utils
