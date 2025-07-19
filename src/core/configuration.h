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

#pragma once

#include <memory>
#include <string>

#include <toml.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "../common/common.h"
#include "../common/enums.h"

namespace tks::Core
{
class Environment;

class Configuration
{
public:
    struct PresetColumnSettings {
        std::string Column;
        std::string OriginalColumn;
        int Order;

        PresetColumnSettings()
            : Column()
            , OriginalColumn()
            , Order(-1)
        {
        }
        PresetColumnSettings(Common::PresetColumn presetColumn);
        ~PresetColumnSettings() = default;
    };

    struct PresetSettings {
        std::string Uuid;
        std::string Name;
        bool IsDefault;
        DelimiterType Delimiter;
        TextQualifierType TextQualifier;
        EmptyValues EmptyValuesHandler;
        NewLines NewLinesHandler;
        BooleanHandler BooleanHandler;
        bool ExcludeHeaders;
        bool IncludeAttributes;
        std::vector<PresetColumnSettings> Columns;

        PresetSettings()
            : Uuid()
            , Name()
            , IsDefault(false)
            , Delimiter(DelimiterType::None)
            , TextQualifier(TextQualifierType::None)
            , EmptyValuesHandler(EmptyValues::None)
            , NewLinesHandler(NewLines::None)
            , BooleanHandler(BooleanHandler::None)
            , ExcludeHeaders(false)
            , IncludeAttributes(false)
            , Columns()
        {
        }
        PresetSettings(Common::Preset preset);
        ~PresetSettings() = default;
    };

    Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~Configuration() = default;

    bool Load();

    bool Save();
    bool RestoreDefaults();

    bool SaveExportPreset(const Common::Preset& presetToSave);
    bool UpdateExportPreset(const Common::Preset& presetToUpdate);
    bool TryUnsetDefaultPreset();

    std::string GetUserInterfaceLanguage() const;
    void SetUserInterfaceLanguage(const std::string& value);

    bool StartOnBoot() const;
    void StartOnBoot(const bool value);

    WindowState GetWindowState() const;
    void SetWindowState(const WindowState value);

    bool ShowInTray() const;
    void ShowInTray(const bool value);

    bool MinimizeToTray() const;
    void MinimizeToTray(const bool value);

    bool CloseToTray() const;
    void CloseToTray(const bool value);

    std::string GetDatabasePath() const;
    void SetDatabasePath(const std::string& value);

    bool BackupDatabase() const;
    void BackupDatabase(const bool value);

    std::string GetBackupPath() const;
    void SetBackupPath(const std::string& value);

    int GetMinutesIncrement() const;
    void SetMinutesIncrement(const int value);

    bool ShowProjectAssociatedCategories() const;
    void ShowProjectAssociatedCategories(const bool value);

    bool UseLegacyTaskDialog() const;
    void UseLegacyTaskDialog(const bool value);

    bool UseReminders() const;
    void UseReminders(const bool value);

    bool UseNotificationBanners() const;
    void UseNotificationBanners(const bool value);

    bool UseTaskbarFlashing() const;
    void UseTaskbarFlashing(const bool value);

    int ReminderInterval() const;
    void SetReminderInterval(const int value);

    bool OpenTaskDialogOnReminderClick() const;
    void OpenTaskDialogOnReminderClick(const bool value);

    bool TodayAlwaysExpanded() const;
    void TodayAlwaysExpanded(const bool value);

    std::string GetExportPath() const;
    void SetExportPath(const std::string& value);

    bool CloseExportDialogAfterExporting() const;
    void CloseExportDialogAfterExporting(const bool value);

    int GetPresetCount() const;
    void SetPresetCount(const int value);

    std::vector<PresetSettings> GetPresets() const;
    void SetPresets(const std::vector<PresetSettings>& values);
    void SetPreset(const PresetSettings& value);
    void EmplacePreset(const PresetSettings& value);
    void ClearPresets();

private:
    bool WriteTomlContentsToFile(const std::string& fileContents);

    void GetGeneralConfig(const toml::value& root);
    void GetDatabaseConfig(const toml::value& root);
    void GetTasksConfig(const toml::value& root);
    void GetTasksViewConfig(const toml::value& root);
    void GetExportConfig(const toml::value& root);
    void GetPresetsConfig(const toml::value& root);
    void GetPresetsConfigEx(const toml::value& root);

    struct Sections {
        static const std::string GeneralSection;
        static const std::string DatabaseSection;
        static const std::string TaskSection;
        static const std::string TasksViewSection;
        static const std::string ExportSection;
        static const std::string PresetsSection;
    };

    struct Settings {
        std::string UserInterfaceLanguage;
        bool StartOnBoot;
        WindowState StartPosition;
        bool ShowInTray;
        bool MinimizeToTray;
        bool CloseToTray;

        std::string DatabasePath;
        bool BackupDatabase;
        std::string BackupPath;

        int TaskMinutesIncrement;
        bool ShowProjectAssociatedCategories;
        bool UseLegacyTaskDialog;
        bool UseReminders;
        bool UseNotificationBanners;
        bool UseTaskbarFlashing;
        int ReminderInterval;
        bool OpenTaskDialogOnReminderClick;

        bool TodayAlwaysExpanded;

        std::string ExportPath;
        bool CloseExportDialogAfterExporting;
        int PresetCount;

        std::vector<PresetSettings> PresetSettings;
    };

    Settings mSettings;

    std::shared_ptr<Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace tks::Core
