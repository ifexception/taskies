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

#include "columnexportmodel.h"

namespace tks::Services::Export
{
ColumnExportModel::ColumnExportModel(const std::string& column,
    const std::string& originalColumn,
    int order)
    : Column(column)
    , OriginalColumn(originalColumn)
    , Order(order)
{
}

std::vector<ColumnExportModel> BuildFromList(const std::vector<UI::ColumnListItemModel>& columns)
{
    std::vector<ColumnExportModel> columnExportModels;
    for (const auto& column : columns) {
        ColumnExportModel model(column.Column, column.OriginalColumn, column.Order);
        columnExportModels.push_back(model);
    }

    return columnExportModels;
}

std::vector<ColumnExportModel> BuildFromPreset(
    const std::vector<Core::Configuration::PresetColumnSettings>& columns)
{
    std::vector<ColumnExportModel> columnExportModels;
    for (const auto& column : columns) {
        ColumnExportModel model(column.Column, column.OriginalColumn, column.Order);
        columnExportModels.push_back(model);
    }

    return columnExportModels;
}

} // namespace tks::Services::Export
