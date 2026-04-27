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

#include "database_backup.h"

#include <fmt/format.h>

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../common/messages/sqlitemessages.h"

#include "configuration.h"
#include "environment.h"

namespace tks::Core
{
int DatabaseBackup::BackupPageSize = 64;

DatabaseBackup::DatabaseBackup(std::shared_ptr<spdlog::logger> logger,
    std::shared_ptr<Configuration> cfg,
    std::shared_ptr<Environment> env)
    : pLogger(logger)
    , pCfg(cfg)
    , pEnv(env)
    , pDb(nullptr)
    , pBackupDb(nullptr)
    , pBackup(nullptr)
{
}

DatabaseBackup::~DatabaseBackup()
{
    CleanUp();
}

SqliteResult DatabaseBackup::Backup()
{
    auto result = Initialize();
    if (!result.Success) {
        return result;
    }

    pBackup = sqlite3_backup_init(/*destination*/ pBackupDb, "main", /*source*/ pDb, "main");
    if (pBackup == nullptr) {
        const char* error = sqlite3_errmsg(pBackupDb);
        int rc = SQLITE_ERROR;
        pLogger->error(
            "Failed to initialize database backup operation. Error {0}: \"{1}\"", rc, error);

        // to refactor, should _not_ need both Cfg and Env to get backup path
        auto backupFilePath =
            fmt::format("{0}/{1}", pCfg->GetBackupPath(), pEnv->GetDatabaseFileName());

        return SqliteResult::FailDetailed(
            fmt::format(Messages::BackupMessage, backupFilePath), rc, std::string(error));
    }

    auto pageCount = sqlite3_backup_pagecount(pBackup);
    SPDLOG_LOGGER_TRACE(pLogger, "Count of pages to backup: \"{0}\"", pageCount);

    int rc = -1;

    while (1) {
        SPDLOG_LOGGER_TRACE(
            pLogger, "Perform backup step with page size of \"{0}\"", BackupPageSize);
        rc = sqlite3_backup_step(pBackup, BackupPageSize);

        /* Get progress information */
        int remaining = sqlite3_backup_remaining(pBackup);
        int total = sqlite3_backup_pagecount(pBackup);
        SPDLOG_LOGGER_TRACE(pLogger, "Total page size \"{0}\"", total);

        if (total > 0) {
            int percent = ((total - remaining) * 100) / total;
            SPDLOG_LOGGER_TRACE(pLogger,
                "Backup progress: {0}% ({1}/{2} pages)",
                percent,
                total - remaining,
                total);
        }

        if (rc == SQLITE_OK) {
            /* Backup is still ongoing, continue stepping */
            continue;
        } else if (rc == SQLITE_DONE) {
            /* Backup completed successfully */
            result.Success = true;
            break;
        } else if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
            /* Database is busy/locked - retry is recommended */
            SPDLOG_LOGGER_TRACE(pLogger, "Database busy/locked, retrying...");
            sqlite3_sleep(128); /* Wait 128ms before retrying */
            continue;
        } else {
            // to refactor, should _not_ need both Cfg and Env to get backup path
            auto backupFilePath =
                fmt::format("{0}/{1}", pCfg->GetBackupPath(), pEnv->GetDatabaseFileName());

            /* Fatal error occurred */
            const char* error = sqlite3_errmsg(pBackupDb);

            result.Success = false;
            result.FriendlyErrorMessage = fmt::format(Messages::BackupMessage, backupFilePath);
            result.ReturnCode = rc;
            result.ErrorMessage = std::string(error);

            break;
        }
    }

    rc = sqlite3_backup_finish(pBackup);
    pBackup = nullptr;

    if (rc != SQLITE_OK && !result.Success) {
        const char* error = sqlite3_errmsg(pBackupDb);
        pLogger->error(
            "Backup failed to finish with error code \"{0}\" and message \"{1}\"", rc, error);

        return result;
    }

    return result;
}

SqliteResult DatabaseBackup::Initialize()
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, pCfg->GetDatabasePath());

    int rc = sqlite3_open_v2(pCfg->GetDatabasePath().c_str(),
        &pDb,
        SQLITE_OPEN_READONLY, /* Read-only to avoid locking the source */
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, pCfg->GetDatabasePath(), rc, error);

        sqlite3_close(pDb);

        return SqliteResult::FailDetailed(Messages::OpenDatabaseMessage, rc, std::string(error));
    }

    rc = sqlite3_exec(pDb, QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        sqlite3_close(pDb);

        return SqliteResult::FailDetailed(Messages::ExecMessage, rc, std::string(error));
    }

    // to refactor, should _not_ need both Cfg and Env to get backup path
    auto backupFilePath = fmt::format("{0}/{1}", pCfg->GetBackupPath(), pEnv->GetDatabaseFileName());

    // open or create the destination database
    rc = sqlite3_open_v2(
        backupFilePath.c_str(), &pBackupDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pBackupDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, backupFilePath, rc, error);

        sqlite3_close(pDb);
        sqlite3_close(pBackupDb);

        return SqliteResult::FailDetailed(Messages::OpenDatabaseMessage, rc, std::string(error));
    }

    return SqliteResult::OK();
}

void DatabaseBackup::CleanUp()
{
    SPDLOG_LOGGER_TRACE(pLogger, "Clean up sqlite3 backup resources");

    if (pDb != nullptr) {
        sqlite3_close(pDb);
        pDb = nullptr;
    }
    if (pBackupDb != nullptr) {
        sqlite3_close(pBackupDb);
        pBackupDb = nullptr;
    }
}
} // namespace tks::Core
