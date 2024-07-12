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
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>();
}

void CsvExporter::GeneratePreview(const std::vector<Projection>& projections)
{
    pQueryBuilder->IsPreview(true);
}

ColumnProjection::ColumnProjection()
    : databaseColumn("")
    , userColumn("")
{
}

ColumnProjection::ColumnProjection(std::string databaseColumn, std::string userColumn)
    : databaseColumn(databaseColumn)
    , userColumn(userColumn)
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
} // namespace tks::Utils
