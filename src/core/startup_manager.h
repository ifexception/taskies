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

#include <functional>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

#include "environment.h"

namespace tks::Core
{
class StartUpManager final
{
public:
    explicit StartUpManager(std::reference_wrapper<std::shared_ptr<Environment>> env) noexcept;

    StartUpManager(const StartUpManager&) = delete;
    StartUpManager& operator=(const StartUpManager&) = delete;
    StartUpManager(StartUpManager&&) noexcept = default;
    StartUpManager& operator=(StartUpManager&&) noexcept = default;
    ~StartUpManager() = default;

    bool IsSetup() const;
    bool SetIsSetup();

private:
    std::string GetRegistryKey() const;

    std::reference_wrapper<std::shared_ptr<Environment>> pEnvRef;
};
} // namespace tks::Core
