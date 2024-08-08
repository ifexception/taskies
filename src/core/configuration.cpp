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
const std::string Configuration::Sections::ExportSection = "export";
const std::string Configuration::Sections::PresetsSection = "presets";

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
        auto root = toml::parse(configPath.string());

        GetGeneralConfig(root);
        GetDatabaseConfig(root);
        GetTasksConfig(root);
        GetTasksViewConfig(root);
        GetExportConfig(root);
        GetPresetsConfigEx(root);
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
    toml::value root(
        toml::table {
            {
                Sections::GeneralSection,
                toml::table { },
            },
            {
                Sections::DatabaseSection,
                toml::table { }
            },
            {
                Sections::TaskSection,
                toml::table { }
            },
            {
                Sections::TasksViewSection,
                toml::table { }
            },
            {
                Sections::ExportSection,
                toml::table { }
            },
            {
                Sections::PresetsSection,
                toml::array { }
            },
        }
    );
    // clang-format on

    // General section
    root.at(Sections::GeneralSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::GeneralSection)["lang"] = mSettings.UserInterfaceLanguage;
    root.at(Sections::GeneralSection)["startOnBoot"] = mSettings.StartOnBoot;
    root.at(Sections::GeneralSection)["startPosition"] = static_cast<int>(mSettings.StartPosition);
    root.at(Sections::GeneralSection)["showInTray"] = mSettings.ShowInTray;
    root.at(Sections::GeneralSection)["minimizeToTray"] = mSettings.MinimizeToTray;
    root.at(Sections::GeneralSection)["closeToTray"] = mSettings.CloseToTray;

    // Database section
    root.at(Sections::DatabaseSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::DatabaseSection)["databasePath"] = mSettings.DatabasePath;
    root.at(Sections::DatabaseSection)["backupDatabase"] = mSettings.BackupDatabase;
    root.at(Sections::DatabaseSection)["backupPath"] = mSettings.BackupPath;
    root.at(Sections::DatabaseSection)["backupRetentionPeriod"] = mSettings.BackupRetentionPeriod;

    // Task section
    root.at(Sections::TaskSection).as_table_fmt().fmt = toml::table_format::multiline;

    root.at(Sections::TaskSection)["minutesIncrement"] = mSettings.TaskMinutesIncrement;
    root.at(Sections::TaskSection)["showProjectAssociatedCategories"] = mSettings.ShowProjectAssociatedCategories;

    // Tasks View section
    root.at(Sections::TasksViewSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::TasksViewSection)["todayAlwaysExpanded"] = mSettings.TodayAlwaysExpanded;

    // Export section
    root.at(Sections::ExportSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::ExportSection)["exportPath"] = mSettings.ExportPath;
    root.at(Sections::ExportSection)["presetCount"] = mSettings.PresetCount;

    // Presets section
    auto& presets = root.at(Sections::PresetsSection);
    presets.as_array_fmt().fmt = toml::array_format::array_of_tables;

    if (mSettings.PresetSettings.empty() && presets.as_array().size() != 1) {
        // clang-format off
        toml::value v(
            toml::table {}
        );
        // clang-format on
        presets.push_back(std::move(v));
    }

    for (const auto& preset : mSettings.PresetSettings) {
        // clang-format off
        toml::value presetValue(
            toml::table {
                { "name", preset.Name },
                { "isDefault", preset.IsDefault },
                { "delimiter", preset.Delimiter },
                { "textQualifier", preset.TextQualifier },
                { "emptyValues", static_cast<int>(preset.EmptyValuesHandler) },
                { "newLines", static_cast<int>(preset.NewLinesHandler) },
                { "excludeHeaders", preset.ExcludeHeaders },
                { "columns", toml::array {} },
            }
        );
        // clang-format on

        auto& columns = presetValue.at("columns");
        for (const auto& column : preset.Columns) {
            // clang-format off
            toml::value columnValue(
                toml::table {
                    { "column", column.Column },
                    { "originalColumn", column.OriginalColumn },
                    { "order", column.Order }
                }
            );
            // clang-format on

            columns.push_back(std::move(columnValue));
        }

        presets.push_back(std::move(presetValue));
    }

    const std::string configString = toml::format(root);

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

    SetExportPath(pEnv->GetExportPath().string());

    // clang-format off
    toml::value root(
        toml::table {
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
            },
            {
                Sections::ExportSection,
                toml::table {
                    { "exportPath", pEnv->GetExportPath().string() },
                    { "presetCount", 0 }
                }
            },
            {
                Sections::PresetsSection,
                toml::array { toml::table {} }
            }
        }
    );
    // clang-format on

    auto& presets = root.at(Sections::PresetsSection);
    presets.as_array_fmt().fmt = toml::array_format::array_of_tables;

    const std::string configString = toml::format(root);

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

bool Configuration::SaveExportPreset(const Common::Preset& preset)
{
    auto configPath = pEnv->GetConfigurationPath().string();

    toml::value root;
    try {
        root = toml::parse(configPath);
    } catch (const toml::syntax_error& error) {
        pLogger->error(
            "Configuration::SaveExportPreset - A TOML syntax/parse error occurred when parsing configuration file {0}",
            error.what());
        return false;
    }

    // clang-format off
    /*toml::value v(
        toml::table {
            {"presets", toml::array {} }
        }
    );*/

    auto& presets = root.at(Sections::PresetsSection);
    toml::value presetValue(
        toml::table {
            { "name", preset.Name },
            { "isDefault", preset.IsDefault },
            { "delimiter", preset.Delimiter },
            { "textQualifier", preset.TextQualifier },
            { "emptyValues", static_cast<int>(preset.EmptyValuesHandler) },
            { "newLines", static_cast<int>(preset.NewLinesHandler) },
            { "excludeHeaders", preset.ExcludeHeaders },
            { "columns", toml::array {} }
        }
    );
    // clang-format on

    auto& columns = presetValue.at("columns");
    for (const auto& column : preset.Columns) {
        // clang-format off
            toml::value columnValue(
                toml::table {
                    { "column", column.Column },
                    { "originalColumn", column.OriginalColumn },
                    { "order", column.Order }
                }
            );
        // clang-format on

        columns.push_back(std::move(columnValue));
    }

    presets.push_back(std::move(presetValue));

    pLogger->info("Configuration::SaveExportPreset - Preset serialized to:\n{0}", toml::format(root));

    const std::string presetConfigString = toml::format(root);

    pLogger->info("Configuration - Probing for configuration file for appending preset at path {0}", configPath);

    std::ofstream configFileStream;
    configFileStream.open(configPath, std::ios_base::out);
    if (!configFileStream) {
        pLogger->error("Configuration - Failed to open configuration file at path {0}", configPath);
        return false;
    }

    configFileStream << presetConfigString;

    configFileStream.close();

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

std::string Configuration::GetExportPath() const
{
    return mSettings.ExportPath;
}

void Configuration::SetExportPath(const std::string& value)
{
    mSettings.ExportPath = value;
}

int Configuration::GetPresetCount() const
{
    return mSettings.PresetCount;
}

void Configuration::SetPresetCount(const int value)
{
    mSettings.PresetCount = value;
}

std::vector<Configuration::PresetSettings> Configuration::GetPresets() const
{
    return mSettings.PresetSettings;
}

void Configuration::SetPresets(const std::vector<PresetSettings>& values)
{
    mSettings.PresetSettings = values;
}

void Configuration::ClearPresets()
{
    mSettings.PresetSettings.clear();
}

void Configuration::GetGeneralConfig(const toml::value& root)
{
    const auto& generalSection = toml::find(root, Sections::GeneralSection);

    mSettings.UserInterfaceLanguage = toml::find<std::string>(generalSection, "lang");
    mSettings.StartOnBoot = toml::find<bool>(generalSection, "startOnBoot");
    auto tomlStartPosition = toml::find<int>(generalSection, "startPosition");
    mSettings.StartPosition = static_cast<WindowState>(tomlStartPosition);
    mSettings.ShowInTray = toml::find<bool>(generalSection, "showInTray");
    mSettings.MinimizeToTray = toml::find<bool>(generalSection, "minimizeToTray");
    mSettings.CloseToTray = toml::find<bool>(generalSection, "closeToTray");
}

void Configuration::GetDatabaseConfig(const toml::value& root)
{
    const auto& databaseSection = toml::find(root, Sections::DatabaseSection);

    mSettings.DatabasePath = toml::find<std::string>(databaseSection, "databasePath");
    mSettings.BackupDatabase = toml::find<bool>(databaseSection, "backupDatabase");
    mSettings.BackupPath = toml::find<std::string>(databaseSection, "backupPath");
    mSettings.BackupRetentionPeriod = toml::find<int>(databaseSection, "backupRetentionPeriod");
}

void Configuration::GetTasksConfig(const toml::value& root)
{
    const auto& taskSection = toml::find(root, Sections::TaskSection);

    mSettings.TaskMinutesIncrement = toml::find<int>(taskSection, "minutesIncrement");
    mSettings.ShowProjectAssociatedCategories = toml::find<bool>(taskSection, "showProjectAssociatedCategories");
}

void Configuration::GetTasksViewConfig(const toml::value& root)
{
    const auto& tasksViewSection = toml::find(root, Sections::TasksViewSection);

    mSettings.TodayAlwaysExpanded = toml::find<bool>(tasksViewSection, "todayAlwaysExpanded");
}

void Configuration::GetExportConfig(const toml::value& root)
{
    const auto& exportSection = toml::find(root, Sections::ExportSection);

    mSettings.ExportPath = toml::find<std::string>(exportSection, "exportPath");
    mSettings.PresetCount = toml::find<int>(exportSection, "presetCount");
}

void Configuration::GetPresetsConfigEx(const toml::value& root)
{
    try {
        auto presetsSectionSize = root.at(Sections::PresetsSection).size();
        for (size_t i = 0; i < presetsSectionSize; i++) {
            if (root.at(Sections::PresetsSection).at(i).as_table().empty()) {
                continue;
            }

            PresetSettings preset;

            preset.Name = root.at(Sections::PresetsSection).at(i).at("name").as_string();
            preset.IsDefault = root.at(Sections::PresetsSection).at(i).at("isDefault").as_boolean();
            preset.Delimiter = root.at(Sections::PresetsSection).at(i).at("delimiter").as_string();
            preset.TextQualifier = root.at(Sections::PresetsSection).at(i).at("textQualifier").as_string();
            preset.EmptyValuesHandler =
                static_cast<EmptyValues>(root.at(Sections::PresetsSection).at(i).at("emptyValues").as_integer());
            preset.NewLinesHandler =
                static_cast<NewLines>(root.at(Sections::PresetsSection).at(i).at("newLines").as_integer());
            preset.ExcludeHeaders = root.at(Sections::PresetsSection).at(i).at("excludeHeaders").as_boolean();

            auto columnsSize = root.at(Sections::PresetsSection).at(i).at("columns").size();
            for (size_t j = 0; j < columnsSize; j++) {
                PresetColumnSettings presetColumn;
                presetColumn.Column =
                    root.at(Sections::PresetsSection).at(i).at("columns").at(j).at("column").as_string();
                presetColumn.OriginalColumn =
                    root.at(Sections::PresetsSection).at(i).at("columns").at(j).at("originalColumn").as_string();
                presetColumn.Order =
                    root.at(Sections::PresetsSection).at(i).at("columns").at(j).at("order").as_integer();

                preset.Columns.push_back(presetColumn);
            }

            // clang-format off
            std::sort(
                preset.Columns.begin(),
                preset.Columns.end(),
                [](const PresetColumnSettings& lhs, const PresetColumnSettings& rhs) {
                    return lhs.Order < rhs.Order;
                }
            );
            // clang-format on

            mSettings.PresetSettings.push_back(preset);
        }
    } catch (const std::out_of_range& error) {
        pLogger->error("Error - {0}", error.what());
    } catch (const toml::type_error& error) {
        pLogger->error("Error - {0}", error.what());
    } catch (const std::exception& error) {
        pLogger->error("Error - {0}", error.what());
    }
}
} // namespace tks::Core
