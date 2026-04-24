// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2026 Szymon Welgus
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

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

#include "staticattributegroupviewmodel.h"

namespace tks::Services
{
struct StaticAttributeGroupsService final : public Persistence::PersistenceBase {
    StaticAttributeGroupsService() = delete;
    StaticAttributeGroupsService(const StaticAttributeGroupsService&) = delete;
    StaticAttributeGroupsService(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~StaticAttributeGroupsService();

    StaticAttributeGroupsService& operator=(
        const StaticAttributeGroupsService&) = delete;

    SqliteResult FilterByStaticFlagAndWithValueCounts(
        /*out*/ std::vector<StaticAttributeGroupViewModel>& staticAttributeGroupViewModels) const;

    static std::string filterStaticWithValueCounts;
};
} // namespace tks::Services
