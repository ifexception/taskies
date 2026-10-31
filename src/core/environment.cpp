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

#include "environment.h"

#include <filesystem>
#include <fstream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include <wx/stdpaths.h>

#include "../utils/utils.h"

namespace tks::Core
{
Environment::Environment() noexcept
    : mBuildConfig(BuildConfiguration::Undefined)
    , mInstallLocation(InstallLocation::Undefined)
{
#ifdef TKS_DEBUG
    mBuildConfig = BuildConfiguration::Debug;
    mInstallLocation = InstallLocation::Debug;
#else
    mBuildConfig = BuildConfiguration::Release;
#ifdef TKS_PORTABLEREL
    mInstallLocation = InstallLocation::Portable;
#else
    mInstallLocation = InstallLocation::ProgramFiles;
#endif // TKS_PORTABLEREL
#endif // TKS_DEBUG
}

BuildConfiguration Environment::GetBuildConfiguration() const noexcept
{
    return mBuildConfig;
}

InstallLocation Environment::GetInstallLocation() const noexcept
{
    return mInstallLocation;
}

std::string Environment::GetProgramPath() const
{
    return GetApplicationLogPath().string();
}

std::string Environment::GetLogFilePath() const
{
    auto path = GetApplicationLogPath() / LOG_FILENAME;
    return path.string();
}

std::string Environment::GetLanguagesPath() const
{
    return GetApplicationLanguagesPath().string();
}

std::string Environment::GetConfigurationFilePath() const
{
    auto path = GetApplicationConfigurationPath() / CFG_FILENAME;
    return path.string();
}

std::string Environment::GetDatabaseFilePath() const
{
    auto path = GetApplicationDatabasePath() / DATABASE_FILENAME;
    return path.string();
}

std::string Environment::GetDatabasePath() const
{
    return GetApplicationDatabasePath().string();
}

std::string Environment::GetResourcesPath() const
{
    return GetApplicationResourcesPath().string();
}

std::string Environment::GetExportPath() const
{
    return GetApplicationExportPath().string();
}

std::string Environment::GetCurrentLocale()
{
#ifdef _WIN32
    LCID lcid = GetThreadLocale();
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int res = LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0);
    if (res == 0) {
        return "en-US";
    }

    return Utils::ToStdString(std::wstring(name));
#else
    // Safe fallback prevents UB on non-Windows platforms
    // Not a concern for now
    if (const char* envLang = std::getenv("LANG")) {
        return std::string(envLang);
    }
    return "en-US";
#endif
}

// Private methods from here
std::filesystem::path Environment::GetApplicationPath() const
{
    return wxPathOnly(wxStandardPaths::Get().GetExecutablePath()).ToStdString();
}

std::filesystem::path Environment::GetApplicationLogPath() const
{
    std::filesystem::path appLogPath;
    const std::string logs = "logs";

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appLogPath = GetApplicationPath() / logs;
        break;
    case BuildConfiguration::Release:
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            appLogPath = std::filesystem::path(GetApplicationPath()) / logs;
            break;
        case InstallLocation::ProgramFiles:
            appLogPath =
                std::filesystem::path(wxStandardPaths::Get().GetUserDataDir().ToStdString()) / logs;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    {
        // spdlog only creates the file, the directory needs to be handled by us
        std::filesystem::create_directories(appLogPath);
    }

    return appLogPath;
}

std::filesystem::path Environment::GetApplicationDatabasePath() const
{
    std::filesystem::path appDataPath;
    const std::string data = "data";

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appDataPath = GetApplicationPath() / data;
        break;
    case BuildConfiguration::Release: {
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            appDataPath = GetApplicationPath() / data;
            break;
        case InstallLocation::ProgramFiles:
            appDataPath =
                std::filesystem::path(wxStandardPaths::Get().GetUserDataDir().ToStdString()) / data;
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    std::filesystem::create_directories(appDataPath);
    return appDataPath;
}

std::filesystem::path Environment::GetApplicationLanguagesPath() const
{
    std::filesystem::path appLangPath;
    const std::string lang = "lang";

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appLangPath = GetApplicationPath() / lang;
        break;
    case BuildConfiguration::Release:
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            appLangPath = std::filesystem::path(GetApplicationPath()) / lang;
            break;
        case InstallLocation::ProgramFiles:
            appLangPath =
                std::filesystem::path(wxStandardPaths::Get().GetResourcesDir().ToStdString()) /
                lang;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return appLangPath;
}

std::filesystem::path Environment::GetApplicationConfigurationPath() const
{
    std::filesystem::path appConfigPath;

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        appConfigPath = GetApplicationPath();
        break;
    case BuildConfiguration::Release:
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            appConfigPath = std::filesystem::path(GetApplicationPath());
            break;
        case InstallLocation::ProgramFiles:
            appConfigPath =
                std::filesystem::path(wxStandardPaths::Get().GetUserDataDir().ToStdString());
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return appConfigPath;
}

std::filesystem::path Environment::GetApplicationResourcesPath() const
{
    std::filesystem::path appResPath;
    const std::string res = "res";

    switch (mBuildConfig) {
    case tks::BuildConfiguration::Debug:
        appResPath = GetApplicationPath() / res;
        break;
    case tks::BuildConfiguration::Release:
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            appResPath = std::filesystem::path(GetApplicationPath()) / res;
            break;
        case InstallLocation::ProgramFiles:
            appResPath =
                std::filesystem::path(wxStandardPaths::Get().GetResourcesDir().ToStdString()) / res;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return appResPath;
}

std::filesystem::path Environment::GetApplicationExportPath() const
{
    std::filesystem::path exportPath;
    const std::string defaultExportPath = "exports";

    switch (mBuildConfig) {
    case BuildConfiguration::Debug:
        exportPath = GetApplicationPath();
        break;
    case BuildConfiguration::Release: {
        switch (mInstallLocation) {
        case InstallLocation::Undefined:
            [[fallthrough]];
        case InstallLocation::Portable:
            exportPath = std::filesystem::path(GetApplicationPath()) / defaultExportPath;
            break;
        case InstallLocation::ProgramFiles:
            exportPath =
                std::filesystem::path(wxStandardPaths::Get().GetAppDocumentsDir().ToStdString());
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    std::filesystem::create_directories(exportPath);

    return exportPath;
}
} // namespace tks::Core
