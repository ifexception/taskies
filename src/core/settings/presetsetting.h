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

#pragma once

#include <string>
#include <vector>

#include "presetcolumnsetting.h"

#include "../../common/enums.h"

namespace tks::Core::Settings
{
struct PresetSetting {
    std::string Uuid;
    std::string Name;
    bool IsDefault = false;
    DelimiterType Delimiter = DelimiterType::None;
    TextQualifierType TextQualifier = TextQualifierType::None;
    EmptyValues EmptyValuesHandler = EmptyValues::None;
    NewLines NewLinesHandler = NewLines::None;
    BooleanHandler BooleanHandler = BooleanHandler::None;
    bool ExcludeHeaders = false;
    bool IncludeAttributes = false;
    std::vector<PresetColumnSetting> Columns;
};
} // namespace tks::Core::Settings
