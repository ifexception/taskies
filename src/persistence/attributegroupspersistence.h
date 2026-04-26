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

#include "base/persistencebase.h"

#include "../models/attributegroupmodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct AttributeGroupsPersistence final : public PersistenceBase {
    AttributeGroupsPersistence() = delete;
    AttributeGroupsPersistence(const AttributeGroupsPersistence&) = delete;
    AttributeGroupsPersistence(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    virtual ~AttributeGroupsPersistence();

    AttributeGroupsPersistence& operator=(const AttributeGroupsPersistence&) = delete;

    SqliteResult Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::AttributeGroupModel>& attributeGroupModels) const;
    SqliteResult FilterByStaticFlag(
        /*out*/ std::vector<Model::AttributeGroupModel>& attributeGroupModels) const;
    SqliteResult GetById(const std::int64_t attributeGroupId,
        /*out*/ Model::AttributeGroupModel& attributeGroupModel) const;
    SqliteResult Create(std::int64_t& attributeGroupId,
        const Model::AttributeGroupModel& attributeGroupModel) const;
    SqliteResult Update(Model::AttributeGroupModel& attributeGroupModel) const;
    SqliteResult Delete(const std::int64_t attributeGroupId) const;
    SqliteResult CheckAttributeGroupAttributeValuesUsage(
        const std::int64_t attributeGroupId,
        bool& value) const;
    SqliteResult CheckAttributeGroupAttributesUsage(const std::int64_t attributeGroupId,
        bool& value) const;
    SqliteResult CheckAttributeGroupStaticAttributesUsage(
        const std::int64_t attributeGroupId,
        bool& value) const;
    SqliteResult UnsetDefault() const;
    SqliteResult SelectDefault(Model::AttributeGroupModel& attributeGroupModel) const;

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
