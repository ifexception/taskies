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

#include "database_optimizer.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../common/messages/sqlitemessages.h"

namespace tks::Core
{
DatabaseOptimizer::DatabaseOptimizer(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , pDb(nullptr)
{
}

DatabaseOptimizer::~DatabaseOptimizer()
{
    SPDLOG_LOGGER_TRACE(pLogger, "Clean up sqlite3 resources");

    if (pDb != nullptr) {
        sqlite3_close(pDb);
        pDb = nullptr;
    }
}

SqliteResult DatabaseOptimizer::Optimize()
{
    auto result = Initialize();
    if (!result.Success) {
        return result;
    }

    int rc = sqlite3_exec(pDb, QueryHelper::Optimize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Optimize, rc, error);

        return SqliteResult::Fail(rc, std::string(error));
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Successfully optimized database");

    return SqliteResult::OK();
}

SqliteResult DatabaseOptimizer::Initialize()
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, mDatabaseFilePath);

    int rc = sqlite3_open_v2(mDatabaseFilePath.c_str(), &pDb, SQLITE_OPEN_READWRITE, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, mDatabaseFilePath, rc, error);

        sqlite3_close(pDb);

        return SqliteResult::FailDetailed(Messages::OpenDatabaseMessage, rc, std::string(error));
    }

    return SqliteResult::OK();
}
} // namespace tks::Core
