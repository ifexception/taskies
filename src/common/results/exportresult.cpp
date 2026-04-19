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

#include "exportresult.h"

namespace tks
{
DatabaseResult DatabaseResult::MakeFromSqliteResult(const SqliteResult& sqliteResult)
{
    return DatabaseResult{
        sqliteResult.FriendlyErrorMessage, sqliteResult.ReturnCode, sqliteResult.ErrorMessage
    };
}

ExportResult ExportResult::OK()
{
    return ExportResult{ true, "" };
}

ExportResult ExportResult::Fail(const std::string& errorMessage)
{
    return ExportResult{ false, errorMessage };
}

ExportResult ExportResult::FailWithSqliteResult(const std::string& errorMessage,
    const SqliteResult& sqliteResult)
{
    return ExportResult{ false, errorMessage, DatabaseResult::MakeFromSqliteResult(sqliteResult) };
}
} // namespace tks
