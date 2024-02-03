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

#include "translator.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include <nlohmann/json.hpp>

namespace tks::UI
{

const std::string Translator::DefaultLanguage = "en-US";

Translator& Translator::GetInstance()
{
    static Translator instance;
    return instance;
}

bool Translator::Load(const std::string& locale, const std::filesystem::path& langPath)
{
    mLocale = locale;

    if (!std::filesystem::exists(langPath)) {
        return false;
    }

    const std::string defaultTranslationFile = mDefaultLocale + ".json";
    const std::filesystem::path defaultTranslationFilePath = langPath / defaultTranslationFile;
    if (!std::filesystem::exists(defaultTranslationFilePath)) {
        return false;
    }

    const auto defaultTranslationFileSize = std::filesystem::file_size(defaultTranslationFilePath);
    std::string translationContents(defaultTranslationFileSize, '\0');
    std::ifstream fileStream(defaultTranslationFilePath, std::ios::in | std::ios::binary);
    fileStream.read(translationContents.data(), defaultTranslationFileSize);

    Language defaultLanguage;
    defaultLanguage.locale = locale;

    nlohmann::json translationJson = nlohmann::json::parse(translationContents);
    for (const auto& item : translationJson.items()) {
        defaultLanguage.translations[item.key()] = item.value();
    }

    mLanguages.insert({ locale, defaultLanguage });

    if (locale == DefaultLanguage) {
        return true;
    }

    /*const std::string translationFile = locale + ".json";
    const std::filesystem::path translationFilePath = langPath / translationFile;

    if (!std::filesystem::exists(translationFilePath)) {
        return false;
    }
*/


    return true;
}

std::string Translator::Translate(const std::string& key)
{
    // find language we have selected
    auto lang = mLanguages.find(mLocale);
    if (lang == mLanguages.end()) {
        lang = mLanguages.find(DefaultLanguage);
    }

    // could not find default language, just return key
    if (lang == mLanguages.end()) {
        return key;
    }

    // try to find translation in selected locale
    auto translation = lang->second.translations.find(key);
    if (translation == lang->second.translations.end()) { // could not find translation, default to en-US
        auto english = mLanguages.find(DefaultLanguage);
        if (english == mLanguages.end()) {
            return key;
        }

        auto englishTranslation = english->second.translations.find(key);
        if (englishTranslation == english->second.translations.end()) {
            return key;
        }

        return englishTranslation->second;
    } else {
        return translation->second;
    }
}

Translator::Translator()
    : mDefaultLocale("en-US")
    , mLocale()
    , mLanguages()
{}
} // namespace tks::UI
