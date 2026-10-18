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

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "../common/enums.h"

namespace tks::Core
{
inline constexpr std::string_view DATABASE_FILENAME = "taskies.db";
inline constexpr std::string_view SETUP_FILENAME = "taskies.setup";
inline constexpr std::string_view LOG_FILENAME = "taskies.log";
inline constexpr std::string_view CFG_FILENAME = "taskies.toml";

class Environment final
{
public:
    explicit Environment() noexcept;
    Environment(const Environment&) = delete;
    Environment& operator=(const Environment&) = delete;
    Environment(Environment&&) noexcept = default;
    Environment& operator=(Environment&&) noexcept = default;
    ~Environment() = default;

    BuildConfiguration GetBuildConfiguration() const noexcept;
    InstallLocation GetInstallLocation() const noexcept;

    std::string GetLogFilePath();
    std::string GetLanguagesPath();
    std::string GetConfigurationFilePath();
    std::string GetDatabaseFilePath();
    std::string GetDatabasePath();
    std::string GetResourcesPath();
    std::string GetExportPath();

    static std::string GetCurrentLocale();

    bool IsSetup();
    bool SetIsSetup();

private:
    std::filesystem::path GetApplicationPath();
    std::filesystem::path GetApplicationDatabasePath();
    std::filesystem::path GetApplicationLogPath();
    std::filesystem::path GetApplicationLanguagesPath();
    std::filesystem::path GetApplicationConfigurationPath();
    std::filesystem::path GetApplicationResourcesPath();
    std::filesystem::path GetApplicationExportPath();

    std::string GetRegistryKey();

    BuildConfiguration mBuildConfig;
    InstallLocation mInstallLocation;
};
} // namespace tks::Core
