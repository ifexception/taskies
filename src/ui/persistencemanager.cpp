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
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to open database {0}", std::string(err));
    }
}

PersistenceManager::~PersistenceManager()
{
    sqlite3_close(pDb);
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, bool* value)
{
    sqlite3_stmt* stmt = nullptr;

    bool ret = ReadValue(who, name, &stmt);

    if (ret) {
        *value = (sqlite3_column_int(stmt, 0) > 0);
    }
    sqlite3_finalize(stmt);
    return ret;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, int* value)
{
    sqlite3_stmt* stmt = nullptr;

    bool ret = ReadValue(who, name, &stmt);

    if (ret) {
        *value = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return ret;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, long* value)
{
    sqlite3_stmt* stmt = nullptr;

    bool ret = ReadValue(who, name, &stmt);

    if (ret) {
        *value = static_cast<long>(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return ret;
}

bool PersistenceManager::RestoreValue(const wxPersistentObject& who, const wxString& name, wxString* value)
{
    sqlite3_stmt* stmt = nullptr;

    bool ret = ReadValue(who, name, &stmt);

    if (ret) {
        const unsigned char* res = sqlite3_column_text(stmt, 0);
        *value = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, 0));
    }
    sqlite3_finalize(stmt);
    return ret;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, bool value)
{
    SaveValue(GetKey(who, name), std::to_string(value));
    return true;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, int value)
{
    SaveValue(GetKey(who, name), std::to_string(value));
    return true;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, long value)
{
    SaveValue(GetKey(who, name), std::to_string(value));
    return true;
}

bool PersistenceManager::SaveValue(const wxPersistentObject& who, const wxString& name, wxString value)
{
    SaveValue(GetKey(who, name), value.ToStdString());
    return true;
}

bool PersistenceManager::ReadValue(const wxPersistentObject& who, const wxString& name, sqlite3_stmt** stmt)
{
    int rc = sqlite3_prepare_v2(pDb, PersistenceSelectQuery.c_str(), -1, *&stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        return false;
    }

    auto key = GetKey(who, name).ToStdString();

    rc = sqlite3_bind_text(*stmt, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0} - {1}", key, std::string(err));
        return false;
    }

    rc = sqlite3_step(*stmt);
    if (rc != SQLITE_ROW) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when executing statement {0}", std::string(err));
        return false;
    }

    return true;
}

wxString PersistenceManager::GetKey(const wxPersistentObject& who, const wxString& name)
{
    return who.GetKind() << wxCONFIG_PATH_SEPARATOR << who.GetName() << wxCONFIG_PATH_SEPARATOR << name;
}

void PersistenceManager::SaveValue(const wxString& key, const std::string& value)
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, PersistenceInsertQuery.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_bind_text(
        stmt, 1, key.ToStdString().c_str(), static_cast<int>(key.ToStdString().size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to bind parameter {0} - {1}", key.ToStdString(), std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_bind_text(
        stmt, 2, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Failed to prepare statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error("Error when executing statement {0}", std::string(err));
        sqlite3_finalize(stmt);
        return;
    }
}
} // namespace tks::UI
