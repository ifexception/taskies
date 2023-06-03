// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

#include <string>

#include <sqlite3.h>
#include <spdlog/spdlog.h>

namespace tks::Core
{
class Environment;
class Configuration;

struct Migration {
    std::string name;
    std::string sql;
};

class DatabaseMigration
{
public:
    DatabaseMigration(std::shared_ptr<Environment> env,
        std::shared_ptr<Configuration> cfg,
        std::shared_ptr<spdlog::logger> logger);
    ~DatabaseMigration();

    bool Migrate();

private:
    void CreateMigrationHistoryTable();
    bool MigrationExists(const std::string& name);

    sqlite3* pDb;
    std::shared_ptr<Environment> pEnv;
    std::shared_ptr<Configuration> pCfg;
    std::shared_ptr<spdlog::logger> pLogger;

    static const std::string BeginTransactionQuery;
    static const std::string CommitTransactionQuery;
    static const std::string CreateMigrationHistoryQuery;
    static const std::string SelectMigrationExistsQuery;
    static const std::string InsertMigrationHistoryQuery;
};
} // namespace tks::Core
