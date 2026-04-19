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

#include "../models/taskattributevaluemodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct TaskAttributeValuesPersistence final : public PersistenceBase {
    TaskAttributeValuesPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~TaskAttributeValuesPersistence() = default;

    SqliteResult Create(std::int64_t& taskAttributeValueId,
        Model::TaskAttributeValueModel& taskAttributeValueModel) const;
    SqliteResult CreateMany(
        std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels) const;
    SqliteResult GetByTaskId(const std::int64_t taskId,
        /*out*/ std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels) const;
    SqliteResult DeleteByTaskId(const std::int64_t taskId) const;
    SqliteResult Update(
        const Model::TaskAttributeValueModel& taskAttributeValueModel) const;
    SqliteResult UpdateMultiple(
        const std::vector<Model::TaskAttributeValueModel>& taskAttributeValueModels) const;

    std::shared_ptr<spdlog::logger> pLogger;

    static std::string getByTaskId;
    static std::string create;
    static std::string deleteByTaskId;
    static std::string update;
};
} // namespace tks::Persistence
