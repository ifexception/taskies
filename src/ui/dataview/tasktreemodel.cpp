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

#include "tasktreemodel.h"

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

TaskTreeModel::TaskTreeModel(date::year_month_day fromDate, date::year_month_day toDate)
    : mFromDate(fromDate)
    , mToDate(toDate)
    , mRootDayNodes()
{
    date::days dayRange = mToDate.day() - mFromDate.day();
    if (dayRange > date::days{ 0 }) {
        int x = 1;

        auto fromDateFormatted = date::format("%F", mFromDate);
        TaskTreeModelNode* firstNode = new TaskTreeModelNode(nullptr, fromDateFormatted);
        mRootDayNodes.push_back(firstNode);

        for (date::day i = mFromDate.day(); i < mToDate.day(); i++) {
            auto nextDay = mFromDate.year() / mFromDate.month() / (mFromDate.day() + date::days{ x++ });
            auto nextDayFormatted = date::format("%F", nextDay);

            TaskTreeModelNode* node = new TaskTreeModelNode(nullptr, nextDayFormatted);
            mRootDayNodes.push_back(node);
        }
    }
}

TaskTreeModel::~TaskTreeModel()
{
    std::size_t count = mRootDayNodes.size();
    for (std::size_t i = 0; i < count; i++) {
        TaskTreeModelNode* root = mRootDayNodes[i];
        delete root;
    }
}

unsigned int TaskTreeModel::GetColumnCount() const
{
    return Col_Max;
}

wxString TaskTreeModel::GetColumnType(unsigned int col) const
{
    if (col == Col_Id) {
        return "long";
    } else {
        return "string";
    }
}

void TaskTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
    wxASSERT(item.IsOk());

    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();
    switch (col) {
    case Col_Project:
        variant = node->GetProjectName();
        break;
    case Col_Category:
        variant = node->GetCategoryName();
        break;
    case Col_Duration:
        variant = node->GetDuration();
        break;
    case Col_Description:
        variant = node->GetDescription();
        break;
    case Col_Id:
        variant = (long) node->GetTaskId();
        break;
    case Col_Max:
    default:
        // log error
        break;
    }
}

bool TaskTreeModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
    wxASSERT(item.IsOk());

    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();
    switch (col) {
    case Col_Project:
        node->SetProjectName(variant.GetString().ToStdString());
        break;
    case Col_Duration:
        node->SetDuration(variant.GetString().ToStdString());
        break;
    case Col_Category:
        node->SetCategoryName(variant.GetString().ToStdString());
        break;
    case Col_Description:
        node->SetDescription(variant.GetString().ToStdString());
        break;
    case Col_Id:
        node->SetTaskId(static_cast<std::int64_t>(variant.GetInteger()));
        break;
    case Col_Max:
    default:
        // pLogger
        break;
    }

    return false;
}

bool TaskTreeModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
    return true;
}

wxDataViewItem TaskTreeModel::GetParent(const wxDataViewItem& item) const
{
    if (!item.IsOk()) {
        return wxDataViewItem(0);
    }

    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();

    for (TaskTreeModelNode* rootNode : mRootDayNodes) {
        if (node == rootNode) {
            return wxDataViewItem(0);
        }
    }

    return wxDataViewItem((void*) node->GetParent());
}

bool TaskTreeModel::IsContainer(const wxDataViewItem& item) const
{
    if (!item.IsOk()) {
        return true;
    }

    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();
    return node->IsContainer();
}

unsigned int TaskTreeModel::GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const
{
    for (TaskTreeModelNode* rootNode : mRootDayNodes) {
        array.Add(wxDataViewItem((void*) rootNode));

        /*if (rootNode->GetChildCount() == 0) {
            return;
        }

        unsigned int count = rootNode->GetChildren().GetCount();
        for (unsigned int pos = 0; pos < count; pos++) {
            TaskTreeModelNode* child = rootNode->GetChildren().Item(pos);
            array.Add(wxDataViewItem((void*) child));
        }

        return count;*/
    }
    return mRootDayNodes.size();
}
} // namespace tks::UI
