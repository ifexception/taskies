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

#include "sqliteresult.h"

#include <fmt/format.h>

namespace tks
{
SqliteResult::SqliteResult()
    : Success(true)
    , FriendlyErrorMessage("")
    , ReturnCode(0)
    , ErrorMessage("")
    , ConditionCheckFailed(false)
{
}

SqliteResult::SqliteResult(const std::string& friendlyErrorMessage)
    : Success(true)
    , FriendlyErrorMessage(friendlyErrorMessage)
    , ReturnCode(0)
    , ErrorMessage("")
    , ConditionCheckFailed(true)
{
}

SqliteResult::SqliteResult(int returnCode, const std::string& errorMessage)
    : Success(false)
    , FriendlyErrorMessage("")
    , ReturnCode(returnCode)
    , ErrorMessage(errorMessage)
    , ConditionCheckFailed(false)
{
}

SqliteResult::SqliteResult(const std::string& friendlyErrorMessage,
    int returnCode,
    const std::string& errorMessage)
    : Success(false)
    , FriendlyErrorMessage(friendlyErrorMessage)
    , ReturnCode(returnCode)
    , ErrorMessage(errorMessage)
    , ConditionCheckFailed(false)
{
}

std::string SqliteResult::GetReturnCodeAndMessage() const
{
    return fmt::format("Code: \"{0}\"\n Message: \"{1}\"", ReturnCode, ErrorMessage);
}

SqliteResult SqliteResult::OK()
{
    return SqliteResult();
}

SqliteResult SqliteResult::SoftFailed(const std::string& friendlyErrorMessage)
{
    return SqliteResult(friendlyErrorMessage);
}

SqliteResult SqliteResult::Fail(int returnCode, const std::string& errorMessage)
{
    return SqliteResult(returnCode, errorMessage);
}

SqliteResult SqliteResult::FailDetailed(const std::string& friendlyErrorMessage,
    int returnCode,
    const std::string& errorMessage)
{
    return SqliteResult(friendlyErrorMessage, returnCode, errorMessage);
}
} // namespace tks
