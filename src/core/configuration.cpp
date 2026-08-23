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

#include "configuration.h"

#include <algorithm>
#include <filesystem>

#include "environment.h"

#include "../common/messages/configmessages.h"

#include "../utils/utils.h"

namespace tks::Core
{
const std::string Sections::GeneralSection = "general";
const std::string Sections::DatabaseSection = "database";
const std::string Sections::TaskSection = "tasks";
const std::string Sections::TasksViewSection = "tasksView";
const std::string Sections::ExportSection = "export";
const std::string Sections::PresetsSection = "presets";

Configuration::Configuration(std::shared_ptr<Environment> env,
    std::shared_ptr<spdlog::logger> logger)
    : pSettings(std::make_unique<Settings::Settings>())
    , pEnv(env)
    , pLogger(logger)
{
}

ConfigResult Configuration::LoadAndOrRecreate()
{
    SPDLOG_LOGGER_TRACE(pLogger,
        "Looking for configuration file at path \"{0}\"",
        pEnv->GetConfigurationFilePath().string());

    if (!std::filesystem::exists(pEnv->GetConfigurationFilePath())) {
        pLogger->warn(
            "Failed to find configuration file at \"{0}\". Creating new one from defaults",
            pEnv->GetConfigurationFilePath().string());

        auto result = RestoreDefaults();
        if (!result.Success) {
            pLogger->error(
                "Failed to recreate configuration file. See earlier logs for more detail");
            return result;
        }
    }

    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationFilePath().string());

        GetGeneralConfig(root);
        GetDatabaseConfig(root);
        GetTasksConfig(root);
        GetTasksViewConfig(root);
        GetExportConfig(root);
        GetPresetsConfig(root);
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());

        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
    } catch (const toml::type_error& error) {
        pLogger->error(
            "A TOML type error occurred when reading configuration values - {0}", error.what());

        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
    } catch (const std::out_of_range& error) {
        pLogger->error(
            "A (TOML) out-of-range error occurred when reading configuration values - {0}",
            error.what());

        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
    }

    return ConfigResult::OK();
}

ConfigResult Configuration::Save()
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
    root.at(Sections::GeneralSection)["lang"] = pSettings->UserInterfaceLanguage;
    root.at(Sections::GeneralSection)["startOnBoot"] = pSettings->StartOnBoot;
    root.at(Sections::GeneralSection)["startPosition"] = static_cast<int>(pSettings->StartPosition);
    root.at(Sections::GeneralSection)["showInTray"] = pSettings->ShowInTray;
    root.at(Sections::GeneralSection)["minimizeToTray"] = pSettings->MinimizeToTray;
    root.at(Sections::GeneralSection)["closeToTray"] = pSettings->CloseToTray;

    // Database section
    root.at(Sections::DatabaseSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::DatabaseSection)["databaseFileName"] = pSettings->DatabaseFileName;
    root.at(Sections::DatabaseSection)["databasePath"] = pSettings->DatabasePath;
    root.at(Sections::DatabaseSection)["backupDatabase"] = pSettings->BackupDatabase;
    root.at(Sections::DatabaseSection)["backupPath"] = pSettings->BackupPath;
    root.at(Sections::DatabaseSection)["backupOnProgramClose"] = pSettings->BackupOnProgramClose;
    root.at(Sections::DatabaseSection)["zipBackupFile"] = pSettings->ZipBackupFile;

    // Task section
    root.at(Sections::TaskSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::TaskSection)["minutesIncrement"] = pSettings->TaskMinutesIncrement;
    root.at(Sections::TaskSection)["maximumDescriptionLength"] =
        pSettings->MaximumDescriptionLength;
    root.at(Sections::TaskSection)["showProjectAssociatedCategories"] =
        pSettings->ShowProjectAssociatedCategories;
    root.at(Sections::TaskSection)["useReminders"] = pSettings->UseReminders;
    root.at(Sections::TaskSection)["useNotificationBanners"] = pSettings->UseNotificationBanners;
    root.at(Sections::TaskSection)["openTaskDialogOnReminderClick"] =
        pSettings->OpenTaskDialogOnReminderClick;
    root.at(Sections::TaskSection)["useTaskbarFlashing"] = pSettings->UseTaskbarFlashing;
    root.at(Sections::TaskSection)["reminderInterval"] = pSettings->ReminderInterval;
    root.at(Sections::TaskSection)["openTaskDialogOnOutlookMeetingAttendanceCheck"] =
        pSettings->OpenTaskDialogOnOutlookMeetingAttendanceCheck;

    // Tasks View section
    root.at(Sections::TasksViewSection).as_table_fmt().fmt = toml::table_format::multiline;

    // Tasks View Columns (sub)section
    toml::value tasksViewColumnArray(toml::array{});
    tasksViewColumnArray.as_array_fmt().fmt = toml::array_format::multiline;
    tasksViewColumnArray.as_array_fmt().body_indent = 4;
    tasksViewColumnArray.as_array_fmt().closing_indent = 0;

    if (pSettings->TasksViewColumnSettings.size() == 0) {
        pSettings->TasksViewColumnSettings = Settings::MakeDefaultTasksViewColumnList();
    }

    for (const auto& tasksViewColumn : pSettings->TasksViewColumnSettings) {
        // clang-format off
        toml::value value(
            toml::table {
                { "name", tasksViewColumn.Name },
                { "displayName", tasksViewColumn.DisplayName },
                { "order", tasksViewColumn.Order },
                { "textAlignment", static_cast<int>(tasksViewColumn.TextAlignment) },
                { "type", static_cast<int>(tasksViewColumn.Type) },
                { "ellipsisMode", static_cast<int>(tasksViewColumn.EllipsisMode) },
                { "width", tasksViewColumn.Width },
                { "selected", tasksViewColumn.Selected },
                { "id", static_cast<int>(tasksViewColumn.TaskViewColumnId) },
            }
        );
        // clang-format on

        tasksViewColumnArray.push_back(std::move(value));
    }

    root.at(Sections::TasksViewSection)["tasksViewColumns"] = tasksViewColumnArray;

    // Export section
    root.at(Sections::ExportSection).as_table_fmt().fmt = toml::table_format::multiline;
    root.at(Sections::ExportSection)["exportPath"] = pSettings->ExportPath;
    root.at(Sections::ExportSection)["closeExportDialogAfterExporting"] =
        pSettings->CloseExportDialogAfterExporting;
    root.at(Sections::ExportSection)["presetCount"] = pSettings->PresetCount;

    // Presets section
    if (pSettings->PresetSettings.size() > 0) {
        for (const auto& preset : pSettings->PresetSettings) {
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

    auto result = WriteTomlContentsToFile(tomlContentsString);
    return result;
}

ConfigResult Configuration::RestoreDefaults()
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
    BackupOnProgramClose(false);
    ZipBackupFile(false);

    SetMinutesIncrement(15);
    SetMaximumDescriptionLength(3000);
    ShowProjectAssociatedCategories(false);
    UseReminders(false);
    UseNotificationBanners(false);
    UseTaskbarFlashing(false);
    SetReminderInterval(0);
    OpenTaskDialogOnReminderClick(false);
    OpenTaskDialogOnOutlookMeetingAttendanceCheck(false);

    SetTasksViewColumns(Settings::MakeDefaultTasksViewColumnList());

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
                    { "closeToTray", false },
                }
            },
            {
                Sections::DatabaseSection,
                toml::table {
                    { "databaseFileName", pEnv->GetDatabaseFileName() },
                    { "databasePath", pEnv->GetDatabasePath().string() },
                    { "backupDatabase", false },
                    { "backupPath", "" },
                    { "backupOnProgramClose", false },
                    { "zipBackupFile", false }
                }
            },
            {
                Sections::TaskSection,
                toml::table {
                    { "minutesIncrement", 15 },
                    { "maximumDescriptionLength", 3000 },
                    { "showProjectAssociatedCategories", false },
                    { "useLegacyTaskDialog", false },
                    { "useReminders", false },
                    { "useNotificationBanners", false },
                    { "useTaskbarFlashing", false },
                    { "reminderInterval", 0 },
                    { "openTaskDialogOnReminderClick", false },
                    { "openTaskDialogOnOutlookMeetingAttendanceCheck", true }
                }
            },
            {
                Sections::TasksViewSection,
                toml::table {
                    { "tasksViewColumns", toml::array {} }
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

    // Tasks View Columns section
    toml::value tasksViewColumnArray(toml::array{});
    tasksViewColumnArray.as_array_fmt().fmt = toml::array_format::multiline;
    tasksViewColumnArray.as_array_fmt().body_indent = 4;
    tasksViewColumnArray.as_array_fmt().closing_indent = 0;

    for (const auto& tasksViewColumn : pSettings->TasksViewColumnSettings) {
        // clang-format off
        toml::value value(
            toml::table {
                { "name", tasksViewColumn.Name },
                { "displayName", tasksViewColumn.Name },
                { "order", tasksViewColumn.Order },
                { "textAlignment", static_cast<int>(tasksViewColumn.TextAlignment) },
                { "type", static_cast<int>(tasksViewColumn.Type) },
                { "ellipsisMode", static_cast<int>(tasksViewColumn.EllipsisMode) },
                { "width", tasksViewColumn.Width },
                { "selected", tasksViewColumn.Selected },
                { "id", static_cast<int>(tasksViewColumn.TaskViewColumnId) },
            }
        );
        // clang-format on

        tasksViewColumnArray.push_back(std::move(value));
    }

    root.at(Sections::TasksViewSection)["tasksViewColumns"] = tasksViewColumnArray;

    const std::string tomlContentsString = toml::format(root);

    auto result = WriteTomlContentsToFile(tomlContentsString);
    return result;
}

ConfigResult Configuration::SaveExportPreset(const Settings::PresetSetting& presetToSave)
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationFilePath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
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

    Settings::PresetSetting newPreset(presetToSave);
    AddPreset(newPreset);

    const std::string tomlContentsString = toml::format(root);

    auto result = WriteTomlContentsToFile(tomlContentsString);
    return result;
}

ConfigResult Configuration::UpdateExportPreset(const Settings::PresetSetting& presetToUpdate)
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationFilePath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
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
            preset["columns"] = toml::array{};

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

    Settings::PresetSetting updatedPresetSettings(presetToUpdate);
    EmplacePreset(updatedPresetSettings);

    const std::string tomlContentsString = toml::format(root);

    auto result = WriteTomlContentsToFile(tomlContentsString);
    return result;
}

ConfigResult Configuration::TryUnsetDefaultPreset()
{
    toml::value root;
    try {
        root = toml::parse(pEnv->GetConfigurationFilePath().string());
    } catch (const toml::syntax_error& error) {
        pLogger->error("A TOML syntax/parse error occurred when parsing configuration file \"{0}\"",
            error.what());
        return ConfigResult::Fail(Messages::ConfigurationFileParseHeaderMessage,
            Messages::CongfigurationFileParseUserMessage,
            fmt::format(Messages::CongfigurationFileParseErrorMessage,
                pEnv->GetConfigurationFilePath().string(),
                error.what()));
    }

    if (!root.contains(Sections::PresetsSection)) {
        return ConfigResult::OK();
    }

    auto& presets = root.at(Sections::PresetsSection).as_array();
    for (auto& preset : presets) {
        preset["isDefault"] = false;
    }

    const std::string tomlContentsString = toml::format(root);

    auto result = WriteTomlContentsToFile(tomlContentsString);
    return result;
}

std::string Configuration::GetUserInterfaceLanguage() const
{
    return pSettings->UserInterfaceLanguage;
}

void Configuration::SetUserInterfaceLanguage(const std::string& value)
{
    pSettings->UserInterfaceLanguage = value;
}

bool Configuration::StartOnBoot() const
{
    return pSettings->StartOnBoot;
}

void Configuration::StartOnBoot(const bool value)
{
    pSettings->StartOnBoot = value;
}

WindowState Configuration::GetWindowState() const
{
    return pSettings->StartPosition;
}

void Configuration::SetWindowState(const WindowState value)
{
    pSettings->StartPosition = value;
}

bool Configuration::ShowInTray() const
{
    return pSettings->ShowInTray;
}

void Configuration::ShowInTray(const bool value)
{
    pSettings->ShowInTray = value;
}

bool Configuration::MinimizeToTray() const
{
    return pSettings->MinimizeToTray;
}

void Configuration::MinimizeToTray(const bool value)
{
    pSettings->MinimizeToTray = value;
}

bool Configuration::CloseToTray() const
{
    return pSettings->CloseToTray;
}

void Configuration::CloseToTray(const bool value)
{
    pSettings->CloseToTray = value;
}

std::string Configuration::GetDatabaseFileName() const
{
    return pSettings->DatabaseFileName;
}

void Configuration::SetDatabaseFileName(const std::string& value)
{
    pSettings->DatabaseFileName = value;
}

std::string Configuration::GetDatabasePath() const
{
    return pSettings->DatabasePath;
}

void Configuration::SetDatabasePath(const std::string& value)
{
    pSettings->DatabasePath = value;
}

bool Configuration::BackupDatabase() const
{
    return pSettings->BackupDatabase;
}

void Configuration::BackupDatabase(const bool value)
{
    pSettings->BackupDatabase = value;
}

std::string Configuration::GetBackupPath() const
{
    return pSettings->BackupPath;
}

void Configuration::SetBackupPath(const std::string& value)
{
    pSettings->BackupPath = value;
}

bool Configuration::BackupOnProgramClose() const
{
    return pSettings->BackupOnProgramClose;
}

void Configuration::BackupOnProgramClose(const bool value)
{
    pSettings->BackupOnProgramClose = value;
}

bool Configuration::ZipBackupFile() const
{
    return pSettings->ZipBackupFile;
}

void Configuration::ZipBackupFile(const bool value)
{
    pSettings->ZipBackupFile = value;
}

int Configuration::GetMinutesIncrement() const
{
    return pSettings->TaskMinutesIncrement;
}

void Configuration::SetMinutesIncrement(const int value)
{
    pSettings->TaskMinutesIncrement = value;
}

int Configuration::GetMaximumDescriptionLength() const
{
    return pSettings->MaximumDescriptionLength;
}

void Configuration::SetMaximumDescriptionLength(const int value)
{
    pSettings->MaximumDescriptionLength = value;
}

bool Configuration::ShowProjectAssociatedCategories() const
{
    return pSettings->ShowProjectAssociatedCategories;
}

void Configuration::ShowProjectAssociatedCategories(const bool value)
{
    pSettings->ShowProjectAssociatedCategories = value;
}

bool Configuration::UseReminders() const
{
    return pSettings->UseReminders;
}

void Configuration::UseReminders(const bool value)
{
    pSettings->UseReminders = value;
}

bool Configuration::UseNotificationBanners() const
{
    return pSettings->UseNotificationBanners;
}

void Configuration::UseNotificationBanners(const bool value)
{
    pSettings->UseNotificationBanners = value;
}

bool Configuration::UseTaskbarFlashing() const
{
    return pSettings->UseTaskbarFlashing;
}

void Configuration::UseTaskbarFlashing(const bool value)
{
    pSettings->UseTaskbarFlashing = value;
}

int Configuration::ReminderInterval() const
{
    return pSettings->ReminderInterval;
}

void Configuration::SetReminderInterval(const int value)
{
    pSettings->ReminderInterval = value;
}

bool Configuration::OpenTaskDialogOnReminderClick() const
{
    return pSettings->OpenTaskDialogOnReminderClick;
}

void Configuration::OpenTaskDialogOnReminderClick(const bool value)
{
    pSettings->OpenTaskDialogOnReminderClick = value;
}

bool Configuration::OpenTaskDialogOnOutlookMeetingAttendanceCheck() const
{
    return pSettings->OpenTaskDialogOnOutlookMeetingAttendanceCheck;
}

void Configuration::OpenTaskDialogOnOutlookMeetingAttendanceCheck(const bool value)
{
    pSettings->OpenTaskDialogOnOutlookMeetingAttendanceCheck = value;
}

std::vector<Settings::TasksViewColumnSetting> Configuration::GetTasksViewColumns() const
{
    return pSettings->TasksViewColumnSettings;
}

void Configuration::SetTasksViewColumns(const std::vector<Settings::TasksViewColumnSetting> values)
{
    pSettings->TasksViewColumnSettings = values;
}

std::string Configuration::GetExportPath() const
{
    return pSettings->ExportPath;
}

void Configuration::SetExportPath(const std::string& value)
{
    pSettings->ExportPath = value;
}

bool Configuration::CloseExportDialogAfterExporting() const
{
    return pSettings->CloseExportDialogAfterExporting;
}

void Configuration::CloseExportDialogAfterExporting(const bool value)
{
    pSettings->CloseExportDialogAfterExporting = value;
}

int Configuration::GetPresetCount() const
{
    return pSettings->PresetCount;
}

void Configuration::SetPresetCount(const int value)
{
    pSettings->PresetCount = value;
}

std::vector<Settings::PresetSetting> Configuration::GetPresets() const
{
    return pSettings->PresetSettings;
}

void Configuration::SetPresets(const std::vector<Settings::PresetSetting>& values)
{
    pSettings->PresetSettings = values;
}

void Configuration::AddPreset(const Settings::PresetSetting& value)
{
    pSettings->PresetSettings.push_back(value);
}

void Configuration::EmplacePreset(const Settings::PresetSetting& value)
{
    // clang-format off
    pSettings->PresetSettings.erase(
        std::remove_if(
            pSettings->PresetSettings.begin(),
            pSettings->PresetSettings.end(),
            [&](const Settings::PresetSetting& preset) {
                return preset.Uuid == value.Uuid;
            }),
        pSettings->PresetSettings.end()
    );
    // clang-format on

    pSettings->PresetSettings.push_back(value);
}

void Configuration::ClearPresets()
{
    pSettings->PresetSettings.clear();
}

std::string Configuration::BuildFullDatabaseFilePath() const
{
    auto result = std::filesystem::path(GetDatabasePath()) / GetDatabaseFileName();
    return result.string();
}

std::string Configuration::BuildFullBackupFilePath() const
{
    auto result = std::filesystem::path(GetBackupPath()) / GetDatabaseFileName();
    return result.string();
}

ConfigResult Configuration::WriteTomlContentsToFile(const std::string& fileContents)
{
    const std::string configFilePath = pEnv->GetConfigurationFilePath().string();

    SPDLOG_LOGGER_TRACE(pLogger, "Looking for configuration file at path \"{0}\"", configFilePath);

    if (!std::filesystem::exists(pEnv->GetConfigurationFilePath())) {
        return ConfigResult::Fail(Messages::ConfigurationFileNotFoundHeaderMessage,
            fmt::format(Messages::ConfigurationFileNotExistUserMessage, configFilePath),
            fmt::format(Messages::ConfigurationFileNotExistErrorMessage, configFilePath));
    }

    std::ofstream configFileStream;
    // Set exceptions to be thrown on failure
    configFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        configFileStream.open(configFilePath, std::ios::out);
    } catch (const std::system_error& e) {
        pLogger->error("Failed to open configuration file at path \"{0}\"", configFilePath);
        return ConfigResult::Fail(Messages::ConfigurationFileOpenHeaderMessage,
            Messages::ConfigurationFileOpenUserMessage,
            fmt::format(Messages::ConfigurationFileOpenErrorMessage, e.code().message()));
    }

    if (!configFileStream.is_open()) {
        pLogger->error("Should not get to this point when opening configuration file: \"{0}\"",
            configFilePath);
    }

    configFileStream << fileContents;

    configFileStream.close();
    return ConfigResult::OK();
}

void Configuration::GetGeneralConfig(const toml::value& root)
{
    if (!root.contains(Sections::GeneralSection)) {
        return;
    }

    const auto& generalSection = toml::find(root, Sections::GeneralSection);

    pSettings->UserInterfaceLanguage = toml::find_or<std::string>(generalSection, "lang", "en-US");

    pSettings->StartOnBoot = toml::find_or<bool>(generalSection, "startOnBoot", false);

    auto tomlStartPosition =
        toml::find_or<int>(generalSection, "startPosition", static_cast<int>(WindowState::Normal));
    pSettings->StartPosition = static_cast<WindowState>(tomlStartPosition);

    pSettings->ShowInTray = toml::find_or<bool>(generalSection, "showInTray", false);

    pSettings->MinimizeToTray = toml::find_or<bool>(generalSection, "minimizeToTray", false);

    pSettings->CloseToTray = toml::find_or<bool>(generalSection, "closeToTray", false);
}

void Configuration::GetDatabaseConfig(const toml::value& root)
{
    if (!root.contains(Sections::DatabaseSection)) {
        return;
    }

    const auto& databaseSection = toml::find(root, Sections::DatabaseSection);

    pSettings->DatabaseFileName = toml::find_or<std::string>(
        databaseSection, "databaseFileName", pEnv->GetDatabaseFileName());
    if (pSettings->DatabaseFileName.empty()) {
        pSettings->DatabaseFileName = pEnv->GetDatabaseFileName();
    }

    pSettings->DatabasePath = toml::find_or<std::string>(
        databaseSection, "databasePath", pEnv->GetDatabasePath().string());

    pSettings->BackupDatabase = toml::find_or<bool>(databaseSection, "backupDatabase", false);

    pSettings->BackupPath = toml::find_or<std::string>(databaseSection, "backupPath", "");

    pSettings->BackupOnProgramClose =
        toml::find_or<bool>(databaseSection, "backupOnProgramClose", false);

    pSettings->ZipBackupFile = toml::find_or<bool>(databaseSection, "zipBackupFile", false);
}

void Configuration::GetTasksConfig(const toml::value& root)
{
    if (!root.contains(Sections::TaskSection)) {
        return;
    }

    const auto& taskSection = toml::find(root, Sections::TaskSection);

    pSettings->TaskMinutesIncrement = toml::find_or<int>(taskSection, "minutesIncrement", 15);

    pSettings->MaximumDescriptionLength =
        toml::find_or<int>(taskSection, "maximumDescriptionLength", 3000);

    pSettings->ShowProjectAssociatedCategories =
        toml::find_or<bool>(taskSection, "showProjectAssociatedCategories", false);

    pSettings->UseReminders = toml::find_or<bool>(taskSection, "useReminders", false);

    pSettings->UseNotificationBanners =
        toml::find_or<bool>(taskSection, "useNotificationBanners", false);

    pSettings->OpenTaskDialogOnReminderClick =
        toml::find_or<bool>(taskSection, "openTaskDialogOnReminderClick", false);

    pSettings->UseTaskbarFlashing = toml::find_or<bool>(taskSection, "useTaskbarFlashing", false);

    pSettings->ReminderInterval = toml::find_or<int>(taskSection, "reminderInterval", 0);

    pSettings->OpenTaskDialogOnOutlookMeetingAttendanceCheck =
        toml::find_or<bool>(taskSection, "openTaskDialogOnOutlookMeetingAttendanceCheck", true);
}

void Configuration::GetTasksViewConfig(const toml::value& root)
{
    if (!root.contains(Sections::TasksViewSection)) {
        return;
    }

    const auto& tasksViewSection = toml::find(root, Sections::TasksViewSection);

    bool tasksViewColumnParsingFailed = false;
    if (tasksViewSection.contains("tasksViewColumns")) {
        const auto& tasksViewArrayTable = toml::find(tasksViewSection, "tasksViewColumns");
        try {
            if (tasksViewArrayTable.is_array()) {
                if (tasksViewArrayTable.size() == 0) {
                    std::vector<Settings::TasksViewColumnSetting> tasksViewColumnSettings =
                        Settings::MakeDefaultTasksViewColumnList();
                } else {
                    std::vector<Settings::TasksViewColumnSetting> columns;
                    for (size_t i = 0; i < tasksViewArrayTable.size(); i++) {
                        Settings::TasksViewColumnSetting column;

                        column.Name = toml::find<std::string>(tasksViewArrayTable[i], "name");
                        column.DisplayName =
                            toml::find<std::string>(tasksViewArrayTable[i], "displayName");
                        column.Order = toml::find<int>(tasksViewArrayTable[i], "order");
                        column.TextAlignment = static_cast<TasksViewColumnTextAlignment>(
                            toml::find<int>(tasksViewArrayTable[i], "textAlignment"));
                        column.EllipsisMode = static_cast<TasksViewColumnEllipsisMode>(
                            toml::find<int>(tasksViewArrayTable[i], "ellipsisMode"));
                        column.Type = static_cast<TasksViewColumnType>(
                            toml::find<int>(tasksViewArrayTable[i], "type"));
                        column.Width = toml::find<int>(tasksViewArrayTable[i], "width");
                        column.Selected = toml::find<bool>(tasksViewArrayTable[i], "selected");
                        column.TaskViewColumnId = static_cast<TasksViewColumnIdentifier>(
                            toml::find<int>(tasksViewArrayTable[i], "id"));

                        columns.push_back(column);
                    }

                    // clang-format off
                    std::sort(
                        columns.begin(),
                        columns.end(),
                        [](
                            const Settings::TasksViewColumnSetting& lhs,
                            const Settings::TasksViewColumnSetting& rhs
                        ) {
                            return lhs.Order < rhs.Order;
                        }
                    );
                    // clang-format on

                    pSettings->TasksViewColumnSettings = columns;
                }
            } else {
                tasksViewColumnParsingFailed = true;
            }
        } catch (const std::out_of_range& error) {
            pLogger->error("Error - {0}", error.what());
            tasksViewColumnParsingFailed = true;
        } catch (const toml::type_error& error) {
            pLogger->error("Error - {0}", error.what());
            tasksViewColumnParsingFailed = true;
        }
    } else {
        tasksViewColumnParsingFailed = true;
    }

    if (tasksViewColumnParsingFailed) {
        pLogger->warn("Tasks view column parsing failed, reset to default columns");
        pSettings->TasksViewColumnSettings = Settings::MakeDefaultTasksViewColumnList();
    }
}

void Configuration::GetExportConfig(const toml::value& root)
{
    if (!root.contains(Sections::ExportSection)) {
        return;
    }

    const auto& exportSection = toml::find(root, Sections::ExportSection);

    pSettings->ExportPath =
        toml::find_or<std::string>(exportSection, "exportPath", pEnv->GetExportPath().string());

    pSettings->CloseExportDialogAfterExporting =
        toml::find_or<bool>(exportSection, "closeExportDialogAfterExporting", false);
    pSettings->PresetCount = toml::find_or<int>(exportSection, "presetCount", 0);
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
        Settings::PresetSetting preset;

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
                    Settings::PresetColumnSetting presetColumn;
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
            [](
                const Settings::PresetColumnSetting& lhs,
                const Settings::PresetColumnSetting& rhs
            ) {
                return lhs.Order < rhs.Order;
            }
        );
        // clang-format on

        pSettings->PresetSettings.push_back(preset);
    }

    if (pSettings->PresetCount == 0 && pSettings->PresetSettings.size() > 0) {
        pSettings->PresetCount = static_cast<int>(pSettings->PresetSettings.size());
    }
}
} // namespace tks::Core
