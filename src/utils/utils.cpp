// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include "utils.h"

#include <chrono>

#include <date/date.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // _WIN32

namespace tks::Utils
{
#ifdef _WIN32
std::string ToStdString(const std::wstring& input)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), NULL, 0, NULL, NULL);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &input[0], (int) input.size(), &result[0], size, NULL, NULL);
    return result;
}
#endif // _WIN32

std::int64_t UnixTimestamp()
{
    auto tp = std::chrono::system_clock::now();
    auto dur = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(dur).count();
    return seconds;
}

std::string ToISODateTime(std::int64_t unixTimestamp)
{
    auto now = std::chrono::system_clock::now();
    auto date = date::format("%F %T", date::floor<std::chrono::seconds>(now));
    return date;
}

int VoidPointerToInt(void* value)
{
    intptr_t p = reinterpret_cast<intptr_t>(value);
    return static_cast<int>(p);
}

void* IntToVoidPointer(int value)
{
    intptr_t p = static_cast<intptr_t>(value);
    return reinterpret_cast<void*>(p);
}

std::int64_t VoidPointerToInt64(void* value)
{
    intptr_t p = reinterpret_cast<intptr_t>(value);
    return static_cast<std::int64_t>(p);
}

void* Int64ToVoidPointer(std::int64_t value)
{
    intptr_t p = static_cast<intptr_t>(value);
    return reinterpret_cast<void*>(p);
}

std::string TrimWhitespace(const std::string& value)
{
    const std::string whitespace = " \n\r\t\f\v";

    std::string trimmedText = value;
    const size_t start = trimmedText.find_first_not_of(whitespace);
    if (start == std::wstring::npos) {
        return "";
    }

    const size_t end = trimmedText.find_last_not_of(whitespace);
    trimmedText = trimmedText.substr(start, end + 1 - start);

    return trimmedText;
}

std::string ReplaceNewlineWithEllipses(const std::string& value)
{
    std::string replacedString = "";
    const std::string newline = "\n";
    const std::string ellipses = "...";

    std::string::size_type index = value.find(newline);
    if (index != std::string::npos) {
        replacedString = value.substr(0, value.find(newline));
        replacedString += ellipses;
    } else {
        replacedString = value;
    }
    /*size_t startPosition = 0;
    while ((startPosition = replacedString.find(newline, startPosition) != std::string::npos)) {
        replacedString.replace(startPosition, newline.length(), ellipses);
        startPosition += ellipses.length();
    }*/

    return replacedString;
}

namespace sqlite
{
std::string FormatSearchTerm(const std::string& source)
{
    return "%" + source + "%";
}

namespace pragmas
{
const char* ForeignKeys = "PRAGMA foreign_keys = ON;";
const char* JournalMode = "PRAGMA journal_mode = WAL;";
const char* Synchronous = "PRAGMA synchronous = normal;";
const char* TempStore = "PRAGMA temp_store = memory;";
const char* MmapSize = "PRAGMA mmap_size = 30000000000;";

const char* Optimize = "PRAGMA optimize;";
} // namespace pragmas
} // namespace sqlite
} // namespace tks::Utils
