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

#include <optional>
#include <string>
#include <sqlite3.h>

namespace tks::Utils::Sqlite
{
// Returns std::nullopt when the column is NULL, otherwise the string contents
std::optional<std::string> GetOptionalText(sqlite3_stmt* stmt, int columnIndex) noexcept;

// Returns the column text or an empty string if NULL
inline std::string GetTextOrEmpty(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    auto optionalValue = GetOptionalText(stmt, columnIndex);
    return optionalValue.has_value() ? std::move(optionalValue.value()) : std::string();
}

// Returns std::nullopt when the column is NULL, otherwise the int value
std::optional<int> GetOptionalInt(sqlite3_stmt* stmt, int columnIndex) noexcept;

// Returns std::nullopt when the column is NULL, otherwise the int64 value
std::optional<std::int64_t> GetOptionalInt64(sqlite3_stmt* stmt, int columnIndex) noexcept;

inline int GetIntOrZero(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    auto optionalValue = GetOptionalInt(stmt, columnIndex);
    return optionalValue.has_value() ? optionalValue.value() : 0;
}

inline std::int64_t GetInt64OrZero(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    auto optionalValue = GetOptionalInt64(stmt, columnIndex);
    return optionalValue.has_value() ? optionalValue.value() : 0;
}
} // namespace tks::Utils::Sqlite
