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

// Convenience wrappers returning zero when NULL
inline int GetIntOrZero(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    auto optionalValue = GetOptionalInt(stmt, columnIndex);
    return optionalValue.has_value() ? optionalValue.value() : 0;
}

// Convenience wrappers returning zero when NULL
inline std::int64_t GetInt64OrZero(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    auto optionalValue = GetOptionalInt64(stmt, columnIndex);
    return optionalValue.has_value() ? optionalValue.value() : 0;
}
} // namespace tks::Utils::Sqlite
