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

#include "utils.h"

#include <chrono>
#include <numeric>
#include <random>

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
    int size = WideCharToMultiByte(
        CP_UTF8, 0, input.data(), static_cast<int>(input.size()), NULL, 0, NULL, NULL);
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

std::string ReplaceAll(std::string value, const std::string& src, const std::string& dest)
{
    std::string replacedValue = "";
    std::string::size_type pos = 0;
    while ((pos = value.find(src, pos)) != std::string::npos) {
        value.replace(pos, src.length(), dest);
        pos += dest.length();
    }

    replacedValue = value;
    return replacedValue;
}

std::string Uuid()
{
    // https://stackoverflow.com/a/58467162/7277716
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char* v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res = "";
    for (int i = 0; i < 16; i++) {
        if (dash[i]) {
            res += "-";
        }

        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

int ConvertMinutesToMilliseconds(const int valueInMinutes)
{
    const int multiplier = 60000;
    int valueInMilliseconds = valueInMinutes * multiplier;
    return valueInMilliseconds;
}

std::string FormatSqlSearchTerm(const std::string& source)
{
    return "%" + source + "%";
}

std::string ConvertListIdsToCommaDelimitedString(const std::vector<std::int64_t> ids)
{
    return std::accumulate(
        std::begin(ids), std::end(ids), std::string(), [](std::string s, std::int64_t i) {
            return s.empty() ? std::to_string(i) : s + "," + std::to_string(i);
        });
}
} // namespace tks::Utils
