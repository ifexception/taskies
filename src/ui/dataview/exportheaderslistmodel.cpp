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

#include "exportheaderslistmodel.h"

namespace tks::UI
{
ExportHeaderListItemModel::ExportHeaderListItemModel(const std::string& header, int orderIndex)
    : Toggled(false)
    , Header(header)
    , OriginalHeader(header)
    , OrderIndex(orderIndex)
{
}

// #############################################

ExportHeadersListModel::ExportHeadersListModel(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mListItemModels()
{
}

void ExportHeadersListModel::GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const
{
    switch (col) {
    case Col_Toggled: {
        variant = mListItemModels[row].Toggled;
        break;
    }
    case Col_Header:
        variant = mListItemModels[row].Header;
        break;
    case Col_OrderIndex:
        variant = (long) mListItemModels[row].OrderIndex;
        break;
    case Col_Max:
    default:
        pLogger->info("ExportHeadersListModel::GetValueByRow - Invalid column selected");
        break;
    }
}

bool ExportHeadersListModel::GetAttrByRow(unsigned int row, unsigned int col, wxDataViewItemAttr& attr) const
{
    return true;
}

bool ExportHeadersListModel::SetValueByRow(const wxVariant& variant, unsigned int row, unsigned int col)
{
    switch (col) {
    case Col_Toggled:
        mListItemModels[row].Toggled = variant.GetBool();
        return true;
    case Col_Header:
        mListItemModels[row].Header = variant.GetString().ToStdString();
        return true;
    case Col_OrderIndex:
        mListItemModels[row].OrderIndex = (static_cast<int>(variant.GetInteger()));
        return true;
    case Col_Max:
    default:
        pLogger->info("ExportHeadersListModel::SetValue - Invalid column selected");
        break;
    }

    return false;
}

unsigned int ExportHeadersListModel::GetCount() const
{
    return mListItemModels.size();
}

void ExportHeadersListModel::Append(const std::string& headerName, int orderIndex)
{
    ExportHeaderListItemModel model(headerName, orderIndex);
    mListItemModels.push_back(model);

    RowAppended();
}

void ExportHeadersListModel::DeleteItems(const wxDataViewItemArray& items)
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

void ExportHeadersListModel::ChangeItem(const wxDataViewItem& item, const std::string& newItem)
{
    unsigned int row = GetRow(item);
    if (newItem.length() > 0) {
        mListItemModels[row].Header = newItem;

        RowChanged(row);
    }
}

void ExportHeadersListModel::MoveItem(const wxDataViewItem& item, bool up)
{
    pLogger->info("ExportHeadersListModel::MoveItem - Begin move item");
    unsigned int row = GetRow(item);
    if (row != 0 && up) {
        pLogger->info("ExportHeadersListModel::MoveItem - Moving header \"{0}\" up", mListItemModels[row].Header);

        auto modelAtRow = mListItemModels[row];
        mListItemModels.erase(mListItemModels.begin() + row);
        modelAtRow.OrderIndex--;
        modelAtRow.Toggled = false;
        RowDeleted(row);

        unsigned int rowAbove = --row;
        mListItemModels[rowAbove].OrderIndex++;
        mListItemModels.insert(mListItemModels.begin() + rowAbove, modelAtRow);

        RowInserted(rowAbove);
    }

    if (row != 0 && row != (mListItemModels.size() - 1) && !up) {
        pLogger->info("ExportHeadersListModel::MoveItem - Moving header \"{0}\" down", mListItemModels[row].Header);
        auto modelAtRow = mListItemModels[row];
        mListItemModels.erase(mListItemModels.begin() + row);
        modelAtRow.OrderIndex++;
        modelAtRow.Toggled = false;
        RowDeleted(row);

        unsigned int rowBelow = ++row;
        mListItemModels[rowBelow].OrderIndex--;
        mListItemModels.insert(mListItemModels.begin() + rowBelow, modelAtRow);

        RowInserted(rowBelow);
    }
}

std::vector<std::string> ExportHeadersListModel::GetSelectedHeaders()
{
    std::vector<std::string> headers;
    for (const auto& listItem : mListItemModels) {
        if (listItem.Toggled) {
            pLogger->info(
                "ExportHeadersListModel::GetSelectedHeaders - Found toggled header with name \"{0}\"", listItem.Header);
            headers.push_back(listItem.Header);
        }
    }

    return headers;
}

std::vector<ExportHeaderListItemModel> ExportHeadersListModel::GetHeadersToExport() const
{
    return mListItemModels;
}
} // namespace tks::UI
