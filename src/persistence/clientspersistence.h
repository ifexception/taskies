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
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "base/persistencebase.h"

#include "../models/clientmodel.h"

#include "../common/results/sqliteresult.h"

namespace tks::Persistence
{
struct ClientsPersistence final : public PersistenceBase {
    ClientsPersistence(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    virtual ~ClientsPersistence() = default;

    Common::SqliteResult Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::ClientModel>& clientModels) const;
    Common::SqliteResult FilterByEmployerId(const std::int64_t employerId,
        /*out*/ std::vector<Model::ClientModel>& clientModels) const;
    Common::SqliteResult GetById(const std::int64_t clientId,
        /*out*/ Model::ClientModel& clientModel) const;
    Common::SqliteResult Create(std::int64_t& clientId,
        const Model::ClientModel& clientModel) const;
    Common::SqliteResult Update(const Model::ClientModel& clientModel) const;
    Common::SqliteResult Delete(const std::int64_t clientId) const;

    std::shared_ptr<spdlog::logger> pLogger;

    static std::string filter;
    static std::string filterByEmployerId;
    static std::string getById;
    static std::string create;
    static std::string update;
    static std::string isActive;
};
} // namespace tks::Persistence
