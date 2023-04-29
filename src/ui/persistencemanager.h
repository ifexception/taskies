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

#pragma once

#include <memory>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/persist.h>
#include <sqlite3.h>
#include <spdlog/spdlog.h>

namespace tks
{
namespace Core
{
class Environment;
} // namespace Core
namespace UI
{
class PersistenceManager : public wxPersistenceManager
{
public:
    PersistenceManager(std::shared_ptr<Core::Environment> env, std::shared_ptr<spdlog::logger> logger);
    virtual ~PersistenceManager();

    bool RestoreValue(const wxPersistentObject& who, const wxString& name, bool* value) override;
    bool RestoreValue(const wxPersistentObject& who, const wxString& name, int* value) override;
    bool RestoreValue(const wxPersistentObject& who, const wxString& name, long* value) override;
    bool RestoreValue(const wxPersistentObject& who, const wxString& name, wxString* value) override;

    bool SaveValue(const wxPersistentObject& who, const wxString& name, bool value) override;
    bool SaveValue(const wxPersistentObject& who, const wxString& name, int value) override;
    bool SaveValue(const wxPersistentObject& who, const wxString& name, long value) override;
    bool SaveValue(const wxPersistentObject& who, const wxString& name, wxString value) override;

private:
    wxString GetKey(const wxPersistentObject& who, const wxString& name);
    bool SaveValue(const std::string& key, const std::string& value);

    sqlite3* pDb;
    std::shared_ptr<Core::Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;

    static const std::string PersistenceSelectQuery;
    static const std::string PersistenceInsertQuery;
};
} // namespace UI
} // namespace tks
