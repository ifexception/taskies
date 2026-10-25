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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include "environment.h"

#ifdef _WIN32
namespace
{
// RAII guard to manage lifetime of HKEY closing when exiting scopes
struct RegKeyGuard {
    HKEY key = nullptr;
    ~RegKeyGuard()
    {
        if (key) {
            ::RegCloseKey(key);
        }
    }
};
} // namespace
#endif // _WIN32

namespace tks::Core
{
class SetupManager final
{
public:
    explicit SetupManager(std::shared_ptr<Environment> env) noexcept;

    SetupManager(const SetupManager&) = delete;
    SetupManager& operator=(const SetupManager&) = delete;
    SetupManager(SetupManager&&) noexcept = default;
    SetupManager& operator=(SetupManager&&) noexcept = default;
    ~SetupManager() = default;

    bool IsSetup() const;
    bool SetIsSetup();

    bool ConfigurationFileMigrationRequired() const;

private:
    std::string GetRegistryKey() const;

    std::shared_ptr<Environment> pEnv;
};
} // namespace tks::Core
