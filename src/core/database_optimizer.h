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

#include <memory>
#include <string>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include <sqlite3.h>

#include "../common/results/sqliteresult.h"

namespace tks::Core
{
class DatabaseOptimizer final
{
public:
    DatabaseOptimizer() = delete;
    DatabaseOptimizer(const DatabaseOptimizer&) = delete;
    DatabaseOptimizer(std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath);
    ~DatabaseOptimizer();

    const DatabaseOptimizer& operator=(const DatabaseOptimizer&) = delete;

    SqliteResult Optimize();

private:
    SqliteResult Initialize();

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;
    sqlite3* pDb;
};
} // namespace tks::Core
