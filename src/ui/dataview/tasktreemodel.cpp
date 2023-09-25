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
TaskTreeModel::TaskTreeModel(std::chrono::time_point<std::chrono::system_clock, date::days> monday,
    std::chrono::time_point<std::chrono::system_clock, date::days> sunday)
    : pRoots()
{
    auto& dateIterator = monday;
    int loopIdx = 0;
    do {
        auto rootDateNode = new TaskTreeModelNode(nullptr, date::format("%F", dateIterator));
        pRoots.push_back(rootDateNode);

        dateIterator += date::days{ 1 };
        loopIdx++;
    } while (dateIterator != sunday);

    pRoots.push_back(new TaskTreeModelNode(nullptr, date::format("%F", dateIterator)));
}

TaskTreeModel::~TaskTreeModel()
{
    for (std::size_t i = 0; i < pRoots.size(); i++) {
        TaskTreeModelNode* child = pRoots[i];
        delete child;
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

    for (std::size_t i = 0; i < pRoots.size(); i++) {
        if (node == pRoots[i]) {
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
        for (std::size_t i = 0; i < pRoots.size(); i++) {
            TaskTreeModelNode* child = pRoots[i];
            array.Add(wxDataViewItem((void*) child));
        }

        return pRoots.size();
    }

    if (node->GetChildCount() == 0) {
        return 0;
    }

    for (TaskTreeModelNode* child : node->GetChildren()) {
        array.Add(wxDataViewItem(child));
    }
    return array.size();
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

    node->GetParent()->GetChildren().Remove(node);
    delete node;

    ItemDeleted(parent, item);
}

void TaskTreeModel::Clear()
{
    for (std::size_t i = 0; i < pRoots.size(); i++) {
        TaskTreeModelNode* node = pRoots[i];
        while (!node->GetChildren().IsEmpty()) {
            TaskTreeModelNode* child = node->GetNthChild(0);
            node->GetChildren().Remove(child);
            delete child;
        }

        delete node;
    }

    Cleared();

    /*for (std::size_t i = 0; i < 3; i++) {
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
    }*/
}

void TaskTreeModel::Insert(std::vector<repos::TaskRepositoryModel> models)
{
    auto mondayNode = pRoots[0];

    for (auto& model : models) {
        auto node = new TaskTreeModelNode(
            mondayNode, model.ProjectName, model.CategoryName, model.GetDuration(), model.Description, model.TaskId);
        mondayNode->Append(node);
    }
}

// void TaskTreeModel::SetDayNodeDateLabels(std::chrono::time_point<std::chrono::system_clock, date::days>& fromDate,
//     std::chrono::time_point<std::chrono::system_clock, date::days>& toDate)
//{
//     auto& dateIterator = fromDate;
//     int loopIdx = 0;
//     do {
//         auto rootDateNode = new TaskTreeModelNode(nullptr, date::format("%F", dateIterator));
//         pRoots.push_back(rootDateNode);
//         // pRoots[loopIdx]->Append(new TaskTreeModelNode(
//         //     pRoots[loopIdx], "Project 1", "Cat#1", "00:00", "task description here", loopIdx));
//
//         dateIterator += date::days{ 1 };
//         loopIdx++;
//     } while (dateIterator != toDate);
//
//     pRoots.push_back(new TaskTreeModelNode(nullptr, date::format("%F", dateIterator)));
//     // pRoots[loopIdx]->Append(
//     //     new TaskTreeModelNode(pRoots[loopIdx], "Project 1", "Cat#1", "00:00", "task description here", loopIdx));
// }
} // namespace tks::UI
