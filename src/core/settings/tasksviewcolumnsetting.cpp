// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "tasksviewcolumnsetting.h"

namespace tks::Core::Settings
{
int TasksViewColumnSetting::ColumnDefaultWidth = 80;
int TasksViewColumnSetting::ColumnAutoWidth = -1;

int TasksViewColumnSetting::DefaultOrderIndex = -1;

TasksViewColumnSetting::TasksViewColumnSetting()
    : Name("")
    , DisplayName("")
    , Order(-1)
    , TextAlignment(TasksViewColumnTextAlignment::Left)
    , Type(TasksViewColumnType::Text)
    , EllipsisMode(TasksViewColumnEllipsisMode::Middle)
    , Width(TasksViewColumnSetting::ColumnDefaultWidth)
    , Selected(false)
    , TaskViewColumnId(TasksViewColumnIdentifier::Unknown)
{
}

TasksViewColumnSetting::TasksViewColumnSetting(const std::string& name,
    const std::string& displayName,
    int order,
    TasksViewColumnTextAlignment textAlignment,
    TasksViewColumnType type,
    TasksViewColumnEllipsisMode ellipsizeMode,
    int width,
    bool selected,
    TasksViewColumnIdentifier columnId)
    : Name(name)
    , DisplayName(displayName)
    , Order(order)
    , TextAlignment(textAlignment)
    , Type(type)
    , EllipsisMode(ellipsizeMode)
    , Width(width)
    , Selected(selected)
    , TaskViewColumnId(columnId)
{
}

bool TasksViewColumnSetting::operator==(const TasksViewColumnSetting& other) const
{
    return Order == other.Order;
}

bool TasksViewColumnSetting::operator!=(const TasksViewColumnSetting& other) const
{
    return Order != other.Order;
}

bool TasksViewColumnSetting::IsValid() const
{
    return !Name.empty() && !DisplayName.empty() &&
           TaskViewColumnId != TasksViewColumnIdentifier::Unknown;
}

TasksViewColumnSetting MakeDescriptionTasksViewColumn()
{
    TasksViewColumnSetting description("Description",
        "Description",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::End,
        TasksViewColumnSetting::ColumnAutoWidth,
        true,
        TasksViewColumnIdentifier::Description);

    return description;
}

const std::vector<TasksViewColumnSetting>& MakeDefaultTasksViewColumnList()
{
    int orderIndex = 1;

    TasksViewColumnSetting date("Date",
        "Date",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Date);

    TasksViewColumnSetting employer("Employer",
        "Employer",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Employer);

    TasksViewColumnSetting client("Client",
        "Client",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Client);

    TasksViewColumnSetting project("Project",
        "Project",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Project);

    TasksViewColumnSetting category("Category",
        "Category",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Category);

    TasksViewColumnSetting duration("Duration",
        "Duration",
        orderIndex++,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Duration);

    TasksViewColumnSetting billable("Billable",
        "Billable",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Toggle,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Billable);

    TasksViewColumnSetting uniqueIdentifier("UniqueIdentifier",
        "Unique ID",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::UniqueIdentifier);

    TasksViewColumnSetting taskCreatedDate("TaskCreatedDate",
        "Created",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskCreatedDate);

    TasksViewColumnSetting taskModifiedDate("TaskModifiedDate",
        "Modified",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskModifiedDate);

    TasksViewColumnSetting taskAttributeValues("TaskAttributeValues",
        "Attribute Values",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskAttributeValues);

    TasksViewColumnSetting isMeeting("IsMeeting",
        "Meeting",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Toggle,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::IsMeeting);

    TasksViewColumnSetting description("Description",
        "Description",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnAutoWidth,
        true,
        TasksViewColumnIdentifier::Description);

    static std::vector<TasksViewColumnSetting> columns{ date,
        employer,
        client,
        project,
        category,
        duration,
        billable,
        uniqueIdentifier,
        taskCreatedDate,
        taskModifiedDate,
        taskAttributeValues,
        isMeeting,
        description };

    return columns;
}

const std::vector<TasksViewColumnSetting>& MakeAllTasksViewColumnList()
{
    TasksViewColumnSetting date("Date",
        "Date",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Date);

    TasksViewColumnSetting employer("Employer",
        "Employer",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Employer);

    TasksViewColumnSetting client("Client",
        "Client",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Client);

    TasksViewColumnSetting project("Project",
        "Project",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Project);

    TasksViewColumnSetting category("Category",
        "Category",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Category);

    TasksViewColumnSetting duration("Duration",
        "Duration",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        true,
        TasksViewColumnIdentifier::Duration);

    TasksViewColumnSetting billable("Billable",
        "Billable",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Toggle,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::Billable);

    TasksViewColumnSetting uniqueIdentifier("UniqueIdentifier",
        "Unique ID",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::UniqueIdentifier);

    TasksViewColumnSetting taskCreatedDate("TaskCreatedDate",
        "Created",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskCreatedDate);

    TasksViewColumnSetting taskModifiedDate("TaskModifiedDate",
        "Modified",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskModifiedDate);

    TasksViewColumnSetting taskAttributeValues("TaskAttributeValues",
        "Attribute Values",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::TaskAttributeValues);

    TasksViewColumnSetting isMeeting("IsMeeting",
        "Meeting",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnType::Toggle,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnDefaultWidth,
        false,
        TasksViewColumnIdentifier::IsMeeting);

    TasksViewColumnSetting description("Description",
        "Description",
        TasksViewColumnSetting::DefaultOrderIndex,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnType::Text,
        TasksViewColumnEllipsisMode::Middle,
        TasksViewColumnSetting::ColumnAutoWidth,
        true,
        TasksViewColumnIdentifier::Description);

    static std::vector<TasksViewColumnSetting> columns{ date,
        employer,
        client,
        project,
        category,
        duration,
        billable,
        uniqueIdentifier,
        taskCreatedDate,
        taskModifiedDate,
        taskAttributeValues,
        isMeeting,
        description };

    return columns;
}
} // namespace tks::Core::Settings
