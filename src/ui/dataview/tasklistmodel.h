// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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
#include <memory>
#include <string>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dataview.h>

#include <spdlog/logger.h>

#include "../../services/tasks/taskviewmodel.h"
#include "../../services/tasks/tasksservice.h"

#include "../../common/constants.h"

namespace tks::UI
{
struct TaskListItemModel final {
    TaskListItemModel(const std::string& projectName,
        const std::string& categoryName,
        const std::string& duration,
        const std::string& description,
        std::int64_t taskId);

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

    std::string mProjectName;
    std::string mDuration;
    std::string mCategoryName;
    std::string mDescription;
    std::int64_t mTaskId;
};

class TaskListModel final : public wxDataViewVirtualListModel
{
public:
    enum { Col_Project = 0, Col_Category, Col_Duration, Col_Description, Col_Id, Col_Max };

    TaskListModel(std::shared_ptr<spdlog::logger> logger);
    virtual ~TaskListModel() = default;

    virtual void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override;
    virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override;
    virtual bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override;
    virtual unsigned int GetCount() const override;

    void Append(const Services::TaskViewModel& model);
    void AppendMany(const std::vector<Services::TaskViewModel>& models);
    void ChangeItem();
    void DeleteItem();
    void Clear();

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::vector<TaskListItemModel> mListItemModels;
};
} // namespace tks::UI
