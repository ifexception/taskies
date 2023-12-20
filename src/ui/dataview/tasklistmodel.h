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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dataview.h>

constexpr auto INITIAL_NUMBER_OF_ITEMS = 32;

namespace tks::UI
{
struct TaskListItemModel final
{

};

class TaskListModel final : public wxDataViewVirtualListModel
{
public:
    enum { Col_Project = 0, Col_Category, Col_Duration, Col_Description, Col_Id, Col_Max };
    TaskListModel();
    virtual ~TaskListModel() = default;

    void Append();
    void ChangeItem();
    void DeleteItem();

private:
};
} // namespace tks::UI
