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

#pragma once

#include <cstdint>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

namespace tks::UI
{
class TaskTreeModelNode;
WX_DEFINE_ARRAY_PTR(TaskTreeModelNode*, TaskTreeModelNodePtrArray);

class TaskTreeModelNode final
{
public:
    TaskTreeModelNode() = delete;
    TaskTreeModelNode(const TaskTreeModelNode&) = delete;
    TaskTreeModelNode(TaskTreeModelNode* parent,
        const std::string& projectName,
        const std::string& categoryName,
        const std::string& duration,
        const std::string& description,
        std::int64_t taskId);
    TaskTreeModelNode(TaskTreeModelNode* parent, const std::string& branch);
    ~TaskTreeModelNode();

    bool IsContainer() const;
    TaskTreeModelNode* GetParent();
    TaskTreeModelNodePtrArray& GetChildren();
    TaskTreeModelNode* GetNthChild(unsigned int n);

    void Insert(TaskTreeModelNode* child, unsigned int n);
    void Append(TaskTreeModelNode* child);
    const unsigned int GetChildCount() const;

    std::string GetProjectName() const;
    std::string GetCategoryName() const;
    std::string GetDuration() const;
    std::string GetDescription() const;
    std::int64_t GetTaskId() const;

    void SetProjectName(const std::string& value);
    void SetCategoryName(const std::string& value);
    void SetDuration(const std::string& value);
    void SetDescription(const std::string& value);
    void SetTaskId(std::int64_t taskId);

private:
    TaskTreeModelNode* pParent;
    TaskTreeModelNodePtrArray mChildren;

    std::string mProjectName;
    std::string mDuration;
    std::string mCategoryName;
    std::string mDescription;
    std::int64_t mTaskId;
    bool bContainer;
};
}
