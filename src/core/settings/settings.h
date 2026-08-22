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

#include <string>
#include <vector>

#include "presetsetting.h"
#include "tasksviewcolumnsetting.h"

#include "../../common/enums.h"

namespace tks::Core::Settings
{
struct Settings {
    std::string UserInterfaceLanguage;
    bool StartOnBoot;
    WindowState StartPosition;
    bool ShowInTray;
    bool MinimizeToTray;
    bool CloseToTray;

    std::string DatabaseFileName;
    std::string DatabasePath;
    bool BackupDatabase;
    std::string BackupPath;
    bool BackupOnProgramClose;
    bool ZipBackupFile;

    int TaskMinutesIncrement;
    int MaximumDescriptionLength;
    bool ShowProjectAssociatedCategories;
    bool UseReminders;
    bool UseNotificationBanners;
    bool UseTaskbarFlashing;
    int ReminderInterval;
    bool OpenTaskDialogOnReminderClick;
    bool OpenTaskDialogOnOutlookMeetingAttendanceCheck;

    bool UseProjectDisplayName;
    std::vector<TasksViewColumnSetting> TasksViewColumnSettings;

    std::string ExportPath;
    bool CloseExportDialogAfterExporting;
    int PresetCount;

    std::vector<PresetSetting> PresetSettings;
};
} // namespace MyNamespace
