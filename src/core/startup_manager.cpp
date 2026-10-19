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

#include "startup_manager.h"

#include <fstream>
#include <filesystem>

namespace tks::Core
{
StartUpManager::StartUpManager(std::reference_wrapper<std::shared_ptr<Environment>> env) noexcept
    : pEnvRef(env)
{
}

bool StartUpManager::IsSetup() const
{
    if (pEnvRef.get()->GetBuildConfiguration() == BuildConfiguration::Release &&
        pEnvRef.get()->GetInstallLocation() == InstallLocation::Portable) {
        return std::filesystem::exists(std::string(SETUP_FILENAME));
    }

    std::string registryKey = GetRegistryKey();
    if (registryKey.empty()) {
        return false;
    }

    std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
    std::wstring wRegistryValue = L"IsSetup";

    HKEY hKey = nullptr;
    LSTATUS status = RegOpenKeyEx(HKEY_CURRENT_USER, wRegistryKey.c_str(), 0, KEY_READ, &hKey);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    status =
        RegGetValue(hKey, nullptr, wRegistryValue.c_str(), RRF_RT_ANY, nullptr, nullptr, nullptr);
    if (status != ERROR_SUCCESS || status != ERROR_MORE_DATA) {
        return false;
    }

    uint32_t data = 0;
    DWORD dataSize = sizeof(data);

    status = RegGetValueW(hKey,
        nullptr,
        wRegistryValue.c_str(),
        RRF_RT_REG_DWORD, // Restrict types to standard 32-bit DWORD
        nullptr,
        &data,
        &dataSize);

    if (status != ERROR_SUCCESS) {
        return false;
    }

    return true;
}

bool StartUpManager::SetIsSetup()
{
    if (pEnvRef.get()->GetBuildConfiguration() == BuildConfiguration::Release &&
        pEnvRef.get()->GetInstallLocation() == InstallLocation::Portable) {
        std::ofstream setupFile(std::string(SETUP_FILENAME), std::ios::out);
        if (setupFile.is_open()) {
            setupFile << true;
            return true;
        }
        return false;
    }

    const std::string registryKey = GetRegistryKey();
    if (registryKey.empty()) {
        return false;
    }

    std::wstring wRegistryKey(registryKey.begin(), registryKey.end());
    std::wstring wRegistryValue = L"IsSetup";

    DWORD disposition = 0;
    DWORD value = static_cast<DWORD>(true);
    HKEY hKey = nullptr;

    LSTATUS status =
        RegCreateKeyExW(HKEY_CURRENT_USER, // Handle to an open parent key (e.g., HKEY_CURRENT_USER)
            wRegistryKey.c_str(), // Name of the subkey to open or create
            0, // Reserved; must be 0
            nullptr, // User-defined class type; usually null
            REG_OPTION_NON_VOLATILE, // Standard persistent key (saved across reboots)
            KEY_ALL_ACCESS, // Security access mask (e.g., KEY_READ, KEY_WRITE, KEY_ALL_ACCESS)
            nullptr, // Security attributes inheriting handle; usually null
            &hKey, // Output pointer to the opened/created key handle
            &disposition // Output variable receiving the disposition result
        );

    if (status != ERROR_SUCCESS) {
        return false;
    }

    // Check disposition to see what happened under the hood
    if (disposition != REG_CREATED_NEW_KEY || disposition != REG_OPENED_EXISTING_KEY) {
        return false;
    }

    status = RegSetValueExW(hKey,
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
}

std::string StartUpManager::GetRegistryKey() const
{
    switch (pEnvRef.get()->GetBuildConfiguration()) {
    case BuildConfiguration::Debug:
        return "Software\\Taskiesd";
    case BuildConfiguration::Release:
        return "Software\\Taskies";
    default:
        return "";
    }
}
} // namespace tks::Core
