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

#include "tasktreemodelnode.h"

namespace tks::UI
{
TaskTreeModelNode::TaskTreeModelNode(TaskTreeModelNode* parent,
    const std::string& projectName,
    const std::string& categoryName,
    const std::string& duration,
    const std::string& description,
    std::int64_t taskId)
    : pParent(parent)
    , mProjectName(projectName)
    , mCategoryName(categoryName)
    , mDuration(duration)
    , mDescription(description)
    , mTaskId(taskId)
    , bContainer(false)
{
}

TaskTreeModelNode::TaskTreeModelNode(TaskTreeModelNode* parent, const std::string& branch)
    : pParent(parent)
    , mProjectName(branch)
    , bContainer(true)
    , mCategoryName("")
    , mDuration("")
    , mDescription("")
    , mTaskId(0)
{
}

TaskTreeModelNode::~TaskTreeModelNode()
{
    std::size_t count = mChildren.GetCount();
    for (std::size_t i = 0; i < count; i++) {
        TaskTreeModelNode* child = mChildren[i];
        delete child;
    }
}

bool TaskTreeModelNode::IsContainer() const
{
    return bContainer;
}

TaskTreeModelNode* TaskTreeModelNode::GetParent()
{
    return pParent;
}

TaskTreeModelNodePtrArray& TaskTreeModelNode::GetChildren()
{
    return mChildren;
}

TaskTreeModelNode* TaskTreeModelNode::GetNthChild(unsigned int n)
{
    return mChildren.Item(n);
}

void TaskTreeModelNode::Insert(TaskTreeModelNode* child, unsigned int n)
{
    mChildren.Insert(child, n);
}

void TaskTreeModelNode::Append(TaskTreeModelNode* child)
{
    mChildren.Add(child);
}

const unsigned int TaskTreeModelNode::GetChildCount() const
{
    return mChildren.Count();
}

std::string TaskTreeModelNode::GetProjectName() const
{
    return mProjectName;
}

std::string TaskTreeModelNode::GetCategoryName() const
{
    return mCategoryName;
}

std::string TaskTreeModelNode::GetDuration() const
{
    return mDuration;
}

std::string TaskTreeModelNode::GetDescription() const
{
    return mDescription;
}

std::int64_t TaskTreeModelNode::GetTaskId() const
{
    return mTaskId;
}

void TaskTreeModelNode::SetProjectName(const std::string& value)
{
    mProjectName = value;
}

void TaskTreeModelNode::SetCategoryName(const std::string& value)
{
    mCategoryName = value;
}

void TaskTreeModelNode::SetDuration(const std::string& value)
{
    mDuration = value;
}

void TaskTreeModelNode::SetDescription(const std::string& value)
{
    mDescription = value;
}

void TaskTreeModelNode::SetTaskId(std::int64_t taskId)
{
    mTaskId = taskId;
}
} // namespace tks::UI
