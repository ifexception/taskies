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

#include "../models/employermodel.h"

#include "../common/results/sqliteresult.h"

namespace tks
{
namespace Persistence
{
struct EmployersPersistence final : public PersistenceBase {
    EmployersPersistence() = delete;
    EmployersPersistence(const EmployersPersistence&) = delete;
    EmployersPersistence(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    virtual ~EmployersPersistence();

    EmployersPersistence& operator=(const EmployersPersistence&) = delete;

    SqliteResult Filter(const std::string& searchTerm,
        /*out*/ std::vector<Model::EmployerModel>& employerModels) const;
    SqliteResult GetById(const std::int64_t employerId,
        /*out*/ Model::EmployerModel& employerModel) const;
    SqliteResult Create(/*out*/ std::int64_t& employerId,
        const Model::EmployerModel& employerModel) const;
    SqliteResult Update(const Model::EmployerModel& employerModel) const;
    SqliteResult Delete(const std::int64_t employerId) const;
    SqliteResult UnsetDefault() const;
    SqliteResult SelectDefault(/*out*/ Model::EmployerModel& employerModel) const;

    static std::string filter;
    static std::string getById;
    static std::string create;
    static std::string update;
    static std::string isActive;
    static std::string unsetDefault;
    static std::string selectDefault;
};
} // namespace Persistence
} // namespace tks
