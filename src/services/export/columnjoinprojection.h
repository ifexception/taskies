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

#include <string>

#include "../../common/enums.h"

namespace tks::Services::Export
{
struct ColumnJoinProjection {
    std::string TableName;
    std::string IdColumn;
    JoinType Join;
    bool IsSecondLevelJoin;

    ColumnJoinProjection();
    ColumnJoinProjection(std::string tableName, std::string idColumn, bool isSecondLevelJoin = false);
    ColumnJoinProjection(std::string tableName, std::string idColumn, JoinType join, bool isSecondLevelJoin = false);
};
}
