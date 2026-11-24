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

#include <filesystem>
#include <string>

#include <toml.hpp>

#include "../common/configuration_constants.h"

namespace tks::Core
{
struct MigrationResult {
    bool Success{ false };
    bool Migrated{ false };
    int PreviousVersion{ 0 };
    int CurrentVersion{ 0 };
    std::filesystem::path BackupPath;
    std::string ErrorMessage;
};

class ConfigurationMigration final
{
public:
    explicit ConfigurationMigration(std::filesystem::path configPath);

    bool IsMigrationRequired() const;

    MigrationResult Migrate();

private:
    bool CreateConfigBackup(std::filesystem::path& outBackupPath) const;
    void ApplyMigrations(toml::value& root, int fromVersion) const;
    bool WriteAtomic(const toml::value& root) const;

    std::filesystem::path mConfigPath;
};
} // namespace tks::Core
