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

#include "configuration.h"

#include <algorithm>
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

Configuration::PresetColumnSettings::PresetColumnSettings(Common::PresetColumn presetColumn)
{
    Column = presetColumn.Column;
    OriginalColumn = presetColumn.OriginalColumn;
    Order = presetColumn.Order;
}

Configuration::PresetSettings::PresetSettings(Common::Preset preset)
{
    Uuid = preset.Uuid;
    Name = preset.Name;
    IsDefault = preset.IsDefault;
    Delimiter = preset.Delimiter;
    TextQualifier = preset.TextQualifier;
    EmptyValuesHandler = preset.EmptyValuesHandler;
    NewLinesHandler = preset.NewLinesHandler;
    BooleanHandler = preset.BooleanHandler;
    ExcludeHeaders = preset.ExcludeHeaders;
    IncludeAttributes = preset.IncludeAttributes;

    for (auto& presetColumn : preset.Columns) {
        PresetColumnSettings presetColumnSettings(presetColumn);
        Columns.push_back(presetColumnSettings);
    }
}

Configuration::Configuration(std::shared_ptr<Environment> env,
    std::shared_ptr<spdlog::logger> logger)
    : pEnv(env)
    , pLogger(logger)
{
}

bool Configuration::Load()
{
    auto configPath = pEnv->GetConfigurationPath();
    bool exists = std::filesystem::exists(configPath);

    pLogger->info(
        "Configuration - Probing for configuration file at path {0}", configPath.string());

    if (!exists) {
        pLogger->error(
            "Configuration - Failed to find configuration file at {0}", configPath.string());
        return false;
    }

    pLogger->info(
        "Configuration - Successfully located configuration file at path {0}", configPath.string());

    try {
        auto root = toml::parse(configPath.string());

        GetGeneralConfig(root);
        GetDatabaseConfig(root);
        GetTasksConfig(root);
        GetTasksViewConfig(root);
        GetExportConfig(root);
        GetPresetsConfigEx(root);
    } catch (const toml::syntax_error& error) {
        pLogger->error("Configuration - A TOML syntax/parse error occurred when parsing "
                       "configuration file {0}",
            error.what());
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

    // Task section
    root.at(Sections::TaskSection).as_table_fmt().fmt = toml::table_format::multiline;

    root.at(Sections::TaskSection)["minutesIncrement"] = mSettings.TaskMinutesIncrement;
    root.at(Sections::TaskSection)["showProjectAssociatedCategories"] =
        mSettings.ShowProjectAssociatedCategories;
    root.at(Sections::TaskSection)["useLegacyTaskDialog"] = mSettings.UseLegacyTaskDialog;
    root.at(Sections::TaskSection)["useReminders"] = mSettings.UseReminders;
    root.at(Sections::TaskSection)["useNotificationBanners"] = mSettings.UseNotificationBanners;
    root.at(Sections::TaskSection)["openTaskDialogOnReminderClick"] =
        mSettings.OpenTaskDialogOnReminderClick;
    root.at(Sections::TaskSection)["useTaskbarFlashing"] = mSettings.UseTaskbarFlashing;
    root.at(Sections::TaskSection)["reminderInterval"] = mSettings.ReminderInterval;

    // Tasks View section
    root.at(Sections::TasksViewSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::TasksViewSection)["todayAlwaysExpanded"] = mSettings.TodayAlwaysExpanded;

    // Export section
    root.at(Sections::ExportSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::ExportSection)["exportPath"] = mSettings.ExportPath;
    root.at(Sections::ExportSection)["closeExportDialogAfterExporting"] =
        mSettings.CloseExportDialogAfterExporting;
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
                { "uuid", preset.Uuid },
                { "name", preset.Name },
                { "isDefault", preset.IsDefault },
                { "delimiter", static_cast<int>(preset.Delimiter) },
                { "textQualifier", static_cast<int>(preset.TextQualifier) },
                { "emptyValues", static_cast<int>(preset.EmptyValuesHandler) },
                { "newLines", static_cast<int>(preset.NewLinesHandler) },
                { "booleans", static_cast<int>(preset.BooleanHandler) },
                { "excludeHeaders", preset.ExcludeHeaders },
                { "includeAttributes", preset.IncludeAttributes },
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

    pLogger->info(
        "Configuration - Probing for configuration file for writing at path {0}", configFilePath);

    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error(
            "Configuration - Failed to open configuration file at path {0}", configFilePath);
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

    SetDatabasePath(pEnv->GetDatabasePath().string());
    BackupDatabase(false);
    SetBackupPath("");

    SetMinutesIncrement(15);
    ShowProjectAssociatedCategories(false);
    UseLegacyTaskDialog(false);
    UseReminders(false);
    UseNotificationBanners(false);
    UseTaskbarFlashing(false);
    SetReminderInterval(0);
    OpenTaskDialogOnReminderClick(false);

    TodayAlwaysExpanded(false);

    SetExportPath(pEnv->GetExportPath().string());
    CloseExportDialogAfterExporting(false);
    SetPresetCount(0);

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
                }
            },
            {
                Sections::TaskSection,
                toml::table {
                    { "minutesIncrement", 15 },
                    { "showProjectAssociatedCategories", false },
                    { "useLegacyTaskDialog", false },
                    { "useReminders", false },
                    { "useNotificationBanners", false },
                    { "useTaskbarFlashing", false },
                    { "reminderInterval", 0 },
                    { "openTaskDialogOnReminderClick", false }
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
                    { "closeExportDialogAfterExporting", false },
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

    pLogger->info(
        "Configuration - Probing for configuration file for writing at path {0}", configFilePath);

    std::ofstream configFile;
    configFile.open(configFilePath, std::ios_base::out);
    if (!configFile) {
        pLogger->error(
            "Configuration - Failed to open configuration file at path {0}", configFilePath);
        return false;
    }

    configFile << configString;

    configFile.close();
    return true;
}

bool Configuration::SaveExportPreset(const Common::Preset& presetToSave)
{
    auto configPath = pEnv->GetConfigurationPath().string();

    toml::value root;
    try {
        root = toml::parse(configPath);
    } catch (const toml::syntax_error& error) {
        pLogger->error("Configuration::SaveExportPreset - A TOML syntax/parse error occurred when "
                       "parsing configuration file {0}",
            error.what());
        return false;
    }

    root.at(Sections::ExportSection)["presetCount"] = GetPresetCount() + 1;

    auto& presets = root.at(Sections::PresetsSection);
    // clang-format off
    toml::value presetValue(
        toml::table {
            { "uuid", presetToSave.Uuid },
            { "name", presetToSave.Name },
            { "isDefault", presetToSave.IsDefault },
            { "delimiter", static_cast<int>(presetToSave.Delimiter) },
            { "textQualifier", static_cast<int>(presetToSave.TextQualifier) },
            { "emptyValues", static_cast<int>(presetToSave.EmptyValuesHandler) },
            { "newLines", static_cast<int>(presetToSave.NewLinesHandler) },
            { "booleans", static_cast<int>(presetToSave.BooleanHandler) },
            { "excludeHeaders", presetToSave.ExcludeHeaders },
            { "includeAttributes", presetToSave.IncludeAttributes },
            { "columns", toml::array {} }
        }
    );
    // clang-format on

    auto& columns = presetValue.at("columns");
    for (const auto& column : presetToSave.Columns) {
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

    pLogger->info(
        "Configuration::SaveExportPreset - Preset serialized to:\n{0}", toml::format(root));

    // update/save ptr data
    PresetSettings newPreset(presetToSave);
    SetPreset(newPreset);

    // save settings to file
    const std::string presetConfigString = toml::format(root);

    pLogger->info("Configuration - Probing for configuration file for appending preset at path {0}",
        configPath);

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

bool Configuration::UpdateExportPreset(const Common::Preset& presetToUpdate)
{
    auto configPath = pEnv->GetConfigurationPath().string();

    toml::value root;
    try {
        root = toml::parse(configPath);
    } catch (const toml::syntax_error& error) {
        pLogger->error(
            "Configuration::UpdateExportPreset - A TOML syntax/parse error occurred when parsing "
            "configuration file {0}",
            error.what());
        return false;
    }

    auto& presets = root.at(Sections::PresetsSection).as_array();
    for (auto& preset : presets) {
        if (preset.as_table().empty()) {
            continue;
        }

        if (preset["uuid"].as_string() == presetToUpdate.Uuid) {
            preset["name"] = presetToUpdate.Name;
            preset["isDefault"] = presetToUpdate.IsDefault;
            preset["delimiter"] = static_cast<int>(presetToUpdate.Delimiter);
            preset["textQualifier"] = static_cast<int>(presetToUpdate.TextQualifier);
            preset["emptyValues"] = static_cast<int>(presetToUpdate.EmptyValuesHandler);
            preset["newLines"] = static_cast<int>(presetToUpdate.NewLinesHandler);
            preset["booleans"] = static_cast<int>(presetToUpdate.BooleanHandler);
            preset["excludeHeaders"] = presetToUpdate.ExcludeHeaders;
            preset["includeAttributes"] = presetToUpdate.IncludeAttributes;

            auto& columns = preset.at("columns").as_array();
            columns.clear();

            for (const auto& column : presetToUpdate.Columns) {
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
            break;
        }
    }

    pLogger->info(
        "Configuration::UpdateExportPreset - Preset serialized to:\n{0}", toml::format(root));

    // update ptr data
    PresetSettings updatedPresetSettings(presetToUpdate);
    EmplacePreset(updatedPresetSettings);

    const std::string presetConfigString = toml::format(root);

    pLogger->info("Configuration::UpdateExportPreset - Probing for configuration file for "
                  "appending preset at path {0}",
        configPath);

    std::ofstream configFileStream;
    configFileStream.open(configPath, std::ios_base::out);
    if (!configFileStream) {
        pLogger->error(
            "Configuration::UpdateExportPreset - Failed to open configuration file at path {0}",
            configPath);
        return false;
    }

    configFileStream << presetConfigString;

    configFileStream.close();

    return true;
}

bool Configuration::TryUnsetDefaultPreset()
{
    auto configPath = pEnv->GetConfigurationPath().string();

    toml::value root;
    try {
        root = toml::parse(configPath);
    } catch (const toml::syntax_error& error) {
        pLogger->error("Configuration::TryUnsetDefaultPreset - A TOML syntax/parse error occurred "
                       "when parsing file {0}",
            error.what());
        return false;
    }

    auto& presets = root.at(Sections::PresetsSection).as_array();
    for (auto& preset : presets) {
        if (preset.as_table().empty()) {
            continue;
        }

        preset["isDefault"] = false;
    }

    pLogger->info(
        "Configuration::TryUnsetDefaultPreset - Preset serialized to:\n{0}", toml::format(root));

    const std::string presetConfigString = toml::format(root);

    pLogger->info("Configuration::TryUnsetDefaultPreset - Probing for configuration file for "
                  "appending preset at path {0}",
        configPath);

    std::ofstream configFileStream;
    configFileStream.open(configPath, std::ios_base::out);
    if (!configFileStream) {
        pLogger->error(
            "Configuration::TryUnsetDefaultPreset - Failed to open configuration file at path {0}",
            configPath);
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

bool Configuration::UseLegacyTaskDialog() const
{
    return mSettings.UseLegacyTaskDialog;
}

void Configuration::UseLegacyTaskDialog(const bool value)
{
    mSettings.UseLegacyTaskDialog = value;
}

bool Configuration::UseReminders() const
{
    return mSettings.UseReminders;
}

void Configuration::UseReminders(const bool value)
{
    mSettings.UseReminders = value;
}

bool Configuration::UseNotificationBanners() const
{
    return mSettings.UseNotificationBanners;
}

void Configuration::UseNotificationBanners(const bool value)
{
    mSettings.UseNotificationBanners = value;
}

bool Configuration::UseTaskbarFlashing() const
{
    return mSettings.UseTaskbarFlashing;
}

void Configuration::UseTaskbarFlashing(const bool value)
{
    mSettings.UseTaskbarFlashing = value;
}

int Configuration::ReminderInterval() const
{
    return mSettings.ReminderInterval;
}

void Configuration::SetReminderInterval(const int value)
{
    mSettings.ReminderInterval = value;
}

bool Configuration::OpenTaskDialogOnReminderClick() const
{
    return mSettings.OpenTaskDialogOnReminderClick;
}

void Configuration::OpenTaskDialogOnReminderClick(const bool value)
{
    mSettings.OpenTaskDialogOnReminderClick = value;
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

bool Configuration::CloseExportDialogAfterExporting() const
{
    return mSettings.CloseExportDialogAfterExporting;
}

void Configuration::CloseExportDialogAfterExporting(const bool value)
{
    mSettings.CloseExportDialogAfterExporting = value;
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

void Configuration::SetPreset(const PresetSettings& value)
{
    mSettings.PresetSettings.push_back(value);
}

void Configuration::EmplacePreset(const PresetSettings& value)
{
    // clang-format off
    mSettings.PresetSettings.erase(
        std::remove_if(
            mSettings.PresetSettings.begin(),
            mSettings.PresetSettings.end(),
            [&](const PresetSettings& preset) {
                return preset.Uuid == value.Uuid;
            }),
        mSettings.PresetSettings.end()
    );
    // clang-format on

    mSettings.PresetSettings.push_back(value);
}

void Configuration::ClearPresets()
{
    mSettings.PresetSettings.clear();
}

void Configuration::GetGeneralConfig(const toml::value& root)
{
    const auto& generalSection = toml::find(root, Sections::GeneralSection);

    mSettings.UserInterfaceLanguage = toml::find_or<std::string>(generalSection, "lang", "en-US");

    mSettings.StartOnBoot = toml::find_or<bool>(generalSection, "startOnBoot", false);

    auto tomlStartPosition =
        toml::find_or<int>(generalSection, "startPosition", static_cast<int>(WindowState::Normal));
    mSettings.StartPosition = static_cast<WindowState>(tomlStartPosition);

    mSettings.ShowInTray = toml::find_or<bool>(generalSection, "showInTray", false);

    mSettings.MinimizeToTray = toml::find_or<bool>(generalSection, "minimizeToTray", false);

    mSettings.CloseToTray = toml::find_or<bool>(generalSection, "closeToTray", false);
}

void Configuration::GetDatabaseConfig(const toml::value& root)
{
    const auto& databaseSection = toml::find(root, Sections::DatabaseSection);

    mSettings.DatabasePath = toml::find_or<std::string>(
        databaseSection, "databasePath", pEnv->GetDatabasePath().string());

    mSettings.BackupDatabase = toml::find_or<bool>(databaseSection, "backupDatabase", false);

    mSettings.BackupPath = toml::find_or<std::string>(databaseSection, "backupPath", "");
}

void Configuration::GetTasksConfig(const toml::value& root)
{
    const auto& taskSection = toml::find(root, Sections::TaskSection);

    mSettings.TaskMinutesIncrement = toml::find_or<int>(taskSection, "minutesIncrement", 15);

    mSettings.ShowProjectAssociatedCategories =
        toml::find_or<bool>(taskSection, "showProjectAssociatedCategories", false);

    mSettings.UseLegacyTaskDialog = toml::find_or<bool>(taskSection, "useLegacyTaskDialog", false);

    mSettings.UseReminders = toml::find_or<bool>(taskSection, "useReminders", false);

    mSettings.UseNotificationBanners =
        toml::find_or<bool>(taskSection, "useNotificationBanners", false);

    mSettings.OpenTaskDialogOnReminderClick =
        toml::find_or<bool>(taskSection, "openTaskDialogOnReminderClick", false);

    mSettings.UseTaskbarFlashing = toml::find_or<bool>(taskSection, "useTaskbarFlashing", false);

    mSettings.ReminderInterval = toml::find_or<int>(taskSection, "reminderInterval", 0);
}

void Configuration::GetTasksViewConfig(const toml::value& root)
{
    const auto& tasksViewSection = toml::find(root, Sections::TasksViewSection);

    mSettings.TodayAlwaysExpanded =
        toml::find_or<bool>(tasksViewSection, "todayAlwaysExpanded", false);
}

void Configuration::GetExportConfig(const toml::value& root)
{
    const auto& exportSection = toml::find(root, Sections::ExportSection);

    mSettings.ExportPath =
        toml::find_or<std::string>(exportSection, "exportPath", pEnv->GetExportPath().string());

    mSettings.CloseExportDialogAfterExporting =
        toml::find_or<bool>(exportSection, "closeExportDialogAfterExporting", false);
    mSettings.PresetCount = toml::find_or<int>(exportSection, "presetCount", 0);
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

            preset.Uuid = root.at(Sections::PresetsSection).at(i).at("uuid").as_string();
            preset.Uuid = toml::get_or(root.at(Sections::PresetsSection).at(i).at("uuid"), "");

            preset.Name = root.at(Sections::PresetsSection).at(i).at("name").as_string();
            preset.IsDefault = root.at(Sections::PresetsSection).at(i).at("isDefault").as_boolean();
            preset.Delimiter = static_cast<DelimiterType>(
                root.at(Sections::PresetsSection).at(i).at("delimiter").as_integer());
            preset.TextQualifier = static_cast<TextQualifierType>(
                root.at(Sections::PresetsSection).at(i).at("textQualifier").as_integer());
            preset.EmptyValuesHandler = static_cast<EmptyValues>(
                root.at(Sections::PresetsSection).at(i).at("emptyValues").as_integer());
            preset.NewLinesHandler = static_cast<NewLines>(
                root.at(Sections::PresetsSection).at(i).at("newLines").as_integer());
            preset.BooleanHandler = static_cast<BooleanHandler>(
                root.at(Sections::PresetsSection).at(i).at("booleans").as_integer());
            preset.ExcludeHeaders =
                root.at(Sections::PresetsSection).at(i).at("excludeHeaders").as_boolean();
            preset.IncludeAttributes =
                root.at(Sections::PresetsSection).at(i).at("includeAttributes").as_boolean();

            auto columnsSize = root.at(Sections::PresetsSection).at(i).at("columns").size();
            for (size_t j = 0; j < columnsSize; j++) {
                PresetColumnSettings presetColumn;
                presetColumn.Column = root.at(Sections::PresetsSection)
                                          .at(i)
                                          .at("columns")
                                          .at(j)
                                          .at("column")
                                          .as_string();
                presetColumn.OriginalColumn = root.at(Sections::PresetsSection)
                                                  .at(i)
                                                  .at("columns")
                                                  .at(j)
                                                  .at("originalColumn")
                                                  .as_string();
                presetColumn.Order = static_cast<int>(root.at(Sections::PresetsSection)
                        .at(i)
                        .at("columns")
                        .at(j)
                        .at("order")
                        .as_integer());

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
