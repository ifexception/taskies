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

DatabaseExportQueryBuilder::DatabaseExportQueryBuilder()
    : mSelectQuery()
    , mFromQuery()
    , mJoinsQuery()
    , mWhereQuery()
{
    mSelectQuery << "SELECT" << newline;
    mFromQuery << "FROM tasks" << newline;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithEmployerName()
{
    mSelectQuery << "employers.name" << comma << newline;

    mJoinsQuery << "INNER JOIN employers ON projects.employer_id = employers.employer_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithClientName()
{
    mSelectQuery << "clients.name" << comma << newline;

    mJoinsQuery << "LEFT JOIN clients ON projects.client_id = clients.client_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithProjectName()
{
    mSelectQuery << "projects.name" << comma << newline;

    mJoinsQuery << "INNER JOIN projects ON tasks.project_id = projects.project_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithProjectDisplayName()
{
    mSelectQuery << "projects.display_name" << comma << newline;

    mJoinsQuery << "INNER JOIN projects ON tasks.project_id = projects.project_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithCategoryName()
{
    mSelectQuery << "categories.name" << comma << newline;

    mJoinsQuery << "INNER JOIN categories ON tasks.category_id = categories.category_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithDate()
{
    mSelectQuery << "workdays.date" << comma << newline;

    mJoinsQuery << "INNER JOIN workdays ON tasks.workday_id = workdays.workday_id" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithTaskDescription()
{
    mSelectQuery << "tasks.description" << comma << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithBillable()
{
    mSelectQuery << "tasks.billable" << comma << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithUniqueId()
{
    mSelectQuery << "tasks.unique_identifier" << comma << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithTime()
{
    mSelectQuery << "tasks.hours" << newline;
    mSelectQuery << "tasks.minutes" << newline;

    return *this;
}

DatabaseExportQueryBuilder& DatabaseExportQueryBuilder::WithDateRange(const std::string& fromDate,
    const std::string& toDate)
{
    mWhereQuery << "workdays.date >= '" << fromDate << "'" << newline;
    mWhereQuery << "workdays.date <= '" << toDate << "'" << newline;

    return *this;
}

std::string DatabaseExportQueryBuilder::Build()
{
    return std::string();
}

CsvExporter::CsvExporter(std::shared_ptr<spdlog::logger> logger, CsvExportOptions options)
    : pLogger(logger)
    , mOptions(options)
{
}
} // namespace tks::Utils
