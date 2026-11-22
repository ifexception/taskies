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

#include "setup_manager.h"

#include <fstream>
#include <filesystem>

#include <toml.hpp>

namespace tks::Core
{
SetupManager::SetupManager(std::shared_ptr<Environment> env) noexcept
    : pEnv(std::move(env))
    , mDebugSetupReleaseConfiguration(DebugSetupReleaseConfiguration::Default)
    , bIsDebugAndPortable(false)
    , bIsReleaseAndPortable(false)
{
    bIsDebugAndPortable =
        pEnv->GetBuildConfiguration() == BuildConfiguration::Debug &&
        mDebugSetupReleaseConfiguration == DebugSetupReleaseConfiguration::Portable;

    bIsReleaseAndPortable = pEnv->GetBuildConfiguration() == BuildConfiguration::Release &&
                            pEnv->GetInstallLocation() == InstallLocation::Portable;
}

void SetupManager::SetDebugSetupReleaseConfiguration(
    const DebugSetupReleaseConfiguration setupReleaseConfiguration)
{
    mDebugSetupReleaseConfiguration = setupReleaseConfiguration;
}

bool SetupManager::IsSetup() const
{
    if (!std::filesystem::exists(pEnv->GetConfigurationFilePath())) {
        return false;
    }

    toml::value root = toml::parse(pEnv->GetConfigurationFilePath());
    bool isSetup = false;
    try {
        const auto& setupSection = toml::find(root, "setup");

        isSetup = toml::find_or<bool>(setupSection, "is_setup", false);
    } catch (const toml::syntax_error&) {
        /*pLogger->error("A TOML syntax/parse error occurred when parsing configuration file
           \"{0}\"", error.what());*/

        return false;
    } catch (const toml::type_error&) {
        /*pLogger->error(
            "A TOML type error occurred when reading configuration values - {0}", error.what());*/

        return false;
    } catch (const std::out_of_range&) {
        /*pLogger->error(
            "A (TOML) out-of-range error occurred when reading configuration values - {0}",
            error.what());*/
        return false;
    }

    if (bIsDebugAndPortable || bIsReleaseAndPortable) {
        return std::filesystem::exists(pEnv->GetDatabaseFilePath()) &&
               std::filesystem::exists(pEnv->GetConfigurationFilePath()) && isSetup;
    }

#ifdef _WIN32
    std::string registryKey = GetRegistryKey();
    if (registryKey.empty()) {
        return false;
    }

    std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
    std::wstring wRegistryValue = L"IsSetup";

    RegKeyGuard guard;
    LSTATUS status =
        ::RegOpenKeyExW(HKEY_CURRENT_USER, wRegistryKey.c_str(), 0, KEY_READ, &guard.key);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    uint32_t data = 0;
    DWORD dataSize = sizeof(data);

    status = ::RegGetValueW(
        guard.key, nullptr, wRegistryValue.c_str(), RRF_RT_REG_DWORD, nullptr, &data, &dataSize);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    return true;
#else
    // Non-Windows fallback stub
    return std::filesystem::exists(std::string(SETUP_FILENAME));
#endif // _WIN32
}

bool SetupManager::SetIsSetup()
{
    if (bIsDebugAndPortable || bIsReleaseAndPortable) {
        toml::value root = toml::parse(pEnv->GetConfigurationFilePath());
        root.at(std::string("setup"))["is_setup"] = true;

        const std::string tomlFormattedData = toml::format(root);

        if (!std::filesystem::exists(pEnv->GetConfigurationFilePath())) {
            return false;
        }

        std::ofstream configFileStream;
        configFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try {
            configFileStream.open(pEnv->GetConfigurationFilePath());
        } catch (const std::system_error&) {
            return false;
        }

        if (!configFileStream.is_open()) {
            return false;
        }

        configFileStream << tomlFormattedData;
        configFileStream.close();

        return true;
    }

#ifdef _WIN32
    const std::string registryKey = GetRegistryKey();
    if (registryKey.empty()) {
        return false;
    }

    std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
    std::wstring wRegistryValue = L"IsSetup";

    DWORD value = static_cast<DWORD>(true);

    RegKeyGuard guard;
    LSTATUS status = ::RegCreateKeyExW(HKEY_CURRENT_USER, // Handle to an open parent key
        wRegistryKey.c_str(), // Name of the subkey to open or create
        0, // Reserved - must be 0
        nullptr, // User-defined class type
        REG_OPTION_NON_VOLATILE, // Standard persistent key
        KEY_ALL_ACCESS, // Security access mask
        nullptr, // Security attributes inheriting handle
        &guard.key, // Output pointer to the opened/created key handle
        nullptr // Output variable receiving the disposition result; can be NULL
    );

    if (status != ERROR_SUCCESS) {
        return false;
    }

    status = ::RegSetValueExW(guard.key,
        wRegistryValue.c_str(),
        0, // Reserved; must be 0
        REG_DWORD, // Flag specifying a 32-bit integer
        reinterpret_cast<const BYTE*>(&value), // Pointer to the data byte buffer
        sizeof(value) // Size of data in bytes (always 4 for DWORD)
    );

    if (status != ERROR_SUCCESS) {
        return false;
    }
    return true;
#else
    // Non-Windows fallback stub
    std::ofstream setupFile(std::string(SETUP_FILENAME), std::ios::out);
    return setupFile.is_open();
#endif // _WIN32
}

// REFACTOR
bool SetupManager::ConfigurationFileMigrationRequired() const
{
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Debug) {
        if (mDebugSetupReleaseConfiguration == DebugSetupReleaseConfiguration::Portable) {
            auto cfgMigrateFile =
                std::filesystem::path(pEnv->GetProgramPath()) / std::string(CFG_MIGRATE_FILENAME);
            if (std::filesystem::exists(cfgMigrateFile)) {
                bool res = std::filesystem::remove(cfgMigrateFile);
                return res;
            }
        }
    }

    if (bIsReleaseAndPortable) {
        auto cfgMigrateFile =
            std::filesystem::path(pEnv->GetProgramPath()) / std::string(CFG_MIGRATE_FILENAME);
        if (std::filesystem::exists(cfgMigrateFile)) {
            bool res = std::filesystem::remove(cfgMigrateFile);
            return res;
        }
    }

#ifdef _WIN32
    if (!bIsReleaseAndPortable) {
        const std::string registryKey = GetRegistryKey();
        if (registryKey.empty()) {
            return false;
        }

        std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
        std::wstring wRegistryValue = L"ConfigMigrationRequired";

        RegKeyGuard guard;
        LSTATUS status =
            ::RegOpenKeyExW(HKEY_CURRENT_USER, wRegistryKey.c_str(), 0, KEY_READ, &guard.key);

        if (status != ERROR_SUCCESS) {
            return false;
        }

        uint32_t data = 0;
        DWORD dataSize = sizeof(data);

        status = ::RegGetValueW(guard.key,
            nullptr,
            wRegistryValue.c_str(),
            RRF_RT_REG_DWORD,
            nullptr,
            &data,
            &dataSize);

        if (status != ERROR_SUCCESS) {
            return false;
        }

        return !!data;
    }

    return false;

#else
    // Non-Windows fallback stub
    return false;
#endif // _WIN32
}

// REFACTOR
bool SetupManager::RemoveConfigurationFileMigrationRegistryKey()
{
#ifdef _WIN32
    const std::string registryKey = GetRegistryKey();
    if (registryKey.empty()) {
        return false;
    }

    std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
    std::wstring wRegistryValue = L"ConfigMigrationRequired";

    RegKeyGuard guard;
    LSTATUS status =
        ::RegOpenKeyExW(HKEY_CURRENT_USER, wRegistryKey.c_str(), 0, KEY_READ, &guard.key);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    status = ::RegDeleteKeyEx(guard.key, wRegistryValue.c_str(), KEY_WOW64_64KEY, 0);

    if (status != ERROR_SUCCESS) {
        return false;
    }
    return true;
#else
    // Non-Windows fallback stub
    return false;
#endif //
}

std::string SetupManager::GetRegistryKey() const
{
    switch (pEnv->GetBuildConfiguration()) {
    case BuildConfiguration::Debug:
        return "Software\\Taskiesd";
    case BuildConfiguration::Release:
        return "Software\\Taskies";
    default:
        return "";
    }
}
} // namespace tks::Core
