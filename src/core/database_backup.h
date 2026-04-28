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
class DatabaseBackup final
{
public:
    DatabaseBackup() = delete;
    DatabaseBackup(const DatabaseBackup&) = delete;
    DatabaseBackup(std::shared_ptr<spdlog::logger> logger);
    ~DatabaseBackup();

    const DatabaseBackup& operator=(const DatabaseBackup&) = delete;

    SqliteResult Backup();
    SqliteResult Restore();

    void SetSourceDatabaseFilePath(const std::string& sourceDatabaseFilePath);
    void SetDestinationDatabaseFilePath(const std::string& destinationDatabaseFilePath);

private:
    SqliteResult Initialize();

    SqliteResult PerformBackup();

    void CleanUp();

    std::shared_ptr<spdlog::logger> pLogger;

    sqlite3* pDb;
    sqlite3* pBackupDb;
    sqlite3_backup* pBackup;

    std::string mSourceDatabaseFilePath;
    std::string mDestinationDatabaseFilePath;

    static int BackupPageSize;
};
} // namespace tks::Core
