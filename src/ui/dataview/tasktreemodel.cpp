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
    : pRoot(nullptr)
    , pDayNodes()
{
    pRoot = new TaskTreeModelNode(nullptr, "root-node");

    for (std::size_t i = 0; i < 3; i++) {
        pDayNodes.push_back(new TaskTreeModelNode(pRoot, "Node Index " + i));

        auto node1 = new TaskTreeModelNode(pDayNodes[i], "Project " + i, "Cat#1", "00:00", "task description here", i);
        auto node2 = new TaskTreeModelNode(pDayNodes[i], "Project " + i, "Cat#2", "00:15", "task description here", i);

        pDayNodes[i]->Append(node1);
        pDayNodes[i]->Append(node2);

        pRoot->Append(pDayNodes[i]);
    }
}

TaskTreeModel::~TaskTreeModel()
{
    delete pRoot;
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

    if (node == pRoot) {
        return wxDataViewItem(0);
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
        array.Add(wxDataViewItem((void*) pRoot));
        return 1;
    }

    if (node->GetChildCount() == 0) {
        return 0;
    }

    unsigned int count = node->GetChildren().GetCount();
    for (unsigned int pos = 0; pos < count; pos++) {
        TaskTreeModelNode* child = node->GetChildren().Item(pos);
        array.Add(wxDataViewItem((void*) child));
    }

    return count;
}

void TaskTreeModel::Delete(const wxDataViewItem& item)
{
    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();
    if (!node) { // happens if item.IsOk()==false
        return;
    }

    wxDataViewItem parent(node->GetParent());
    if (!parent.IsOk()) { // this means that the root node was clicked and has no parent
        return;
    }

    // do not delete the special nodes
    for (std::size_t i = 0; i < 3; i++) {
        if (node == pDayNodes[i]) {
            return;
        }
    }

    node->GetParent()->GetChildren().Remove(node);
    delete node;

    ItemDeleted(parent, item);
}

void TaskTreeModel::ClearAll()
{
    for (std::size_t i = 0; i < 3; i++) {
        auto node = pDayNodes[i];
        wxDataViewItemArray itemsRemoved;
        unsigned int count = node->GetChildCount();

        for (unsigned int pos = 0; pos < count; pos++) {
            TaskTreeModelNode* child = node->GetChildren().Item(pos);
            itemsRemoved.Add(wxDataViewItem((void*) child));
        }

        for (auto child : node->GetChildren()) {
            delete child;
            child = nullptr;
        }

        node->GetChildren().clear();

        count = node->GetChildCount();

        wxDataViewItem parent((void*) node);
        ItemsDeleted(parent, itemsRemoved);
    }
}
} // namespace tks::UI
