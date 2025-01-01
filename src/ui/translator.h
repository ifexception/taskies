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

#include <filesystem>
#include <map>
#include <string>

#define i18n(key) tks::UI::Translator::GetInstance().Translate(key)

namespace tks::UI
{
class Translator
{
public:
    struct Language {
        std::string locale;
        std::string name;
        std::map<std::string, std::string> translations;
    };

    static Translator& GetInstance();

    Translator(const Translator&) = delete;

    Translator& operator=(const Translator&) = delete;

    bool Load(const std::string& locale, const std::filesystem::path& langPath);
    std::string Translate(const std::string& key);

private:
    Translator();

    std::string mDefaultLocale;
    std::string mLocale;
    std::map<std::string, Language> mLanguages;

    static const std::string DefaultLanguage;
};
}
