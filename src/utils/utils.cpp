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
    if (input.empty()) {
        return std::string();
    }

    int sizeNeeded = WideCharToMultiByte(
        CP_UTF8, 0, input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);

    if (sizeNeeded <= 0) {
        return std::string();
    }

    std::string result(static_cast<size_t>(sizeNeeded), '\0');

    WideCharToMultiByte(CP_UTF8,
        0,
        input.data(),
        static_cast<int>(input.size()),
        result.data(),
        sizeNeeded,
        nullptr,
        nullptr);

    return result;
}
#endif // _WIN32

std::int64_t UnixTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::int64_t UnixTimestampTodayMidnight()
{
    auto midnight = std::chrono::time_point_cast<date::days>(std::chrono::system_clock::now());
    auto duration = midnight.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::int64_t UnixTimestampTomorrowMidnight()
{
    auto midnight = std::chrono::time_point_cast<date::days>(std::chrono::system_clock::now());
    auto tomorrowMidnight = midnight + date::days{ 1 };
    auto duration = tomorrowMidnight.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
}

std::string ToISODateTime(std::int64_t unixTimestamp)
{
    date::sys_seconds tp{ std::chrono::seconds{ unixTimestamp } };
    std::string date = date::format("%F %T", tp);
    return date;
}

std::string Timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::string date = date::format("%F-%T", now);
    date = ReplaceAll(date, ":", "-");
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
    if (start == std::string::npos) {
        return "";
    }

    const size_t end = trimmedText.find_last_not_of(whitespace);
    trimmedText = trimmedText.substr(start, end + 1 - start);

    return trimmedText;
}

std::string TrimToLengthAndAddEllipses(const std::string& value, std::size_t length)
{
    const std::string ellipses = "...";

    if (length == 0) {
        length = MAX_CHARACTER_TRIM_COUNT;
    }
    if (value.size() <= length) {
        return value;
    }

    return value.substr(0, length) + ellipses;
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

std::string RightTrim(std::string value)
{
    size_t end = value.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
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

std::vector<std::string> Split(std::string s, std::string delimiter)
{
    size_t pos_start = 0, pos_end, delimiter_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delimiter_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

int ConvertMinutesToMilliseconds(const int valueInMinutes)
{
    const int multiplier = 60000;
    int valueInMilliseconds = valueInMinutes * multiplier;
    return valueInMilliseconds;
}

std::string ToExcelColumnName(int columnNumber)
{
    std::string columnName = "";

    while (columnNumber > 0) {
        int modulo = (columnNumber - 1) % 26;
        char name = 'A' + modulo;
        columnName = std::string{ name } + columnName;
        columnNumber = (columnNumber - modulo) / 26;
    }

    return columnName;
}

std::string FormatSqlSearchTerm(const std::string& source)
{
    return "%" + source + "%";
}

// clang-format off
std::string ConvertListIdsToCommaDelimitedString(const std::vector<std::int64_t> ids)
{
    return std::accumulate(
        std::begin(ids),
        std::end(ids),
        std::string(),
        [](std::string s, std::int64_t i) {
            return s.empty() ? std::to_string(i) : s + "," + std::to_string(i);
        }
    );
}

std::string ConvertListStringToCommaDelimitedString(const std::vector<std::string> inputs)
{
    return std::accumulate(
        std::begin(inputs),
        std::end(inputs),
        std::string(),
        [](std::string s, std::string i) {
            return s.empty() ? i : s + "," + i;
        }
    );
}
// clang-format on

static bool IsEmoji(unsigned int codepoint)
{
    // Comprehensive common emoji Unicode ranges
    if ((codepoint >= 0x1F300 && codepoint <= 0x1F6FF) || // Misc Symbols & Pictographs / Emoticons
        (codepoint >= 0x1F900 && codepoint <= 0x1F9FF) || // Supplemental Symbols & Pictographs
        (codepoint >= 0x1FA00 && codepoint <= 0x1FA6F) || // Symbols and Pictographs Extended-A
        (codepoint >= 0x2600 && codepoint <= 0x27BF)) // Miscellaneous Symbols & Dingbats
    {
        return true;
    }
    return false;
}

// Remove all emoji from a std::wstring and return narrow string safely
std::string RemoveEmoticons(const std::wstring& input)
{
    std::wstring result;
    result.reserve(input.length()); // Performance optimization to minimize allocations

    for (size_t i = 0; i < input.length(); /* increment handled manually */) {
        wchar_t ch = input[i];

        // Safe Surrogate pair parsing for 32-bit codepoints on Windows
        if (ch >= 0xD800 && ch <= 0xDBFF && (i + 1) < input.length()) {
            wchar_t next = input[i + 1];
            if (next >= 0xDC00 && next <= 0xDFFF) {
                // Calculate the true UTF-32 codepoint value
                unsigned int codepoint = 0x10000 + ((static_cast<unsigned int>(ch) & 0x3FF) << 10) +
                                         (static_cast<unsigned int>(next) & 0x3FF);

                if (IsEmoji(codepoint)) {
                    i += 2; // Skip both surrogate units safely without trailing skips
                    continue;
                }

                result += ch;
                result += next;
                i += 2; // dvance past both parts of the accepted surrogate pair
                continue;
            }
        }

        if (!IsEmoji(static_cast<unsigned int>(ch))) {
            result += ch;
        }
        i++;
    }

    return ToStdString(result);
}

int RoundUpToMultiple(int number, int multiple)
{
    int result = number + multiple / 2;
    result -= result % multiple;
    return result;
}

void DeconstructDurationTimePeriod(const int value, int& hours, int& minutes)
{
    minutes = value % 60;
    hours = value / 60;
}
} // namespace tks::Utils
