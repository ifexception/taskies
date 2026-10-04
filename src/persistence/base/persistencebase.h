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

#include <string>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include <sqlite3.h>

namespace tks::Persistence
{
struct PersistenceResult {
    bool Success;
    int ReturnCode;
    std::string Error;

    PersistenceResult();
    PersistenceResult(int returnCode, const std::string& error);
    ~PersistenceResult() = default;
};

struct SqliteDbDeleterFn {
    void operator()(sqlite3* db) const
    {
        if (db) {
            sqlite3_close(db);
        }
    }
};

struct PersistenceBase {
    PersistenceBase(std::shared_ptr<spdlog::logger> logger, std::string databaseFilePath);
    virtual ~PersistenceBase();

    std::shared_ptr<spdlog::logger> pLogger;
    std::unique_ptr<sqlite3, SqliteDbDeleterFn> pDb;
    PersistenceResult result;
};
} // namespace tks::Persistence
