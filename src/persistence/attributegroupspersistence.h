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

#include "../models/attributegroupmodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct AttributeGroupsPersistence final : public PersistenceBase {
    AttributeGroupsPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~AttributeGroupsPersistence() = default;

    Common::SqliteResult Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::AttributeGroupModel>& attributeGroupModels) const;
    Common::SqliteResult FilterByStaticFlag(
        /*out*/ std::vector<Model::AttributeGroupModel>& attributeGroupModels) const;
    Common::SqliteResult GetById(const std::int64_t attributeGroupId,
        /*out*/ Model::AttributeGroupModel& attributeGroupModel) const;
    Common::SqliteResult Create(std::int64_t& attributeGroupId,
        const Model::AttributeGroupModel& attributeGroupModel) const;
    Common::SqliteResult Update(Model::AttributeGroupModel& attributeGroupModel) const;
    Common::SqliteResult Delete(const std::int64_t attributeGroupId) const;
    Common::SqliteResult CheckAttributeGroupAttributeValuesUsage(
        const std::int64_t attributeGroupId,
        bool& value) const;
    Common::SqliteResult CheckAttributeGroupAttributesUsage(const std::int64_t attributeGroupId,
        bool& value) const;
    Common::SqliteResult CheckAttributeGroupStaticAttributesUsage(
        const std::int64_t attributeGroupId,
        bool& value) const;
    Common::SqliteResult UnsetDefault() const;
    Common::SqliteResult SelectDefault(Model::AttributeGroupModel& attributeGroupModel) const;

    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static std::string filter;
    static std::string filterStatic;
    static std::string getById;
    static std::string create;
    static std::string update;
    static std::string updateIfInUse;
    static std::string isActive;
    static std::string checkAttributeGroupAttributeValuesUsage;
    static std::string checkAttributeGroupAttributesUsage;
    static std::string checkAttributeGroupStaticAttributesUsage;
    static std::string unsetDefault;
    static std::string selectDefault;
};
} // namespace tks::Persistence
