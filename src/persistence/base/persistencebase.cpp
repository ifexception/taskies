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

#include "persistencebase.h"

#include "../../common/logmessages.h"
#include "../../common/queryhelper.h"

namespace tks::Persistence
{
PersistenceResult::PersistenceResult()
    : Success(true)
    , ReturnCode(0)
    , Error("")
{
}

PersistenceResult::PersistenceResult(int returnCode, const std::string& error)
    : Success(false)
    , ReturnCode(returnCode)
    , Error(error)
{
}

PersistenceBase::PersistenceBase(std::shared_ptr<spdlog::logger> logger,
    std::string databaseFilePath)
    : pLogger(std::move(logger))
    , pDb(nullptr)
    , result()
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, databaseFilePath);

    sqlite3* db = nullptr;
    int rc = sqlite3_open(databaseFilePath.c_str(), &db);

    pDb.reset(db);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::OpenDatabaseTemplate, databaseFilePath, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb.get(), QueryHelper::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb.get(), QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb.get(), QueryHelper::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Synchronous, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb.get(), QueryHelper::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::TempStore, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }

    rc = sqlite3_exec(pDb.get(), QueryHelper::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb.get());
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::MmapSize, rc, error);

        result = PersistenceResult(rc, std::string(error));
        return;
    }
}

PersistenceBase::~PersistenceBase()
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}
} // namespace tks::Persistence
