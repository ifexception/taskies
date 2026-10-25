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

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <toml.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "settings/tasksviewcolumnsetting.h"
#include "settings/presetsetting.h"
#include "settings/settings.h"

#include "../common/enums.h"

#include "../common/results/configresult.h"

namespace tks::Core
{
class Environment;

struct Sections {
    static constexpr std::string_view GeneralSection = "general";
    static constexpr std::string_view DatabaseSection = "database";
    static constexpr std::string_view TaskSection = "tasks";
    static constexpr std::string_view TasksViewSection = "tasksView";
    static constexpr std::string_view ExportSection = "export";
    static constexpr std::string_view PresetsSection = "presets";
};

class Configuration
{
public:
    Configuration(
        std::shared_ptr<spdlog::logger> logger,
        std::shared_ptr<Environment> env);
    ~Configuration() = default;

    ConfigResult LoadConfigurationFile();
    ConfigResult RecreateConfigurationFile();

    ConfigResult Save();
    ConfigResult RestoreDefaults();

    ConfigResult SaveExportPreset(Settings::PresetSetting presetToSave);
    ConfigResult UpdateExportPreset(Settings::PresetSetting presetToUpdate);
    ConfigResult TryUnsetDefaultPreset();

    std::string GetUserInterfaceLanguage() const;
    void SetUserInterfaceLanguage(std::string_view value);

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

    bool ShowOutlookMeetingsPanel() const;
    void ShowOutlookMeetingsPanel(const bool value);

    std::string GetDatabaseFileName() const;
    void SetDatabaseFileName(std::string_view value);

    std::string GetDatabasePath() const;
    void SetDatabasePath(std::string_view value);

    bool BackupDatabase() const;
    void BackupDatabase(const bool value);

    std::string GetBackupPath() const;
    void SetBackupPath(std::string_view value);

    bool BackupOnProgramClose() const;
    void BackupOnProgramClose(const bool value);

    bool ZipBackupFile() const;
    void ZipBackupFile(const bool value);

    int GetMinutesIncrement() const;
    void SetMinutesIncrement(const int value);

    int GetMaximumDescriptionLength() const;
    void SetMaximumDescriptionLength(const int value);

    bool ShowProjectAssociatedCategories() const;
    void ShowProjectAssociatedCategories(const bool value);

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

    std::vector<Settings::TasksViewColumnSetting> GetTasksViewColumns() const;
    void SetTasksViewColumns(const std::vector<Settings::TasksViewColumnSetting> values);

    std::string GetExportPath() const;
    void SetExportPath(std::string_view value);

    bool CloseExportDialogAfterExporting() const;
    void CloseExportDialogAfterExporting(const bool value);

    int GetPresetCount() const;
    void SetPresetCount(const int value);

    std::vector<Settings::PresetSetting> GetPresets() const;
    void SetPresets(const std::vector<Settings::PresetSetting>& values);
    void AddPreset(const Settings::PresetSetting& value);
    void EmplacePreset(const Settings::PresetSetting& value);
    void ClearPresets();


    std::string BuildFullDatabaseFilePath() const;
    std::string BuildFullBackupFilePath() const;

private:
    ConfigResult WriteTomlContentsToFile(const std::string& fileContents);

    void GetGeneralConfig(const toml::value& root);
    void GetDatabaseConfig(const toml::value& root);
    void GetTasksConfig(const toml::value& root);
    void GetTasksViewConfig(const toml::value& root);
    void GetExportConfig(const toml::value& root);
    void GetPresetsConfig(const toml::value& root);

    std::unique_ptr<Settings::Settings> pSettings;

    std::shared_ptr<spdlog::logger> pLogger;
    std::shared_ptr<Environment> pEnv;
};
} // namespace tks::Core
