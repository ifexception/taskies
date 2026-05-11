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

#include "../../common/enums.h"

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

namespace tks::Services
{
struct FilterEntityModel final {
    EditListEntityType Type;
    std::int64_t EntityId;
    std::string EntityName;
    std::int32_t EntityDateModified;
    std::vector<std::string> Metadata;

    FilterEntityModel(EditListEntityType type);
    FilterEntityModel(EditListEntityType type,
        std::int64_t entityId,
        const std::string& entityName,
        std::int32_t entityDateModified,
        std::vector<std::string> metadata);
    ~FilterEntityModel() = default;
};

struct FilterEntityService final : public Persistence::PersistenceBase {
    FilterEntityService() = delete;
    FilterEntityService(const FilterEntityService&) = delete;
    FilterEntityService(const std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    virtual ~FilterEntityService();

    FilterEntityService& operator=(const FilterEntityService&) = delete;

    SqliteResult FilterClients(const std::string& searchTerm,
        std::vector<FilterEntityModel>& models);

    SqliteResult FilterProjects(const std::string& searchTerm,
        std::vector<FilterEntityModel>& models);

    static std::string filterClients;
    static std::string filterProjects;
};
} // namespace tks::Services
