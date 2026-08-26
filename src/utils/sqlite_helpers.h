#pragma once

#include <optional>
#include <string>
#include <sqlite3.h>

namespace tks::Utils::Sqlite
{
    // Returns std::nullopt when the column is NULL, otherwise the string contents.
    std::optional<std::string> GetOptionalText(sqlite3_stmt* stmt, int columnIndex) noexcept;

    // Convenience: returns the column text or an empty string if NULL.
    inline std::string GetTextOrEmpty(sqlite3_stmt* stmt, int columnIndex) noexcept
    {
        auto opt = GetOptionalText(stmt, columnIndex);
        return opt.has_value() ? std::move(opt.value()) : std::string();
    }
} // namespace tks::Utils::Sqlite
