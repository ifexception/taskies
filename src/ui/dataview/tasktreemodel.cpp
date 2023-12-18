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

#include <algorithm>
#include <cstdint>

#include "../../utils/utils.h"

namespace tks::UI
{
TaskTreeModel::TaskTreeModel(std::chrono::time_point<std::chrono::system_clock, date::days> monday,
    std::chrono::time_point<std::chrono::system_clock, date::days> sunday,
    std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , pRoots()
{
    pLogger->info("TaskTreeModel - Initialize root nodes starting from {0}", date::format("%F", monday));
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
    pLogger->info("TaskTreeModel - Delete root nodes");
    for (std::size_t i = 0; i < pRoots.size(); i++) {
        TaskTreeModelNode* root = pRoots[i];
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
        pLogger->info("TaskTreeModel::GetValue - Invalid column selected");
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
        pLogger->info("TaskTreeModel::SetValue - Invalid column selected");
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
    pLogger->info("TaskTreeModel::GetParent - Begin to get parent");
    if (!item.IsOk()) {
        pLogger->info("TaskTreeModel::GetParent - Selected \"wxDataViewItem\" is not OK");
        return wxDataViewItem(0);
    }

    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();

    for (auto parentNode : pRoots) {
        if (node == parentNode) {
            pLogger->info(
                "TaskTreeModel::GetParent - Node matched with one of the root nodes \"{0}\"", node->GetProjectName());
            return wxDataViewItem(0);
        }
    }

    pLogger->info("TaskTreeModel::GetParent - Node is child, call up the node structure");
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
    pLogger->info("TaskTreeModel::GetChildren - Begin to get children of parent");
    TaskTreeModelNode* node = (TaskTreeModelNode*) parent.GetID();
    if (!node) {
        pLogger->info("TaskTreeModel::GetChildren - Selected node is a parent node");
        for (auto parentNode : pRoots) {
            array.Add(wxDataViewItem((void*) parentNode));
        }

        return pRoots.size();
    }

    if (node->GetChildCount() == 0) {
        return 0;
    }

    pLogger->info("TaskTreeModel::GetChildren - Get children node");
    for (const auto& child : node->GetChildren()) {
        array.Add(wxDataViewItem(child.get()));
    }
    return array.size();
}

void TaskTreeModel::Delete(const wxDataViewItem& item)
{
    pLogger->info("TaskTreeModel::Delete - Begin to delete node");
    TaskTreeModelNode* node = (TaskTreeModelNode*) item.GetID();
    if (!node) {
        return;
    }

    wxDataViewItem parent(node->GetParent());
    if (!parent.IsOk()) { // this means that the root node was clicked and has no parent
        pLogger->info("TaskTreeModel::Delete - Root node selected and skipping deletion of root node");
        return;
    }

    pLogger->info("TaskTreeModel::Delete - Delete node from parent");

    auto& children = node->GetChildren();
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (it->get() == node) {
            children.erase(it);
            break;
        }
    }

    ItemDeleted(parent, item);
}

void TaskTreeModel::DeleteChild(const std::string& date, const std::int64_t taskId)
{
    pLogger->info("TaskTreeModel::DeleteChild - Begin");
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == date; });

    if (iterator != pRoots.end()) {
        auto parentNode = *iterator;
        wxDataViewItem parent((void*) parentNode);

        auto& children = parentNode->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it) {
            if (it->get()->GetTaskId() == taskId) {
                auto child = it->get();
                wxDataViewItem childItem((void*) child);
                children.erase(it);

                ItemDeleted(parent, childItem);
                break;
            }
        }
    }
}

void TaskTreeModel::ChangeChild(const std::string& date, repos::TaskRepositoryModel& taskModel)
{
    pLogger->info("TaskTreeModel::ChangeChild - Begin");
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == date; });

    if (iterator != pRoots.end()) {
        auto parentNode = *iterator;
        wxDataViewItem parent((void*) parentNode);

        auto& children = parentNode->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it) {
            if (it->get()->GetTaskId() == taskModel.TaskId) {
                auto child = it->get();
                child->SetProjectName(taskModel.ProjectName);
                child->SetCategoryName(taskModel.CategoryName);
                child->SetDuration(taskModel.GetDuration());
                child->SetDescription(taskModel.GetTrimmedDescription());

                wxDataViewItem item((void*) child);
                ItemChanged(item);

                break;
            }
        }
    }
}

void TaskTreeModel::Clear()
{
    for (auto node : pRoots) {
        node->GetChildren().clear();
    }
    pRoots.clear();

    Cleared();
}

void TaskTreeModel::ClearAll()
{
    for (auto parentNode : pRoots) {
        wxDataViewItemArray itemsRemoved;

        auto& children = parentNode->GetChildren();
        for (auto it = children.begin(); it != children.end(); ++it) {
            auto child = it->get();
            itemsRemoved.Add(wxDataViewItem((void*) child));

            children.erase(it);
        }

        parentNode->GetChildren().clear();

        wxDataViewItem parent((void*) parentNode);
        ItemsDeleted(parent, itemsRemoved);
    }

    for (auto parentNode : pRoots) {
        wxDataViewItemArray itemsRemoved;
        itemsRemoved.Add(wxDataViewItem((void*) parentNode));

        wxDataViewItem parent((void*) parentNode->GetParent());
        ItemsDeleted(parent, itemsRemoved);
    }

    pRoots.clear();
}

void TaskTreeModel::ClearNodeEntriesByDateKey(const std::string& date)
{
    pLogger->info("TaskTreeModel::ClearNodeEntriesByDateKey - Begin");
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == date; });

    if (iterator != pRoots.end()) {
        auto parentNode = *iterator;
        pLogger->info("TaskTreeModel::ClearNodeEntriesByDateKey - Located root node associated with date key \"{0}\"",
            parentNode->GetProjectName());

        wxDataViewItemArray itemsRemoved;
        auto& children = parentNode->GetChildren();
        auto count = children.size();
        for (auto it = children.begin(); it != children.end(); ++it) {
            auto child = it->get();
            itemsRemoved.Add(wxDataViewItem((void*) it->get()));
        }

        pLogger->info("TaskTreeModel::ClearNodeEntriesByDateKey - Removed children {0}", count);
        parentNode->GetChildren().clear();

        wxDataViewItem parent((void*) parentNode);
        ItemsDeleted(parent, itemsRemoved);
    }
}

void TaskTreeModel::InsertChildNode(const std::string& date, repos::TaskRepositoryModel& taskModel)
{
    pLogger->info("TaskTreeModel::InsertChildNodes - Begin append of task for \"{0}\"", date);
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == date; });

    if (iterator != pRoots.end()) {
        auto parentNode = *iterator;
        auto childNode = new TaskTreeModelNode(parentNode,
            taskModel.ProjectName,
            taskModel.CategoryName,
            taskModel.GetDuration(),
            taskModel.GetTrimmedDescription(),
            taskModel.TaskId);
        parentNode->Append(childNode);

        wxDataViewItem child((void*) childNode);
        wxDataViewItem parent((void*) parentNode);
        ItemAdded(parent, child);
    }
}

void TaskTreeModel::InsertChildNodes(const std::string& date, std::vector<repos::TaskRepositoryModel> models)
{
    pLogger->info("TaskTreeModel::InsertChildNodes - Begin insertion of tasks for \"{0}\"", date);
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == date; });

    if (iterator != pRoots.end()) {
        auto parentNode = *iterator;

        pLogger->info("TaskTreeModel::InsertChildNodes - Located root node associated with date key \"{0}\"",
            parentNode->GetProjectName());

        wxDataViewItemArray itemsAdded;
        for (auto& model : models) {
            auto childNode = new TaskTreeModelNode(parentNode,
                model.ProjectName,
                model.CategoryName,
                model.GetDuration(),
                model.GetTrimmedDescription(),
                model.TaskId);
            parentNode->Append(childNode);

            wxDataViewItem child((void*) childNode);
            itemsAdded.Add(child);
        }

        wxDataViewItem parent((void*) parentNode);
        ItemsAdded(parent, itemsAdded);

        pLogger->info("TaskTreeModel::InsertChildNodes - Number of inserted children \"{0}\"", models.size());
    }
}

void TaskTreeModel::InsertRootAndChildNodes(const std::string& date, std::vector<repos::TaskRepositoryModel> models)
{
    pLogger->info("TaskTreeModel::InsertRootAndChildNodes - Begin insertion of tasks for \"{0}\"", date);

    auto rootDateNode = new TaskTreeModelNode(nullptr, date);
    pRoots.push_back(rootDateNode);

    for (auto& model : models) {
        auto node = new TaskTreeModelNode(rootDateNode,
            model.ProjectName,
            model.CategoryName,
            model.GetDuration(),
            model.GetTrimmedDescription(),
            model.TaskId);
        rootDateNode->Append(node);
    }

    wxDataViewItem child((void*) rootDateNode);
    wxDataViewItem parent((void*) nullptr);
    ItemAdded(parent, child);

    pLogger->info(
        "TaskTreeModel::InsertRootAndChildNodes - Inserted \"{0}\" children for root node {1}", models.size(), date);
}

wxDataViewItem TaskTreeModel::TryExpandTodayDateNode(const std::string& todayDate)
{
    pLogger->info("TaskTreeModel::TryExpandTodayDateNode - Locate root node with date: \"{0}\"", todayDate);
    auto iterator = std::find_if(
        pRoots.begin(), pRoots.end(), [&](TaskTreeModelNode* ptr) { return ptr->GetProjectName() == todayDate; });

    if (iterator != pRoots.end()) {
        pLogger->info("TaskTreeModel::TryExpandTodayDateNode - Found root node to expand");
        auto node = *iterator;
        return wxDataViewItem((void*) node);
    }

    return wxDataViewItem();
}
} // namespace tks::UI
