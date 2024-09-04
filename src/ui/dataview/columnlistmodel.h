// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "../../common/constants.h"
#include "columnlistitemmodel.h"

namespace tks::UI
{
class ColumnListModel : public wxDataViewVirtualListModel
{
public:
    enum { Col_Toggled = 0, Col_Column, Col_Order, Col_Max };

    ColumnListModel() = delete;
    ColumnListModel(const ColumnListModel&) = delete;
    ColumnListModel(std::shared_ptr<spdlog::logger> logger);
    virtual ~ColumnListModel() = default;

    const ColumnListModel& operator=(const ColumnListModel&) = delete;

    virtual void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override;
    virtual bool GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const override;
    virtual bool SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col) override;
    virtual unsigned int GetCount() const override;

    void Append(const std::string& columnName, int order);
    void DeleteItems(const wxDataViewItemArray& items);
    void ChangeItem(const wxDataViewItem& item, const std::string& newItem);
    void MoveItem(const wxDataViewItem& item, bool asc = true);

    void AppendStagingItem(const std::string& column, const std::string& originalColumn, int order);
    void AppendFromStaging();

    std::vector<ColumnListItemModel> GetSelectedColumns();

    std::vector<ColumnListItemModel> GetColumnsToExport() const;
    void Clear();

private:
    std::shared_ptr<spdlog::logger> pLogger;

    std::vector<ColumnListItemModel> mListItemModels;
    std::vector<ColumnListItemModel> mListItemModelsStaging;
};
} // namespace tks::UI
