// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

namespace tks::Core
{
class Environment;

class Configuration
{
public:
    Configuration(std::shared_ptr<Environment> env, std::shared_ptr<spdlog::logger> logger);
    ~Configuration() = default;

    bool Load();

    void Save();

    std::string GetUserInterfaceLanguage();
    void SetUserInterfaceLanguage(const std::string& value);

    std::string GetDatabasePath() const;
    void SetDatabasePath(const std::string& value);

private:
    void GetGeneralConfig(const toml::value& config);
    void GetDatabaseConfig(const toml::value& config);

    struct Sections {
        static const std::string GeneralSection;
        static const std::string DatabaseSection;
    };

    struct Settings {
        std::string UserInterfaceLanguage;
        std::string DatabasePath;
    };

    Settings mSettings;
    std::shared_ptr<Environment> pEnv;
    std::shared_ptr<spdlog::logger> pLogger;
};
} // namespace tks::Core
