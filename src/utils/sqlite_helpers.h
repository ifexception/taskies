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
} // namespace tks::Utils::Sqlite
