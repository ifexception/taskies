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

#include "columnlistmodel.h"

#include <algorithm>

namespace tks::UI
{
ColumnListModel::ColumnListModel(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mListItemModels()
{
}

void ColumnListModel::GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const
{
    switch (col) {
    case Col_Toggled: {
        variant = mListItemModels[row].Toggled;
        break;
    }
    case Col_Column:
        variant = mListItemModels[row].Column;
        break;
    case Col_Order:
        variant = (long) mListItemModels[row].Order;
        break;
    case Col_Max:
    default:
        pLogger->info("ColumnListModel::GetValueByRow - Invalid column selected");
        break;
    }
}

bool ColumnListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const
{
    return true;
}

bool ColumnListModel::SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col)
{
    switch (col) {
    case Col_Toggled:
        mListItemModels[row].Toggled = variant.GetBool();
        return true;
    case Col_Column:
        mListItemModels[row].Column = variant.GetString().ToStdString();
        return true;
    case Col_Order:
        mListItemModels[row].Order = (static_cast<int>(variant.GetInteger()));
        return true;
    case Col_Max:
    default:
        pLogger->info("ColumnListModel::SetValue - Invalid column selected");
        break;
    }

    return false;
}

unsigned int ColumnListModel::GetCount() const
{
    return mListItemModels.size();
}

void ColumnListModel::Append(const std::string& columnName)
{
    int order = 0;

    // check if we are appending to existing list
    if (!mListItemModels.empty()) {
        // overwrite the order as appending could be starting at 0
        order = mListItemModels.back().Order;
        order++;
    }

    ColumnListItemModel model(columnName, order);
    mListItemModels.push_back(model);

    RowAppended();
}

void ColumnListModel::DeleteItems(const wxDataViewItemArray& items)
{
    wxArrayInt rows;
    for (auto i = 0; i < items.GetCount(); i++) {
        unsigned int row = GetRow(items[i]);
        if (row < mListItemModels.size()) {
            rows.Add(row);
        }
    }

    for (auto i = 0; i < items.GetCount(); i++) {
        mListItemModels.erase(mListItemModels.begin() + rows[i]);
    }

    RowsDeleted(rows);
}

void ColumnListModel::ChangeItem(const wxDataViewItem& item, const std::string& newItem)
{
    unsigned int row = GetRow(item);
    if (newItem.length() > 0) {
        mListItemModels[row].Column = newItem;

        RowChanged(row);
    }
}

void ColumnListModel::MoveItem(const wxDataViewItem& item, bool asc)
{
    pLogger->info("ColumnListModel::MoveItem - Begin move item");
    unsigned int row = GetRow(item);
    if (row != 0 && asc) {
        pLogger->info("ColumnListModel::MoveItem - Moving column \"{0}\" up", mListItemModels[row].Column);

        auto modelAtRow = mListItemModels[row];
        modelAtRow.Order -= 1;
        modelAtRow.Toggled = false;

        unsigned int rowAbove = row - 1;
        mListItemModels[rowAbove].Order += 1;

        mListItemModels.erase(mListItemModels.begin() + row);
        RowDeleted(row);

        mListItemModels.insert(mListItemModels.begin() + rowAbove, modelAtRow);
        RowInserted(rowAbove);
    }

    if (row != (mListItemModels.size() - 1) && !asc) {
        pLogger->info("ColumnListModel::MoveItem - Moving column \"{0}\" down", mListItemModels[row].Column);

        auto modelAtRow = mListItemModels[row];
        modelAtRow.Order += 1;
        modelAtRow.Toggled = false;

        unsigned int rowBelow = row + 1;
        mListItemModels[rowBelow].Order -= 1;

        mListItemModels.erase(mListItemModels.begin() + row);
        RowDeleted(row);

        mListItemModels.insert(mListItemModels.begin() + rowBelow, modelAtRow);
        RowInserted(rowBelow);
    }

    std::sort(mListItemModels.begin(),
        mListItemModels.end(),
        [&](const ColumnListItemModel& lhs, const ColumnListItemModel& rhs) { return lhs.Order < rhs.Order; });
}

void ColumnListModel::AppendStagingItem(const std::string& column, const std::string& originalColumn, int order)
{
    ColumnListItemModel model(column, originalColumn, order);
    mListItemModelsStaging.push_back(model);
}

void ColumnListModel::AppendFromStaging()
{
    // clang-format off
    std::sort(
        mListItemModelsStaging.begin(),
        mListItemModelsStaging.end(),
        [&](
            const ColumnListItemModel& lhs,
            const ColumnListItemModel& rhs
        ) {
                return lhs.Order < rhs.Order;
        });
    // clang-format on

    for (const auto& model : mListItemModelsStaging) {
        mListItemModels.push_back(model);

        RowAppended();
    }

    mListItemModelsStaging.clear();
}

std::vector<ColumnListItemModel> ColumnListModel::GetSelectedColumns()
{
    std::vector<ColumnListItemModel> selectedColumns;
    for (const auto& listItem : mListItemModels) {
        if (listItem.Toggled) {
            pLogger->info(
                "ColumnListModel::GetSelectedColumns - Found toggled column with name \"{0}\"", listItem.Column);
            selectedColumns.push_back(listItem);
        }
    }

    return selectedColumns;
}

std::vector<ColumnListItemModel> ColumnListModel::GetColumns() const
{
    return mListItemModels;
}

void ColumnListModel::Clear()
{
    mListItemModels.clear();
    Reset(0);
}
} // namespace tks::UI
