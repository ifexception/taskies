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

namespace tks::Core
{
SetupManager::SetupManager(std::shared_ptr<Environment> env) noexcept
    : pEnv(std::move(env))
{
}

bool SetupManager::IsSetup() const
{
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Release &&
        pEnv->GetInstallLocation() == InstallLocation::Portable) {
        return std::filesystem::exists(std::string(SETUP_FILENAME));
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
    if (pEnv->GetBuildConfiguration() == BuildConfiguration::Release &&
        pEnv->GetInstallLocation() == InstallLocation::Portable) {
        std::ofstream setupFile(std::string(SETUP_FILENAME), std::ios::out);
        if (setupFile.is_open()) {
            setupFile << true;
            return true;
        }
        return false;
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

bool SetupManager::ConfigurationFileMigrationRequired() const
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

    uint32_t data = 0;
    DWORD dataSize = sizeof(data);

    status = ::RegGetValueW(
        guard.key, nullptr, wRegistryValue.c_str(), RRF_RT_REG_DWORD, nullptr, &data, &dataSize);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    return !!data;
#else
    // Non-Windows fallback stub
    return false;
#endif // _WIN32
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
