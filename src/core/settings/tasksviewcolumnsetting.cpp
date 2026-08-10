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
TasksViewColumnSetting::TasksViewColumnSetting()
    : Name("")
    , DisplayName("")
    , Order(-1)
    , TextAlignment(TasksViewColumnTextAlignment::Left)
    , TaskViewColumnId(TasksViewColumnIdentifier::Unknown)
    , Width(DefaultWidth)
    , Type(TasksViewColumnType::Text)
    , Selected(false)
{
}

TasksViewColumnSetting::TasksViewColumnSetting(const std::string& name,
    const std::string& displayName,
    int order,
    TasksViewColumnTextAlignment textAlignment,
    TasksViewColumnIdentifier columnId,
    int width,
    TasksViewColumnType type,
    bool selected)
    : Name(name)
    , DisplayName(displayName)
    , Order(order)
    , TextAlignment(textAlignment)
    , TaskViewColumnId(columnId)
    , Width(width)
    , Type(type)
    , Selected(selected)
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

TasksViewColumnSetting MakeDescriptionTasksViewColumn()
{
    TasksViewColumnSetting description("Description",
        "Description",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Description,
        -1,
        TasksViewColumnType::Text,
        true);

    return description;
}

const std::vector<TasksViewColumnSetting>& MakeDefaultTasksViewColumnList()
{
    int orderIndex = 1;

    TasksViewColumnSetting project("Project",
        "Project",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Project,
        DefaultWidth,
        TasksViewColumnType::Text,
        true);
    TasksViewColumnSetting category("Category",
        "Category",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Category,
        DefaultWidth,
        TasksViewColumnType::Text,
        true);
    TasksViewColumnSetting duration("Duration",
        "Duration",
        orderIndex++,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::Duration,
        DefaultWidth,
        TasksViewColumnType::Text,
        true);
    TasksViewColumnSetting description("Description",
        "Description",
        orderIndex++,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Description,
        -1,
        TasksViewColumnType::Text,
        true);

    static std::vector<TasksViewColumnSetting> columns{ project, category, duration, description };
    return columns;
}

const std::vector<TasksViewColumnSetting>& MakeAllTasksViewColumnList()
{
    TasksViewColumnSetting date("Date",
        "Date",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::Date,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting employer("Employer",
        "Employer",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Employer,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting client("Client",
        "Client",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Client,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting project("Project",
        "Project",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Project,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting category("Category",
        "Category",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Category,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting duration("Duration",
        "Duration",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::Duration,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting billable("Billable",
        "Billable",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::Billable,
        DefaultWidth,
        TasksViewColumnType::Toggle,
        false);

    TasksViewColumnSetting uniqueIdentifier("UniqueIdentifier",
        "Unique ID",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::UniqueIdentifier,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting taskAttributeValues("TaskAttributeValues",
        "Attributes + Values",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::TaskAttributeValues,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting taskCreatedDate("TaskCreatedDate",
        "Created",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::TaskCreatedDate,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting taskModifiedDate("TaskModifiedDate",
        "Modified",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::TaskModifiedDate,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting taskAttributes("TaskAttributes",
        "Attribute Values",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::TaskAttributes,
        DefaultWidth,
        TasksViewColumnType::Text,
        false);

    TasksViewColumnSetting isMeeting("IsMeeting",
        "Meeting",
        -1,
        TasksViewColumnTextAlignment::Center,
        TasksViewColumnIdentifier::IsMeeting,
        DefaultWidth,
        TasksViewColumnType::Toggle,
        false);

    TasksViewColumnSetting description("Description",
        "Description",
        -1,
        TasksViewColumnTextAlignment::Left,
        TasksViewColumnIdentifier::Description,
        -1,
        TasksViewColumnType::Text,
        false);

    static std::vector<TasksViewColumnSetting> columns{ date,
        employer,
        client,
        project,
        category,
        duration,
        billable,
        uniqueIdentifier,
        taskAttributeValues,
        taskCreatedDate,
        taskModifiedDate,
        isMeeting,
        description };
    return columns;
}
} // namespace tks::Core::Settings
