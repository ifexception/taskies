// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include <cstdint>
#include <optional>
#include <string>

namespace tks::Services
{
struct CategoryViewModel {
    CategoryViewModel();

    std::int64_t CategoryId;
    std::string Name;
    unsigned int Color;
    bool Billable;
    std::optional<std::string> Description;
    std::int32_t DateCreated;
    std::int32_t DateModified;
    bool IsActive;
    std::optional<std::int64_t> ProjectId;

    std::optional<std::string> ProjectDisplayName;

    std::string GetFormattedName();

    const std::string GetDateCreatedString() const;
    const std::string GetDateModifiedString() const;
};
} // namespace tks::Services
