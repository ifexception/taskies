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

namespace tks::Core::Configuration::Settings
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

std::vector<TasksViewColumnSetting> MakeDefaultTasksViewColumnList()
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

    return std::vector<TasksViewColumnSetting>{
        project,
        category,
        duration,
        description
    };
}
} // namespace tks::Core::Configuration::Settings
