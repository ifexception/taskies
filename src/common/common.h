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

#include <vector>
#include <string>
#include <utility>

#include "enums.h"

namespace tks
{
namespace Common
{
std::string GetProgramName();

std::string GetProgramIconBundleName();

std::string GetExitIconBundleName();

std::string GetAddTaskIconBundleName();

std::string GetAboutIconBundleName();

std::string GetPreferencesIconBundleName();

std::string GetLicense();

struct PresetColumn {
    std::string Column;
    std::string OriginalColumn;
    int Order;

    PresetColumn()
        : Column()
        , OriginalColumn()
        , Order(-1)
    {
    }
};

struct Preset {
    std::string Uuid;
    std::string Name;
    bool IsDefault;
    DelimiterType Delimiter;
    TextQualifierType TextQualifier;
    EmptyValues EmptyValuesHandler;
    NewLines NewLinesHandler;
    BooleanHandler BooleanHandler;
    bool ExcludeHeaders;
    std::vector<PresetColumn> Columns;

    Preset()
        : Uuid()
        , Name()
        , IsDefault(false)
        , Delimiter(DelimiterType::None)
        , TextQualifier(TextQualifierType::None)
        , EmptyValuesHandler(EmptyValues::None)
        , NewLinesHandler(NewLines::None)
        , BooleanHandler(BooleanHandler::None)
        , ExcludeHeaders(false)
        , Columns()
    {
    }
    ~Preset() {}
};

namespace Resources
{
std::string Bell();
std::string BellNotification();
std::string Wizard();
} // namespace Resources

namespace Static
{
std::vector<std::pair<std::string, DelimiterType>> DelimiterList();
std::vector<std::pair<std::string, TextQualifierType>> TextQualifierList();
std::vector<std::string> EmptyValueHandlerList();
std::vector<std::string> NewLinesHandlerList();
std::vector<std::string> BooleanHandlerList();
} // namespace Static
} // namespace Common
} // namespace tks
