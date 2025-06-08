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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include <sqlite3.h>

#include "../models/staticattributevaluemodel.h"

namespace tks::Persistence
{
struct StaticAttributeValuesPersistence final {
    StaticAttributeValuesPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    ~StaticAttributeValuesPersistence();

    std::int64_t Create(const Model::StaticAttributeValueModel& staticAttributeValueModel) const;
    int CreateMultiple(
        const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;
    int FilterByAttributeGroupId(const std::int64_t attributeGroupId,
        /*out*/ std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;
    int Update(const Model::StaticAttributeValueModel& staticAttributeValueModel) const;
    int UpdateMultiple(
        const std::vector<Model::StaticAttributeValueModel>& staticAttributeValueModels) const;

    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    static std::string create;
    static std::string filterByAttributeGroupId;
    static std::string update;
};
} // namespace tks::Persistence
