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

#pragma once

#include <memory>
#include <string>

#include <toml.hpp>
#include <spdlog/spdlog.h>

#include "../common/common.h"
#include "../common/enums.h"

namespace tks::Core
{
class Environment;

class Configuration
{
public:
    struct PresetSettings {
        std::string Name;
        bool IsDefault;
        std::string Delimiter;
        std::string TextQualifier;
        EmptyValues EmptyValuesHandler;
        NewLines NewLinesHandler;
        bool ExcludeHeaders;
        std::vector<std::string> Columns;
        std::vector<std::string> OriginalColumns;
    };

    Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~Configuration() = default;

    bool Load();

    bool Save();
    bool RestoreDefaults();

    bool SaveExportPreset(const Common::Preset& preset);

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

    int GetBackupRetentionPeriod() const;
    void SetBackupRetentionPeriod(const int value);

    int GetMinutesIncrement() const;
    void SetMinutesIncrement(const int value);

    bool ShowProjectAssociatedCategories() const;
    void ShowProjectAssociatedCategories(const bool value);

    bool TodayAlwaysExpanded() const;
    void TodayAlwaysExpanded(const bool value);

    std::string GetExportPath() const;
    void SetExportPath(const std::string& value);

    int GetPresetCount() const;
    void SetPresetCount(const int value);

    std::vector<PresetSettings> GetPresets() const;
    void SetPresets(const std::vector<PresetSettings>& values);
    void ClearPresets();

private:
    void GetGeneralConfig(const toml::value& root);
    void GetDatabaseConfig(const toml::value& root);
    void GetTasksConfig(const toml::value& root);
    void GetTasksViewConfig(const toml::value& root);
    void GetExportConfig(const toml::value& root);
    void GetPresetsConfig(const toml::value& root);
    void GetPresetsConfig2(const toml::value& root);

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
        int BackupRetentionPeriod;

        int TaskMinutesIncrement;
        bool ShowProjectAssociatedCategories;

        bool TodayAlwaysExpanded;

        std::string ExportPath;
        int PresetCount;

        std::vector<PresetSettings> PresetSettings;
    };

    Settings mSettings;

    std::shared_ptr<Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace tks::Core
