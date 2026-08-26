#include "sqlite_helpers.h"

namespace tks::Utils::Sqlite
{
std::optional<std::string> GetOptionalText(sqlite3_stmt* stmt, int columnIndex) noexcept
{
    if (!stmt) {
        return std::nullopt;
    }

    // If the column is explicitly NULL, return nullopt.
    if (sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL) {
        return std::nullopt;
    }

    const unsigned char* text = sqlite3_column_text(stmt, columnIndex);
    if (text == nullptr) {
        // Handle unexpected NULL pointer (shouldn't happen if column_type != NULL)
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
