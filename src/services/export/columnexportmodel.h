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

#include <vector>

#include "../../core/configuration.h"

#include "../../ui/dataview/columnlistitemmodel.h"

namespace tks::Services::Export
{
struct ColumnExportModel {
    ColumnExportModel(const std::string& column, const std::string& originalColumn, int order);

    std::string Column;
    std::string OriginalColumn;
    int Order;
};

std::vector<ColumnExportModel> BuildFromList(const std::vector<UI::ColumnListItemModel>& columns);

std::vector<ColumnExportModel> BuildFromPreset(
    const std::vector<Core::Configuration::PresetColumnSettings>& columns);
} // namespace tks::Services::Export
