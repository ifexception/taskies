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

namespace tks::Model
{
struct AttributeGroupModel {
    AttributeGroupModel();
    ~AttributeGroupModel() = default;

    std::int64_t AttributeGroupId;
    std::string Name;
    std::optional<std::string> Description;
    std::uint32_t DateCreated;
    std::uint32_t DateModified;
    bool IsActive;

    const std::string GetDateCreatedString() const;
    const std::string GetDateModifiedString() const;
};
} // namespace tks::Model
