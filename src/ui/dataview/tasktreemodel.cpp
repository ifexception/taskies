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

#include <cstdint>

#include "../../utils/utils.h"

namespace tks::UI
{
TaskTreeModel::TaskTreeModel()
    : pTopLevel()
    , pNode1(nullptr)
    , pNode2(nullptr)
{
    pNode1 = new TaskTreeModelNode(nullptr, Utils::ToISODateTime(Utils::UnixTimestamp()));
    pNode1->Append(new TaskTreeModelNode(pNode1, "ControlFirst", "Meetings", "00:15", "Stand up", 1));
    pNode1->Append(new TaskTreeModelNode(pNode1, "ControlFirst", "Coding", "00:15", "Hierarchy improvements", 2));

    pNode2 = new TaskTreeModelNode(nullptr, Utils::ToISODateTime(Utils::UnixTimestamp()));
    pNode2->Append(new TaskTreeModelNode(pNode2, "SLA", "Coding", "00:15", "Fix permissions", 3));

    pTopLevel.Add(pNode1);
    pTopLevel.Add(pNode2);
}

TaskTreeModel::~TaskTreeModel()
{
    std::size_t count = pTopLevel.size();
    for (std::size_t i = 0; i < count; i++) {
        TaskTreeModelNode* root = pTopLevel[i];
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

    size_t count = pTopLevel.GetCount();
    for (size_t i = 0; i < count; i++) {
        if (node == pTopLevel[i]) {
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
    TaskTreeModelNode* node = (TaskTreeModelNode*) parent.GetID();
    if (!node) {
        size_t count = pTopLevel.GetCount();
        for (size_t i = 0; i < count; i++) {
            TaskTreeModelNode* child = pTopLevel[i];
            array.Add(wxDataViewItem((void*) child));
        }
    }
    return pTopLevel.size();
}
} // namespace tks::UI
