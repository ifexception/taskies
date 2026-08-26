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

        const unsigned char* textPtr = sqlite3_column_text(stmt, columnIndex);
        if (textPtr == nullptr) {
            // Defensively handle unexpected NULL pointer (shouldn't happen if column_type != NULL).
            return std::nullopt;
        }

        int len = sqlite3_column_bytes(stmt, columnIndex);
        return std::make_optional(std::string(reinterpret_cast<const char*>(textPtr), len));
    }
} // namespace tks::Utils::Sqlite
