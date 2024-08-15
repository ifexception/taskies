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

#include "enums.h"

namespace tks
{
std::string BuildConfigurationToString(BuildConfiguration buildConfiguration)
{
    switch (buildConfiguration) {
    case tks::BuildConfiguration::Undefined:
        return "Undefined";
    case tks::BuildConfiguration::Debug:
        return "Debug";
    case tks::BuildConfiguration::Release:
        return "Release";
    default:
        return "";
    }
}

std::string WindowStateToString(WindowState windowState)
{
    switch (windowState) {
    case tks::WindowState::Normal:
        return "Normal";
    case tks::WindowState::Minimized:
        return "Minimized";
    case tks::WindowState::Hidden:
        return "Hidden";
    case tks::WindowState::Maximized:
        return "Maximized";
    default:
        return "";
    }
}

std::string NotificationTypeToString(NotificationType notificationType)
{
    switch (notificationType) {
    case tks::NotificationType::Information:
        return "Information";
    case tks::NotificationType::Error:
        return "Error";
    default:
        return "";
    }
}

std::string MapDelimiterEnumToValue(Delimiter delimiter)
{
    switch (delimiter) {
    case tks::Delimiter::None:
        return "";
    case tks::Delimiter::Comma:
        return ",";
    case tks::Delimiter::Semicolon:
        return ";";
    case tks::Delimiter::Pipe:
        return "|";
    case tks::Delimiter::Tab:
        return "(tab)";
    case tks::Delimiter::Space:
        return "(space)";
    default:
        return "";
    }
}

Delimiter MapValueToDelimiterEnum(std::string delimiter)
{
    if (delimiter.empty() || delimiter == "") {
        return Delimiter::None;
    } else if (delimiter == ",") {
        return Delimiter::Comma;
    } else if (delimiter == ";") {
        return Delimiter::Semicolon;
    } else if (delimiter == "|") {
        return Delimiter::Pipe;
    } else if (delimiter == "\t") {
        return Delimiter::Tab;
    } else if (delimiter == " ") {
        return Delimiter::Space;
    } else {
        return Delimiter::None;
    }
}
} // namespace tks
