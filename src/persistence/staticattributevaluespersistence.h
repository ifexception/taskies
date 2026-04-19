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

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "base/persistencebase.h"

#include "../models/staticattributevaluemodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct StaticAttributeValuesPersistence final : public PersistenceBase {
    StaticAttributeValuesPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~StaticAttributeValuesPersistence() = default;

    SqliteResult Create(std::int64_t& staticAttributeValueId,
        const Model::StaticAttributeValueModel& staticAttributeValueModel) const;
    SqliteResult CreateMultiple(
        const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;
    SqliteResult FilterByAttributeGroupId(const std::int64_t attributeGroupId,
        /*out*/ std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;
    SqliteResult Update(
        const Model::StaticAttributeValueModel& staticAttributeValueModel) const;
    SqliteResult UpdateMultiple(
        const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;
    SqliteResult Delete(const std::vector<std::int64_t>& staticAttributeValueIds) const;
    SqliteResult CheckUsage(const std::vector<std::int64_t>& attributeIds,
        bool& value) const;

    std::shared_ptr<spdlog::logger> pLogger;

    static std::string create;
    static std::string filterByAttributeGroupId;
    static std::string update;
    static std::string isActive;
    static std::string checkUsage;
};
} // namespace tks::Persistence
