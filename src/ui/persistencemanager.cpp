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

#include "persistencemanager.h"

#include "../core/environment.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::UI
{
std::string PersistenceManager::PersistenceSelectQuery =
    "SELECT value FROM persistent_objects WHERE key = ?;";
std::string PersistenceManager::PersistenceInsertQuery =
    "INSERT OR REPLACE INTO persistent_objects(key, value) VALUES(?, ?);";

PersistenceManager::PersistenceManager(std::shared_ptr<spdlog::logger> logger,
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

PersistenceManager::~PersistenceManager()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who,
    const wxString& name,
    bool* value)
{
    sqlite3_stmt* stmt = nullptr;

    auto key = GetKey(who, name).ToStdString();

    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);

        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, key, 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    } else if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    *value = (sqlite3_column_int(stmt, 0) > 0);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who,
    const wxString& name,
    int* value)
{
    sqlite3_stmt* stmt = nullptr;

    auto key = GetKey(who, name).ToStdString();

    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, key, 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    } else if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    *value = sqlite3_column_int(stmt, 0);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who,
    const wxString& name,
    long* value)
{
    sqlite3_stmt* stmt = nullptr;

    auto key = GetKey(who, name).ToStdString();

    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, key, 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    } else if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    *value = static_cast<long>(sqlite3_column_int(stmt, 0));

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who,
    const wxString& name,
    wxString* value)
{
    sqlite3_stmt* stmt = nullptr;

    auto key = GetKey(who, name).ToStdString();

    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, key, 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    } else if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, PersistenceSelectQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    const unsigned char* res = sqlite3_column_text(stmt, 0);
    *value = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, 0));

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessages::ExecQueryDidNotReturnOneResultTemplate, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, bool value)
{
    auto key = GetKey(who, name).ToStdString();

    bool ret = SaveValue(key, std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, int value)
{
    auto key = GetKey(who, name).ToStdString();

    bool ret = SaveValue(key, std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, long value)
{
    auto key = GetKey(who, name).ToStdString();

    bool ret = SaveValue(key, std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who,
    const wxString& name,
    wxString value)
{
    auto key = GetKey(who, name).ToStdString();

    bool ret = SaveValue(key, value.ToStdString());
    return ret;
}

wxString PersistenceManager::GetKey(const wxPersistentObject& who, const wxString& name)
{
    return who.GetKind() << wxCONFIG_PATH_SEPARATOR << who.GetName() << wxCONFIG_PATH_SEPARATOR
                         << name;
}

bool PersistenceManager::SaveValue(const std::string& key, const std::string& value)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, PersistenceInsertQuery.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, PersistenceInsertQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, key, 1, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc =
        sqlite3_bind_text(stmt, 2, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);

    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, value, 2, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, PersistenceInsertQuery, rc, err);

        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
} // namespace tks::UI
