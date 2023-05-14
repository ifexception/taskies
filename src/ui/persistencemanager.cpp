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

#include "persistencemanager.h"

#include "../core/environment.h"
#include "../common/constants.h"
#include "../utils/utils.h"

namespace tks::UI
{
const std::string PersistenceManager::PersistenceSelectQuery = "SELECT value FROM persistent_objects WHERE key = ?;";
const std::string PersistenceManager::PersistenceInsertQuery =
    "INSERT OR REPLACE INTO persistent_objects(key, value) VALUES(?, ?);";

PersistenceManager::PersistenceManager(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
    , pDb(nullptr)
{
    auto databaseFile = pEnv->GetDatabasePath().string();
    int rc = sqlite3_open(databaseFile.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate,
            "PersistenceManager",
            pEnv->GetDatabaseName(),
            pEnv->GetDatabasePath().string(),
            rc,
            std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "PersistenceManager", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "PersistenceManager", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessage::ExecQueryTemplate, "PersistenceManager", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "PersistenceManager", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "PersistenceManager", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

PersistenceManager::~PersistenceManager()
{
    sqlite3_close(pDb);
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, bool* value)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    auto key = GetKey(who, name).ToStdString();
    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", key, 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    *value = (sqlite3_column_int(stmt, 0) > 0);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "PersistenceManager", rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, int* value)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    auto key = GetKey(who, name).ToStdString();
    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", key, 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    *value = sqlite3_column_int(stmt, 0);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "PersistenceManager", rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, long* value)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    auto key = GetKey(who, name).ToStdString();
    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", key, 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    *value = static_cast<long>(sqlite3_column_int(stmt, 0));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "PersistenceManager", rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, wxString* value)
{
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    auto key = GetKey(who, name).ToStdString();
    rc = sqlite3_bind_text(stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", key, 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "PersistenceManager", PersistenceSelectQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    const unsigned char* res = sqlite3_column_text(stmt, 0);
    *value = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, 0));

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->warn(LogMessage::ExecStepMoreResultsThanExpectedTemplate, "PersistenceManager", rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, bool value)
{
    bool ret = SaveValue(GetKey(who, name).ToStdString(), std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, int value)
{
    bool ret = SaveValue(GetKey(who, name).ToStdString(), std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, long value)
{
    bool ret = SaveValue(GetKey(who, name).ToStdString(), std::to_string(value));
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, wxString value)
{
    bool ret = SaveValue(GetKey(who, name).ToStdString(), value.ToStdString());
    return ret;
}

wxString PersistenceManager::GetKey(const wxPersistentObject& who, const wxString& name)
{
    return who.GetKind() << wxCONFIG_PATH_SEPARATOR << who.GetName() << wxCONFIG_PATH_SEPARATOR << name;
}

bool PersistenceManager::SaveValue(const std::string& key, const std::string& value)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, PersistenceInsertQuery.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "PersistenceManager", PersistenceInsertQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(
        stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", key, 1, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_bind_text(stmt, 2, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::BindParameterTemplate, "PersistenceManager", value, 2, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "PersistenceManager", PersistenceInsertQuery, rc, err);
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
} // namespace tks::UI
