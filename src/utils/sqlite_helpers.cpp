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

#include "sqlite_helpers.h"

namespace tks::Utils::Sqlite
{
std::optional<std::string> GetOptionalText(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    if (!stmt) {
        return std::nullopt;
    }

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }

    const unsigned char* text = sqlite3_column_text(stmt, columnIndex);
    if (text == nullptr) {
        return std::nullopt;
    }

    int length = sqlite3_column_bytes(stmt, columnIndex);
    return std::make_optional(std::string(reinterpret_cast<const char*>(text), length));
}

std::optional<int> GetOptionalInt(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    if (!stmt) {
        return std::nullopt;
    }

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }

    return std::make_optional(static_cast<int>(sqlite3_column_int(stmt, columnIndex)));
}

std::optional<std::int64_t> GetOptionalInt64(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    if (!stmt) {
        return std::nullopt;
    }

    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }

    return std::make_optional(static_cast<std::int64_t>(sqlite3_column_int64(stmt, columnIndex)));
}
} // namespace tks::Utils::Sqlite
