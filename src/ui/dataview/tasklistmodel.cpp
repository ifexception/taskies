// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

#include "tasklistmodel.h"

namespace tks::UI
{
TaskListItemModel::TaskListItemModel(const std::string& projectName,
    const std::string& categoryName,
    const std::string& duration,
    const std::string& description,
    std::int64_t taskId)
    : mProjectName(projectName)
    , mCategoryName(categoryName)
    , mDuration(duration)
    , mDescription(description)
    , mTaskId(taskId)
{
}

std::string TaskListItemModel::GetProjectName() const
{
    return mProjectName;
}

std::string TaskListItemModel::GetCategoryName() const
{
    return mCategoryName;
}

std::string TaskListItemModel::GetDuration() const
{
    return mDuration;
}

std::string TaskListItemModel::GetDescription() const
{
    return mDescription;
}

std::int64_t TaskListItemModel::GetTaskId() const
{
    return mTaskId;
}

void TaskListItemModel::SetProjectName(const std::string& value)
{
    mProjectName = value;
}

void TaskListItemModel::SetCategoryName(const std::string& value)
{
    mCategoryName = value;
}

void TaskListItemModel::SetDuration(const std::string& value)
{
    mDuration = value;
}

void TaskListItemModel::SetDescription(const std::string& value)
{
    mDescription = value;
}

void TaskListItemModel::SetTaskId(std::int64_t taskId)
{
    mTaskId = taskId;
}

// ###################################################################

TaskListModel::TaskListModel()
    : mListItemModels()
{
}

void TaskListModel::Append() {}

void TaskListModel::ChangeItem() {}

void TaskListModel::DeleteItem() {}
} // namespace tks::UI
