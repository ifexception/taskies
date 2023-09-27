// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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

namespace sqlite
{
// SQLite interprets single quotes as string and performs no parameterization.
// If a parameter is in single quotes, then this function handles adding the LIKE operator '%'
// to the string so the parameterization takes effect
std::string FormatSearchTerm(const std::string& source);

namespace pragmas
{
// https://phiresky.github.io/blog/2020/sqlite-performance-tuning/
extern const char* ForeignKeys;
extern const char* JournalMode;
extern const char* Synchronous;
extern const char* TempStore;
extern const char* MmapSize;

extern const char* Optimize;
} // namespace pragmas
} // namespace sqlite
} // namespace tks::Utils
