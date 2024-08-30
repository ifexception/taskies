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

#include "availablecolumns.h"

namespace tks::Services::Export
{
std::vector<AvailableColumn> MakeAvailableColumns()
{
    AvailableColumn employer{ "name", "Employer", "employers", "employer_id", JoinType::InnerJoin };
    AvailableColumn client{ "name", "Client", "clients", "client_id", JoinType::LeftJoin };
    AvailableColumn project{ "name", "Project", "projects", "project_id", JoinType::InnerJoin };
    AvailableColumn projectDisplayName{ "display_name", "Display Name", "projects", "project_id", JoinType::InnerJoin };
    AvailableColumn category{ "name", "Category", "categories", "category_id", JoinType::InnerJoin };
    AvailableColumn date{ "date", "Date", "workdays", "workday_id", JoinType::None };
    AvailableColumn description{ "description", "Description", "tasks", "", JoinType::None };
    AvailableColumn billable{ "billable", "Billable", "tasks", "", JoinType::None };
    AvailableColumn uid{ "unique_identifier", "Unique ID", "tasks", "", JoinType::None };
    AvailableColumn time{
        "*time*", "Duration", "tasks", "", JoinType::None
    }; // *time* special identifier to select two columns into one

    return std::vector<AvailableColumn>{
        employer, client, project, projectDisplayName, category, date, description, billable, uid, time
    };
}
} // namespace tks::Services::Export
