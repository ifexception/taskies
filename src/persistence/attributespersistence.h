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

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "base/persistencebase.h"

#include "../models/attributemodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct AttributesPersistence final : public PersistenceBase {
    AttributesPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~AttributesPersistence() = default;

    SqliteResult Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::AttributeModel>& attributeModels) const;
    SqliteResult FilterByAttributeGroupId(const std::int64_t attributeGroupId,
        /*out*/ std::vector<Model::AttributeModel>& attributeModels) const;
    SqliteResult FilterByAttributeGroupIdAndIsStatic(const std::int64_t attributeGroupId,
        /*out*/ std::vector<Model::AttributeModel>& attributeModels) const;
    SqliteResult GetById(const std::int64_t attributeId,
        /*out*/ Model::AttributeModel& attributeModel) const;
    SqliteResult Create(std::int64_t& attributeId,
        const Model::AttributeModel& attributeModel) const;
    SqliteResult Update(Model::AttributeModel attributeModel) const;
    SqliteResult UpdateIfInUse(Model::AttributeModel attributeModel) const;
    SqliteResult Delete(const std::int64_t attributeId) const;
    SqliteResult CheckAttributeUsage(const std::int64_t attributeId, bool& value) const;

    std::shared_ptr<spdlog::logger> pLogger;

    static std::string filter;
    static std::string filterByAttributeGroupId;
    static std::string filterByAttributeGroupIdAndIsStatic;
    static std::string getById;
    static std::string create;
    static std::string update;
    static std::string updateIfInUse;
    static std::string isActive;
    static std::string checkUsage;
};
} // namespace tks::Persistence
