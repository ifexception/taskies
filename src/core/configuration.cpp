// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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
const std::string Configuration::Sections::TaskSection = "tasks";
const std::string Configuration::Sections::TasksViewSection = "tasksView";

Configuration::Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
{
}

bool Configuration::Load()
{
    auto configPath = pEnv->GetConfigurationPath();
    bool exists = std::filesystem::exists(configPath);

    pLogger->info("Configuration - Probing for configuration file at path {0}", configPath.string());

    if (!exists) {
        pLogger->error("Configuration - Failed to find configuration file at {0}", configPath.string());
        return false;
    }

    pLogger->info("Configuration - Successfully located configuration file at path {0}", configPath.string());

    try {
        auto data = toml::parse(configPath.string());

        GetGeneralConfig(data);
        GetDatabaseConfig(data);
        GetTasksConfig(data);
        GetTasksViewConfig(data);
    } catch (const toml::syntax_error& error) {
        pLogger->error(
            "Configuration - A TOML syntax/parse error occurred when parsing configuration file {0}", error.what());
        return false;
    }

    return true;
}

bool Configuration::Save()
{
    // clang-format off
    const toml::value v(
        toml::table{
            {
                Sections::GeneralSection,
                toml::table {
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
                toml::table {
                    { "databasePath", mSettings.DatabasePath },
                    { "backupDatabase", mSettings.BackupDatabase },
                    { "backupPath", mSettings.BackupPath },
                    { "backupRetentionPeriod", mSettings.BackupRetentionPeriod }
                }
            },
            {
                Sections::TaskSection,
                toml::table {
                    { "minutesIncrement", mSettings.TaskMinutesIncrement },
                    { "showProjectAssociatedCategories", mSettings.ShowProjectAssociatedCategories }
                }
            },
            {
                Sections::TasksViewSection,
                toml::table {
                    { "todayAlwaysExpanded", mSettings.TodayAlwaysExpanded }
                }
            }
        }
    );
    // clang-format on

    const std::string configString = toml::format(v);

    const std::string configFilePath = pEnv->GetConfigurationPath().string();

    pLogger->info("Configuration - Probing for configuration file for writing at path {0}", configFilePath);

    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error("Configuration - Failed to open configuration file at path {0}", configFilePath);
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

    SetMinutesIncrement(15);
    ShowProjectAssociatedCategories(false);

    // clang-format off
    const toml::value v(
        toml::table{
            {
                Sections::GeneralSection,
                toml::table {
                    { "lang", "en-US" },
                    { "startOnBoot", false },
                    { "startPosition", static_cast<int>(WindowState::Normal) },
                    { "showInTray", false },
                    { "minimizeToTray", false },
                    { "closeToTray", mSettings.CloseToTray },
                }
            },
            {
                Sections::DatabaseSection,
                toml::table {
                    { "databasePath", pEnv->ApplicationDatabasePath().string() },
                    { "backupDatabase", false },
                    { "backupPath", "" },
                    { "backupRetentionPeriod", 0 }
                }
            },
            {
                Sections::TaskSection,
                toml::table {
                    { "minutesIncrement", 15 },
                    { "showProjectAssociatedCategories", false }
                }
            },
            {
                Sections::TasksViewSection,
                toml::table {
                    { "todayAlwaysExpanded", false }
                }
            }
        }
    );
    // clang-format on

    const std::string configString = toml::format(v);

    const std::string configFilePath = pEnv->GetConfigurationPath().string();

    pLogger->info("Configuration - Probing for configuration file for writing at path {0}", configFilePath);

    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error("Configuration - Failed to open configuration file at path {0}", configFilePath);
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

int Configuration::GetMinutesIncrement() const
{
    return mSettings.TaskMinutesIncrement;
}

void Configuration::SetMinutesIncrement(const int value)
{
    mSettings.TaskMinutesIncrement = value;
}

bool Configuration::ShowProjectAssociatedCategories() const
{
    return mSettings.ShowProjectAssociatedCategories;
}

void Configuration::ShowProjectAssociatedCategories(const bool value)
{
    mSettings.ShowProjectAssociatedCategories = value;
}

bool Configuration::TodayAlwaysExpanded() const
{
    return mSettings.TodayAlwaysExpanded;
}

void Configuration::TodayAlwaysExpanded(const bool value)
{
    mSettings.TodayAlwaysExpanded = value;
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

void Configuration::GetTasksConfig(const toml::value& config)
{
    const auto& taskSection = toml::find(config, Sections::TaskSection);

    mSettings.TaskMinutesIncrement = toml::find<int>(taskSection, "minutesIncrement");
    mSettings.ShowProjectAssociatedCategories = toml::find<bool>(taskSection, "showProjectAssociatedCategories");
}

void Configuration::GetTasksViewConfig(const toml::value& config)
{
    const auto& tasksViewSection = toml::find(config, Sections::TasksViewSection);

    mSettings.TodayAlwaysExpanded = toml::find<bool>(tasksViewSection, "todayAlwaysExpanded");
}
} // namespace tks::Core
