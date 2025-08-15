// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include <cstdint>
#include <string>
#include <vector>

namespace tks::Utils
{
#ifdef _WIN32
std::string ToStdString(const std::wstring& input);
#endif // _WIN32

std::int64_t UnixTimestamp();

std::string ToISODateTime(std::int64_t unixTimestamp);

int VoidPointerToInt(void* value);

void* IntToVoidPointer(int value);

std::int64_t VoidPointerToInt64(void* value);

void* Int64ToVoidPointer(std::int64_t value);

std::string TrimWhitespace(const std::string& value);

std::string ReplaceNewlineWithEllipses(const std::string& value);

std::string ReplaceAll(std::string value, const std::string& src, const std::string& dest);

std::string Uuid();

std::vector<std::string> Split(std::string s, std::string delimiter);

int ConvertMinutesToMilliseconds(const int valueInMinutes);

// SQLite interprets single quotes as string and performs no parameterization.
// If a parameter is in single quotes, then this function handles adding the LIKE operator '%'
// to the string so the parameterization takes effect
std::string FormatSqlSearchTerm(const std::string& source);

std::string ConvertListIdsToCommaDelimitedString(const std::vector<std::int64_t> ids);
} // namespace tks::Utils
