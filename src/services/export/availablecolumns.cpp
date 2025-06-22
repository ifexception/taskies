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

#include "availablecolumns.h"

namespace tks::Services::Export
{
std::vector<AvailableColumn> MakeAvailableColumns()
{
    AvailableColumn employer{
        "name", "Employer", "employers", "employer_id", JoinType::InnerJoin, FieldType::Default
    };
    AvailableColumn client{
        "name", "Client", "clients", "client_id", JoinType::LeftJoin, FieldType::Default
    };
    AvailableColumn project{
        "name", "Project", "projects", "project_id", JoinType::InnerJoin, FieldType::Default
    };
    AvailableColumn projectDisplayName{ "display_name",
        "Display Name",
        "projects",
        "project_id",
        JoinType::InnerJoin,
        FieldType::Default };
    AvailableColumn category{
        "name", "Category", "categories", "category_id", JoinType::InnerJoin, FieldType::Default
    };
    AvailableColumn date{
        "date", "Date", "workdays", "workday_id", JoinType::None, FieldType::Default
    };
    AvailableColumn description{
        "description", "Description", "tasks", "", JoinType::None, FieldType::Default
    };
    AvailableColumn billable{
        "billable", "Billable", "tasks", "", JoinType::None, FieldType::Default
    };
    AvailableColumn uid{
        "unique_identifier", "Unique ID", "tasks", "", JoinType::None, FieldType::Default
    };
    AvailableColumn hours{ "hours", "Hours", "tasks", "", JoinType::None, FieldType::Formatted };
    AvailableColumn minutes{
        "minutes", "Minutes", "tasks", "", JoinType::None, FieldType::Formatted
    };
    // *time* special identifier to select two columns into one
    AvailableColumn time{ "*time*", "Duration", "tasks", "", JoinType::None, FieldType::Formatted };

    return std::vector<AvailableColumn>{ employer,
        client,
        project,
        projectDisplayName,
        category,
        date,
        description,
        billable,
        uid,
        hours,
        minutes,
        time };
}
} // namespace tks::Services::Export
