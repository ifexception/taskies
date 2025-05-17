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

#include "database_migration.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

#include "environment.h"
#include "configuration.h"

namespace
{
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static BOOL CALLBACK EnumMigrations(HMODULE hModule,
    LPCTSTR lpszType,
    LPTSTR lpszName,
    LONG_PTR lParam)
{
    std::vector<tks::Core::Migration>* migrations =
        reinterpret_cast<std::vector<tks::Core::Migration>*>(lParam);

    HRSRC rc = FindResource(hModule, lpszName, lpszType);
    if (rc == nullptr) {
        return FALSE;
    }

    DWORD size = SizeofResource(hModule, rc);
    HGLOBAL data = LoadResource(hModule, rc);
    if (data == nullptr) {
        return false;
    }

    const char* buffer = reinterpret_cast<const char*>(LockResource(data));

    tks::Core::Migration m;
    m.name = tks::Utils::ToStdString(lpszName);
    m.sql = std::string(buffer, size);

    migrations->push_back(m);

    return TRUE;
}
} // namespace

namespace tks::Core
{
std::string DatabaseMigration::BeginTransactionQuery = "BEGIN TRANSACTION";

std::string DatabaseMigration::CommitTransactionQuery = "COMMIT";

std::string DatabaseMigration::CreateMigrationHistoryQuery =
    "CREATE TABLE IF NOT EXISTS migration_history("
    "id INTEGER PRIMARY KEY NOT NULL,"
    "name TEXT NOT NULL"
    ");";

std::string DatabaseMigration::SelectMigrationExistsQuery =
    "SELECT COUNT(*) FROM migration_history WHERE name = ?";

std::string DatabaseMigration::InsertMigrationHistoryQuery =
    "INSERT INTO migration_history (name) VALUES (?)";

DatabaseMigration::DatabaseMigration(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, databaseFilePath, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Synchronous, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::TempStore, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::MmapSize, rc, error);

        return;
    }
}

DatabaseMigration::~DatabaseMigration()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

bool DatabaseMigration::Migrate()
{
    CreateMigrationHistoryTable();

    std::vector<Migration> migrations;

    // clang-format off
    EnumResourceNames(
        nullptr,
        TEXT("MIGRATION"),
        &EnumMigrations,
        reinterpret_cast<LONG_PTR>(&migrations)
    );
    // clang-format on

    SPDLOG_LOGGER_TRACE(pLogger, "Count of migrations to run: {0}", migrations.size());

    int rc = sqlite3_exec(pDb, BeginTransactionQuery.c_str(), nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, BeginTransactionQuery.c_str(), rc, err);

        return false;
    }

    for (const auto& migration : migrations) {
        SPDLOG_LOGGER_TRACE(pLogger, "Begin to run migration \"{0}\"", migration.name);

        if (MigrationExists(migration.name)) {
            continue;
        }

        sqlite3_stmt* migrationStmt = nullptr;
        const char* sql = migration.sql.c_str();

        do {
            int mrc = sqlite3_prepare_v2(pDb, sql, -1, &migrationStmt, &sql);

            if (mrc != SQLITE_OK) {
                const char* err = sqlite3_errmsg(pDb);
                pLogger->error(
                    LogMessages::PrepareStatementTemplate, migration.name.c_str(), rc, err);

                sqlite3_finalize(migrationStmt);
                return false;
            }

            if (migrationStmt == nullptr) {
                break;
            }

            mrc = sqlite3_step(migrationStmt);

            if (mrc != SQLITE_OK && mrc != SQLITE_DONE) {
                const char* err = sqlite3_errmsg(pDb);
                pLogger->error(LogMessages::ExecStepTemplate, migration.name.c_str(), rc, err);

                sqlite3_finalize(migrationStmt);
                return false;
            }

            sqlite3_finalize(migrationStmt);

        } while (sql && sql[0] != '\0');

        SPDLOG_LOGGER_TRACE(pLogger, "Completed migration \"{0}\"", migration.name);

        sqlite3_stmt* migrationHistoryStmt = nullptr;

        int mhrc = sqlite3_prepare_v2(
            pDb, InsertMigrationHistoryQuery.c_str(), -1, &migrationHistoryStmt, nullptr);

        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::PrepareStatementTemplate,
                InsertMigrationHistoryQuery.c_str(),
                rc,
                err);

            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_bind_text(migrationHistoryStmt,
            1,
            migration.name.c_str(),
            static_cast<int>(migration.name.size()),
            SQLITE_TRANSIENT);

        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::BindParameterTemplate, migration.name.c_str(), 1, rc, err);

            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_step(migrationHistoryStmt);

        if (mhrc != SQLITE_DONE) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::ExecStepTemplate, migration.name.c_str(), rc, err);

            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        sqlite3_finalize(migrationHistoryStmt);

        SPDLOG_LOGGER_TRACE(pLogger,
            "Completed insert of migration \"{0}\" into MigrationHistory table",
            migration.name);
    }

    rc = sqlite3_exec(pDb, CommitTransactionQuery.c_str(), nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, CommitTransactionQuery.c_str(), rc, err);

        return false;
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Commit migration transaction");

    return true;
}

void DatabaseMigration::CreateMigrationHistoryTable() const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, CreateMigrationHistoryQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, CreateMigrationHistoryQuery.c_str(), rc, err);

        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, CreateMigrationHistoryQuery.c_str(), rc, err);

        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

bool DatabaseMigration::MigrationExists(const std::string& name) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, SelectMigrationExistsQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, SelectMigrationExistsQuery.c_str(), rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), static_cast<int>(name.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, name.c_str(), 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, SelectMigrationExistsQuery.c_str(), rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    int count = sqlite3_column_int(stmt, 0);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);
    }

    SPDLOG_LOGGER_TRACE(pLogger, "Migration \"{0}\" status: {1}", name, count);

    sqlite3_finalize(stmt);
    return count > 0;
}
} // namespace tks::Core
