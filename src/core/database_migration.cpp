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

#include "database_migration.h"

#include "../common/constants.h"

#include "../utils/utils.h"

#include "environment.h"
#include "configuration.h"

namespace
{
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static BOOL CALLBACK EnumMigrations(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
    std::vector<tks::Core::Migration>* migrations = reinterpret_cast<std::vector<tks::Core::Migration>*>(lParam);

    HRSRC rc = FindResource(hModule, lpszName, lpszType);
    DWORD size = SizeofResource(hModule, rc);
    HGLOBAL data = LoadResource(hModule, rc);
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
const std::string DatabaseMigration::BeginTransactionQuery = "BEGIN TRANSACTION";
const std::string DatabaseMigration::CommitTransactionQuery = "COMMIT";
const std::string DatabaseMigration::CreateMigrationHistoryQuery = "CREATE TABLE IF NOT EXISTS migration_history("
                                                                   "id INTEGER PRIMARY KEY NOT NULL,"
                                                                   "name TEXT NOT NULL"
                                                                   ");";
const std::string DatabaseMigration::SelectMigrationExistsQuery =
    "SELECT COUNT(*) FROM migration_history WHERE name = ?";
const std::string DatabaseMigration::InsertMigrationHistoryQuery = "INSERT INTO migration_history (name) VALUES (?);";

DatabaseMigration::DatabaseMigration(
    std::shared_ptr<spdlog::logger> logger, const std::string& databaseFilePath)
    : pLogger(logger)
    , pDb(nullptr)
{
    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "DatabaseMigration",
            databaseFilePath,
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "DatabaseMigration", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "DatabaseMigration", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "DatabaseMigration", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "DatabaseMigration", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "DatabaseMigration", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

DatabaseMigration::~DatabaseMigration()
{
    sqlite3_close(pDb);
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

    int rc = sqlite3_exec(pDb, BeginTransactionQuery.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "DatabaseMigration", BeginTransactionQuery.c_str(), rc, err);
        return false;
    }

    for (const auto& migration : migrations) {
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
                    LogMessage::PrepareStatementTemplate, "DatabaseMigration", migration.name.c_str(), rc, err);
                sqlite3_finalize(migrationStmt);
                return false;
            }

            if (migrationStmt == nullptr) {
                break;
            }

            mrc = sqlite3_step(migrationStmt);
            if (mrc != SQLITE_OK && mrc != SQLITE_DONE) {
                const char* err = sqlite3_errmsg(pDb);
                pLogger->error(LogMessage::ExecStepTemplate, "DatabaseMigration", migration.name.c_str(), rc, err);
                sqlite3_finalize(migrationStmt);
                return false;
            }

            sqlite3_finalize(migrationStmt);
        } while (sql && sql[0] != '\0');

        sqlite3_stmt* migrationHistoryStmt = nullptr;

        int mhrc = sqlite3_prepare_v2(pDb, InsertMigrationHistoryQuery.c_str(), -1, &migrationHistoryStmt, nullptr);
        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::PrepareStatementTemplate,
                "DatabaseMigration",
                InsertMigrationHistoryQuery.c_str(),
                rc,
                err);
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_bind_text(
            migrationHistoryStmt, 1, migration.name.c_str(), static_cast<int>(migration.name.size()), SQLITE_TRANSIENT);
        if (mhrc != SQLITE_OK) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::BindParameterTemplate, "DatabaseMigration", migration.name.c_str(), 1, rc, err);
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_step(migrationHistoryStmt);
        if (mhrc != SQLITE_DONE) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->error(LogMessage::ExecStepTemplate, "DatabaseMigration", migration.name.c_str(), rc, err);
            sqlite3_finalize(migrationHistoryStmt);
            return false;
        }

        mhrc = sqlite3_step(migrationHistoryStmt);
        if (mhrc != SQLITE_DONE) {
            const char* err = sqlite3_errmsg(pDb);
            pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "DatabaseMigration", rc, err);
        }

        sqlite3_finalize(migrationHistoryStmt);
    }

    rc = sqlite3_exec(pDb, CommitTransactionQuery.c_str(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "DatabaseMigration", CommitTransactionQuery.c_str(), rc, err);
        return false;
    }

    return true;
}

void DatabaseMigration::CreateMigrationHistoryTable()
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, CreateMigrationHistoryQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::PrepareStatementTemplate, "DatabaseMigration", CreateMigrationHistoryQuery.c_str(), rc, err);
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "DatabaseMigration", CreateMigrationHistoryQuery.c_str(), rc, err);
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
}

bool DatabaseMigration::MigrationExists(const std::string& name)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, SelectMigrationExistsQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::PrepareStatementTemplate, "DatabaseMigration", SelectMigrationExistsQuery.c_str(), rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), static_cast<int>(name.size()), SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "DatabaseMigration", name.c_str(), 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "DatabaseMigration", SelectMigrationExistsQuery.c_str(), rc, err);
        sqlite3_finalize(stmt);
        return false;
    }
    int count = sqlite3_column_int(stmt, 0);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "DatabaseMigration", rc, err);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}
} // namespace tks::Core
