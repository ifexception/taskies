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

#include "configuration.h"

#include <filesystem>

#include "environment.h"

namespace tks::Core
{
const std::string Configuration::Sections::GeneralSection = "general";
const std::string Configuration::Sections::DatabaseSection = "database";

Configuration::Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
{
}

bool Configuration::Load()
{
    auto configPath = pEnv->GetConfigurationPath();
    bool exists = std::filesystem::exists(configPath);
    if (!exists) {
        pLogger->error("Could not locate configuration file at {0}", configPath.string());
        return false;
    }

    try {
        auto data = toml::parse(configPath.string());

        GetGeneralConfig(data);
        GetDatabaseConfig(data);
    } catch (const toml::syntax_error& error) {
        pLogger->error("Error occured when parsing toml configuration {0}", error.what());
        return false;
    }

    return true;
}

bool Configuration::Save()
{
    // clang-format off
    const toml::value data{
        {
            Sections::GeneralSection,
            {
                { "lang", mSettings.UserInterfaceLanguage },
                { "startOnBoot", mSettings.StartOnBoot },
                { "startPosition", static_cast<int>(mSettings.StartPosition) },
                { "showInTray", mSettings.ShowInTray },
                { "minimizeToTray", mSettings.MinimizeToTray },
                { "closeToTray", mSettings.CloseToTray },
            }
        },
        {
            Sections::DatabaseSection,
            {
                { "databasePath", mSettings.DatabasePath },
                { "backupDatabase", mSettings.BackupDatabase },
                { "backupPath", mSettings.BackupPath },
                { "backupRetentionPeriod", mSettings.BackupRetentionPeriod }
            }
        }
    };
    // clang-format on

    const std::string configString = toml::format(data, 100);

    const std::string configFilePath = pEnv->GetConfigurationPath().string();
    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error("Failed to open config file at specified location {0}", configFilePath);
        return false;
    }

    configFile << configString;

    configFile.close();
    return true;
}

bool Configuration::RestoreDefaults()
{
    SetUserInterfaceLanguage("en-US");
    StartOnBoot(false);
    SetWindowState(WindowState::Normal);
    ShowInTray(false);
    MinimizeToTray(false);
    CloseToTray(false);

    SetDatabasePath(pEnv->ApplicationDatabasePath().string());
    BackupDatabase(false);
    SetBackupPath("");
    SetBackupRetentionPeriod(0);

    // clang-format off
    const toml::value data{
        {
            Sections::GeneralSection,
            {
                { "lang", "en-US" },
                { "startOnBoot", false },
                { "startPosition", static_cast<int>(WindowState::Normal) },
                { "showInTray", false },
                { "minimizeToTray", false },
                { "closeToTray", false },
            }
        },
        {
            Sections::DatabaseSection,
            {
                { "databasePath", pEnv->ApplicationDatabasePath().string() },
                { "backupDatabase", false },
                { "backupPath", "" },
                { "backupRetentionPeriod", 0 }
            }
        }
    };
    // clang-format on

    const std::string configString = toml::format(data, 100);

    const std::string configFilePath = pEnv->GetConfigurationPath().string();
    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error("Failed to open config file at specified location {0}", configFilePath);
        return false;
    }

    configFile << configString;

    configFile.close();
    return true;
}

std::string Configuration::GetUserInterfaceLanguage() const
{
    return mSettings.UserInterfaceLanguage;
}

void Configuration::SetUserInterfaceLanguage(const std::string& value)
{
    mSettings.UserInterfaceLanguage = value;
}

bool Configuration::StartOnBoot() const
{
    return mSettings.StartOnBoot;
}

void Configuration::StartOnBoot(const bool value)
{
    mSettings.StartOnBoot = value;
}

WindowState Configuration::GetWindowState() const
{
    return mSettings.StartPosition;
}

void Configuration::SetWindowState(const WindowState value)
{
    mSettings.StartPosition = value;
}

bool Configuration::ShowInTray() const
{
    return mSettings.ShowInTray;
}

void Configuration::ShowInTray(const bool value)
{
    mSettings.ShowInTray = value;
}

bool Configuration::MinimizeToTray() const
{
    return mSettings.MinimizeToTray;
}

void Configuration::MinimizeToTray(const bool value)
{
    mSettings.MinimizeToTray = value;
}

bool Configuration::CloseToTray() const
{
    return mSettings.CloseToTray;
}

void Configuration::CloseToTray(const bool value)
{
    mSettings.CloseToTray = value;
}

std::string Configuration::GetDatabasePath() const
{
    return mSettings.DatabasePath;
}

std::string Configuration::GetFullDatabasePath() const
{
    std::filesystem::path fullPath = std::filesystem::path(mSettings.DatabasePath) / pEnv->GetDatabaseName();
    return fullPath.string();
}

void Configuration::SetDatabasePath(const std::string& value)
{
    mSettings.DatabasePath = value;
}

bool Configuration::BackupDatabase() const
{
    return mSettings.BackupDatabase;
}

void Configuration::BackupDatabase(const bool value)
{
    mSettings.BackupDatabase = value;
}

std::string Configuration::GetBackupPath() const
{
    return mSettings.BackupPath;
}

void Configuration::SetBackupPath(const std::string& value)
{
    mSettings.BackupPath = value;
}

int Configuration::GetBackupRetentionPeriod() const
{
    return mSettings.BackupRetentionPeriod;
}

void Configuration::SetBackupRetentionPeriod(const int value)
{
    mSettings.BackupRetentionPeriod = value;
}

void Configuration::GetGeneralConfig(const toml::value& config)
{
    const auto& generalSection = toml::find(config, Sections::GeneralSection);

    mSettings.UserInterfaceLanguage = toml::find<std::string>(generalSection, "lang");
    mSettings.StartOnBoot = toml::find<bool>(generalSection, "startOnBoot");
    auto tomlStartPosition = toml::find<int>(generalSection, "startPosition");
    mSettings.StartPosition = static_cast<WindowState>(tomlStartPosition);
    mSettings.ShowInTray = toml::find<bool>(generalSection, "showInTray");
    mSettings.MinimizeToTray = toml::find<bool>(generalSection, "minimizeToTray");
    mSettings.CloseToTray = toml::find<bool>(generalSection, "closeToTray");
}

void Configuration::GetDatabaseConfig(const toml::value& config)
{
    const auto& databaseSection = toml::find(config, Sections::DatabaseSection);

    mSettings.DatabasePath = toml::find<std::string>(databaseSection, "databasePath");
    mSettings.BackupDatabase = toml::find<bool>(databaseSection, "backupDatabase");
    mSettings.BackupPath = toml::find<std::string>(databaseSection, "backupPath");
    mSettings.BackupRetentionPeriod = toml::find<int>(databaseSection, "backupRetentionPeriod");
}

} // namespace tks::Core
