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

#include <memory>
#include <string>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dataview.h>

#include <date/date.h>

#include <spdlog/logger.h>

#include "tasktreemodelnode.h"

#include "../../services/tasks/taskviewmodel.h"

namespace tks::UI
{
class TaskTreeModel : public wxDataViewModel
{
public:
    enum { Col_Project = 0, Col_Category, Col_Duration, Col_Description, Col_Id, Col_Max };

    TaskTreeModel(const std::vector<std::string>& weekDates,
        std::shared_ptr<spdlog::logger> logger);
    ~TaskTreeModel();

    unsigned int GetColumnCount() const override;
    wxString GetColumnType(unsigned int col) const override;
    void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
    bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override;
    bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override;
    wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    bool IsContainer(const wxDataViewItem& item) const override;
    unsigned int GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const override;

    void Delete(const wxDataViewItem& item);
    void DeleteChild(const std::string& date, const std::int64_t taskId);

    void ChangeChild(const std::string& date, Services::TaskViewModel& taskModel);
    //void ChangeContainerLabelWithTime(const std::string date, const std::string time);

    void Clear();
    void ClearAll();
    void ClearNodeEntriesByDateKey(const std::string& date);

    void InsertChildNode(const std::string& date, Services::TaskViewModel& taskModel);
    void InsertChildNodes(const std::string& date, std::vector<Services::TaskViewModel> models);
    void InsertRootAndChildNodes(const std::string& date, std::vector<Services::TaskViewModel> models);

    wxDataViewItem TryExpandTodayDateNode(const std::string& todaysDate);
    wxDataViewItemArray TryCollapseDateNodes();

    wxDataViewItemArray TryExpandAllDateNodes(const std::vector<std::string>& dates);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    std::vector<std::unique_ptr<TaskTreeModelNode>> pRoots;
};
} // namespace tks::UI
