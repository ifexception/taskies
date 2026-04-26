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

#include "configresult.h"

namespace tks
{
ConfigResult::ConfigResult()
    : Success(true)
    , HeaderMessage("")
    , UserMessage("")
    , ErrorMessage("")
{
}

ConfigResult::ConfigResult(const std::string& headerMessage,
    const std::string& userMessage,
    const std::string& errorMessage)
    : Success(false)
    , HeaderMessage(headerMessage)
    , UserMessage(userMessage)
    , ErrorMessage(errorMessage)
{
}

ConfigResult ConfigResult::OK()
{
    return ConfigResult();
}

ConfigResult ConfigResult::Fail(const std::string& headerMessage,
    const std::string& userMessage,
    const std::string& errorMessage)
{
    return ConfigResult(headerMessage, userMessage, errorMessage);
}
} // namespace tks
