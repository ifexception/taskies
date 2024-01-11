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

TaskListModel::TaskListModel(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mListItemModels()
{
}

void TaskListModel::GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const
{
    switch (col) {
    case Col_Project:
        variant = mListItemModels[row].GetProjectName();
        break;
    case Col_Category:
        variant = mListItemModels[row].GetCategoryName();
        break;
    case Col_Duration:
        variant = mListItemModels[row].GetDuration();
        break;
    case Col_Description:
        variant = mListItemModels[row].GetDescription();
        break;
    case Col_Id:
        variant = (long) mListItemModels[row].GetTaskId();
        break;
    case Col_Max:
    default:
        pLogger->info("TaskListModel::GetValueByRow - Invalid column selected");
        break;
    }
}

bool TaskListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const
{
    return false;
}

bool TaskListModel::SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col)
{
    switch (col) {
    case Col_Project:
        mListItemModels[row].SetProjectName(variant.GetString().ToStdString());
        break;
    case Col_Duration:
        mListItemModels[row].SetDuration(variant.GetString().ToStdString());
        break;
    case Col_Category:
        mListItemModels[row].SetCategoryName(variant.GetString().ToStdString());
        break;
    case Col_Description:
        mListItemModels[row].SetDescription(variant.GetString().ToStdString());
        break;
    case Col_Id:
        mListItemModels[row].SetTaskId(static_cast<std::int64_t>(variant.GetInteger()));
        break;
    case Col_Max:
    default:
        pLogger->info("TaskListModel::SetValue - Invalid column selected");
        break;
    }

    return false;
}

void TaskListModel::Append(const repos::TaskRepositoryModel& model)
{
    TaskListItemModel listModel(
        model.ProjectName, model.CategoryName, model.GetDuration(), model.Description, model.TaskId);
    mListItemModels.push_back(listModel);

    RowAppended();
}

void TaskListModel::AppendMany(const std::vector<repos::TaskRepositoryModel>& models)
{
    for (const auto& model : models) {
        Append(model);
    }
}

void TaskListModel::ChangeItem() {}

void TaskListModel::DeleteItem() {}
} // namespace tks::UI
