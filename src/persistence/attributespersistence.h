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

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <sqlite3.h>

#include "../models/attributemodel.h"

namespace tks::Persistence
{
class AttributesPersistence final
{
public:
    AttributesPersistence(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath);
    ~AttributesPersistence();

    int Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::AttributeModel>& attributeModels);
    int GetById(const std::int64_t attributeId,
        /*out*/ Model::AttributeModel& attributeModel);
    std::int64_t Create(const Model::AttributeModel& attributeModel);
    int Update(Model::AttributeModel attributeModel);
    int Delete(const std::int64_t attributeId);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    sqlite3* pDb;

    std::string mClassName;

    static const std::string filter;
    static const std::string getById;
    static const std::string create;
    static const std::string update;
    static const std::string isActive;
};
}
