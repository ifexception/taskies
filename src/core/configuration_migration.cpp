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

#include "configuration_migration.h"

#include <fmt/format.h>

namespace tks::Core
{
ConfigurationMigration::ConfigurationMigration(std::filesystem::path configPath)
    : mConfigPath(configPath)
{
}

bool ConfigurationMigration::IsMigrationRequired() const
{
    std::error_code ec;
    if (!std::filesystem::exists(mConfigPath, ec) || ec) {
        return false; // Config file does not exist so nothing to migrate
    }

    try {
        const toml::value root = toml::parse(mConfigPath);
        const int fileVersion = toml::find_or<int>(root, "schema_version", 0);
        return fileVersion < CURRENT_SCHEMA_VERSION;
    } catch (const std::exception&) {
        // Corrupted or unreadable TOML; let Migrate() report the detailed error
        return true;
    }
}

MigrationResult ConfigurationMigration::Migrate()
{
    MigrationResult result;
    result.CurrentVersion = CURRENT_SCHEMA_VERSION;

    std::error_code ec;
    if (!std::filesystem::exists(mConfigPath, ec) || ec) {
        result.ErrorMessage =
            fmt::format("Configuration file does not exist: \"{0}\"", mConfigPath.string());
        return result;
    }

    toml::value root;
    try {
        root = toml::parse(mConfigPath);
    } catch (const std::exception& ex) {
        result.ErrorMessage = fmt::format("Failed to parse TOML configuration: \"{0}\"", ex.what());
        return result;
    }

    const int fileVersion = toml::find_or<int>(root, "schema_version", 0);
    result.PreviousVersion = fileVersion;

    if (fileVersion >= CURRENT_SCHEMA_VERSION) {
        result.Success = true;
        result.Migrated = false;
        return result;
    }

    std::filesystem::path backupPath;
    if (!CreateConfigBackup(backupPath)) {
        result.ErrorMessage = "Failed to create configuration backup before migration";
        return result;
    }
    result.BackupPath = backupPath;

    try {
        ApplyMigrations(root, fileVersion);
        root["schema_version"] = CURRENT_SCHEMA_VERSION;
    } catch (const std::exception& ex) {
        result.ErrorMessage = fmt::format("Migration step failed with error \"{0}\"", ex.what());
        return result;
    }

    if (!WriteAtomic(root)) {
        result.ErrorMessage = "Failed to atomically write migrated configuration to disk.";
        return result;
    }

    result.Success = true;
    result.Migrated = true;

    return result;
}

bool ConfigurationMigration::CreateConfigBackup(std::filesystem::path& outBackupPath) const
{
    std::filesystem::path backupPath = mConfigPath;
    backupPath += ".bak";

    std::error_code ec;
    std::filesystem::copy_file(
        mConfigPath, backupPath, std::filesystem::copy_options::overwrite_existing, ec);

    if (ec) {
        return false;
    }

    outBackupPath = std::move(backupPath);
    return true;
}

void ConfigurationMigration::ApplyMigrations(toml::value& root, int fromVersion) const
{
    if (fromVersion < 1) {
        if (!root.contains("schema_versions")) {
            root["schema_versions"] = fromVersion;
        }

        if (!root.contains("setup") || !root.at("setup").is_table()) {
            root["setup"] = toml::table{};
        }

        auto& setup = root["setup"];
        if (!setup.contains("is_setup")) {
            setup["is_setup"] = false;
        }
    }
}

bool ConfigurationMigration::WriteAtomic(const toml::value& root) const
{
    std::filesystem::path tempPath = mConfigPath;
    tempPath += ".tmp";

    {
        std::ofstream ofs(tempPath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) {
            return false;
        }

        ofs << toml::format(root);
        ofs.flush();

        if (!ofs.good()) {
            ofs.close();
            std::error_code ec;
            std::filesystem::remove(tempPath, ec);
            return false;
        }
    }

    // Atomically replace the target file with the temp file
    std::error_code ec;
    std::filesystem::rename(tempPath, mConfigPath, ec);
    if (ec) {
        // Clean up orphan .tmp file if rename fails
        std::error_code removeEc;
        std::filesystem::remove(tempPath, removeEc);
        return false;
    }

    return true;
}
} // namespace tks::Core
