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

#include "../utils/utils.h"

namespace tks::Core
{
const std::string Sections::GeneralSection = "general";
const std::string Sections::DatabaseSection = "database";
const std::string Sections::TaskSection = "tasks";
const std::string Sections::TasksViewSection = "tasksView";
const std::string Sections::ExportSection = "export";
const std::string Sections::PresetsSection = "presets";

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

bool Configuration::LoadAndOrRecreate()
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Looking for configuration file at path \"{0}\"",
        pEnv->GetConfigurationPath().string());

    if (!std::filesystem::exists(pEnv->GetConfigurationPath())) {
        pLogger->warn(
            "Failed to find configuration file at \"{0}\". Creating new one from defaults",
            pEnv->GetConfigurationPath().string());

        if (!RestoreDefaults()) {
            pLogger->error(
                "Failed to recreate configuration file. See earlier logs for more detail");
            return false;
        }
    }

    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationPath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return false;
    }

    GetGeneralConfig(root);
    GetDatabaseConfig(root);
    GetTasksConfig(root);
    GetTasksViewConfig(root);
    GetExportConfig(root);
    GetPresetsConfig(root);

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
    if (mSettings.PresetSettings.size() > 0) {
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
            columns.as_array_fmt().fmt = toml::array_format::array_of_tables;

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

            if (!root.contains(Sections::PresetsSection)) {
                toml::value presetArray(toml::array{ presetValue });
                root[Sections::PresetsSection] = presetArray;
            } else {
                auto& presets = root.at(Sections::PresetsSection);
                presets.push_back(std::move(presetValue));
            }

            root[Sections::PresetsSection].as_array_fmt().fmt = toml::array_format::array_of_tables;
        }
    }

    const std::string tomlContentsString = toml::format(root);

    bool writeSuccess = WriteTomlContentsToFile(tomlContentsString);
    return writeSuccess;
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
            }
        }
    );
    // clang-format on

    const std::string tomlContentsString = toml::format(root);

    bool writeSuccess = WriteTomlContentsToFile(tomlContentsString);
    return writeSuccess;
}

bool Configuration::SaveExportPreset(const Common::Preset& presetToSave)
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationPath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return false;
    }

    root.at(Sections::ExportSection)["presetCount"] = GetPresetCount() + 1;

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
    columns.as_array_fmt().fmt = toml::array_format::array_of_tables;

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

    if (!root.contains(Sections::PresetsSection)) {
        toml::value presetArray(toml::array{ presetValue });
        root[Sections::PresetsSection] = presetArray;
    } else {
        auto& presets = root.at(Sections::PresetsSection);
        presets.push_back(std::move(presetValue));
    }

    PresetSettings newPreset(presetToSave);
    AddPreset(newPreset);

    const std::string tomlContentsString = toml::format(root);

    bool writeSuccess = WriteTomlContentsToFile(tomlContentsString);
    return writeSuccess;
}

bool Configuration::UpdateExportPreset(const Common::Preset& presetToUpdate)
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationPath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return false;
    }

    auto& presets = root.at(Sections::PresetsSection).as_array();
    for (auto& preset : presets) {
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

            auto& columns = preset.at("columns");
            columns.as_array_fmt().fmt = toml::array_format::array_of_tables;

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

    PresetSettings updatedPresetSettings(presetToUpdate);
    EmplacePreset(updatedPresetSettings);

    const std::string tomlContentsString = toml::format(root);

    bool writeSuccess = WriteTomlContentsToFile(tomlContentsString);
    return writeSuccess;
}

bool Configuration::TryUnsetDefaultPreset()
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationPath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return false;
    }

    if (!root.contains(Sections::PresetsSection)) {
        return true;
    }

    auto& presets = root.at(Sections::PresetsSection).as_array();
    for (auto& preset : presets) {
        preset["isDefault"] = false;
    }

    const std::string tomlContentsString = toml::format(root);

    bool writeSuccess = WriteTomlContentsToFile(tomlContentsString);
    return writeSuccess;
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

void Configuration::AddPreset(const PresetSettings& value)
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

bool Configuration::WriteTomlContentsToFile(const std::string& fileContents)
{
    const std::string configFilePath = pEnv->GetConfigurationPath().string();

    SPDLOG_LOGGER_TRACE(pLogger, "Looking for configuration file at path \"{0}\"", configFilePath);

    std::ofstream configFileStream;
    configFileStream.open(configFilePath, std::ios::out);
    if (!configFileStream.is_open()) {
        pLogger->error("Failed to open configuration file at path \"{0}\"", configFilePath);
        return false;
    }

    configFileStream << fileContents;

    configFileStream.close();
    return true;
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

void Configuration::GetPresetsConfig(const toml::value& root)
{
    if (!root.contains(Sections::PresetsSection)) {
        return;
    }

    const auto& presetSection = toml::find(root, Sections::PresetsSection);

    if (!presetSection.is_array_of_tables()) {
        return;
    }

    for (size_t i = 0; i < presetSection.size(); i++) {
        PresetSettings preset;

        preset.Uuid = toml::find_or<std::string>(presetSection[i], "uuid", Utils::Uuid());
        preset.Name = toml::find_or<std::string>(presetSection[i], "name", "<MissingName>");
        preset.IsDefault = toml::find_or<bool>(presetSection[i], "isDefault", false);
        preset.Delimiter = static_cast<DelimiterType>(
            toml::find_or<int>(presetSection[i], "delimiter", 1 /*Comma*/));
        preset.TextQualifier = static_cast<TextQualifierType>(
            toml::find_or<int>(presetSection[i], "textQualifier", 1 /*None*/));
        preset.EmptyValuesHandler = static_cast<EmptyValues>(
            toml::find_or<int>(presetSection[i], "emptyValues", 0 /*None*/));
        preset.NewLinesHandler =
            static_cast<NewLines>(toml::find_or<int>(presetSection[i], "newLines", 0 /*None*/));
        preset.BooleanHandler = static_cast<BooleanHandler>(
            toml::find_or<int>(presetSection[i], "booleans", 0 /*None*/));
        preset.ExcludeHeaders = toml::find_or<bool>(presetSection[i], "excludeHeaders", false);
        preset.IncludeAttributes =
            toml::find_or<bool>(presetSection[i], "includeAttributes", false);

        const auto& columnsArrayTable = toml::find(presetSection[i], "columns");

        bool failedToFindPresetColumns = false;
        try {
            if (columnsArrayTable.is_array()) {
                for (size_t j = 0; j < columnsArrayTable.size(); j++) {
                    PresetColumnSettings presetColumn;
                    presetColumn.Column = toml::find<std::string>(columnsArrayTable[j], "column");
                    presetColumn.OriginalColumn =
                        toml::find<std::string>(columnsArrayTable[j], "originalColumn");
                    presetColumn.Order = toml::find<int>(columnsArrayTable[j], "order");

                    preset.Columns.push_back(presetColumn);
                }
            }
        } catch (const std::out_of_range& error) {
            pLogger->error("Error - {0}", error.what());
            failedToFindPresetColumns = true;
        } catch (const toml::type_error& error) {
            pLogger->error("Error - {0}", error.what());
            failedToFindPresetColumns = true;
        }

        if (failedToFindPresetColumns) {
            preset.Columns.clear();
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

    if (mSettings.PresetCount == 0 && mSettings.PresetSettings.size() > 0) {
        mSettings.PresetCount = static_cast<int>(mSettings.PresetSettings.size());
    }
}
} // namespace tks::Core
