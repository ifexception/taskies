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

#include <memory>

#include <spdlog/logger.h>

#include "../../ui/dataview/columnlistitemmodel.h"

#include "availablecolumns.h"
#include "projection.h"
#include "columnjoinprojection.h"

namespace tks::Services::Export
{
class ProjectionBuilder final
{
public:
    ProjectionBuilder() = delete;
    ProjectionBuilder(std::shared_ptr<spdlog::logger> logger);
    ~ProjectionBuilder() = default;

    std::vector<Projection> BuildProjections(const std::vector<UI::ColumnListItemModel>& columns);
    std::vector<ColumnJoinProjection> BuildJoinProjections(const std::vector<UI::ColumnListItemModel>& columns);

private:
    std::shared_ptr<spdlog::logger> pLogger;

    std::vector<AvailableColumn> mAvailableColumns;

    Projection BuildProjection(const UI::ColumnListItemModel& column, const AvailableColumn& availableColumn);
    void SortProjectionsByOrder(std::vector<Projection>& projections);
    ColumnJoinProjection BuildRequiredProjectTableJoinProjection();
    ColumnJoinProjection BuildJoinProjection(const UI::ColumnListItemModel& column,
        const AvailableColumn& availableColumn);
};
} // namespace tks::Services::Export
