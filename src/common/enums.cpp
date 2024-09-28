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

std::string MapDelimiterEnumToValue(DelimiterType delimiter)
{
    switch (delimiter) {
    case tks::DelimiterType::None:
        return "";
    case tks::DelimiterType::Comma:
        return ",";
    case tks::DelimiterType::Semicolon:
        return ";";
    case tks::DelimiterType::Pipe:
        return "|";
    case tks::DelimiterType::Tab:
        return "(tab)";
    case tks::DelimiterType::Space:
        return "(space)";
    default:
        return "";
    }
}

DelimiterType MapValueToDelimiterEnum(std::string delimiter)
{
    if (delimiter.empty() || delimiter == "") {
        return DelimiterType::None;
    } else if (delimiter == ",") {
        return DelimiterType::Comma;
    } else if (delimiter == ";") {
        return DelimiterType::Semicolon;
    } else if (delimiter == "|") {
        return DelimiterType::Pipe;
    } else if (delimiter == "\t") {
        return DelimiterType::Tab;
    } else if (delimiter == " ") {
        return DelimiterType::Space;
    } else {
        return DelimiterType::None;
    }
}

std::string MapBooleanEnumToValue(BooleanHandler booleanHandler)
{
    switch (booleanHandler) {
    case BooleanHandler::OneZero:
        return "1|0";
    case BooleanHandler::TrueFalse:
        return "true|false";
    case BooleanHandler::YesNo:
        return "yes|no";
    default:
        return "";
    }
}

BooleanHandler MapValueToBooleanHandlerEnum(std::string booleanHandler)
{
    if (booleanHandler.empty() || booleanHandler == "1|0") {
        return BooleanHandler::OneZero;
    } else if (booleanHandler == "true|false") {
        return BooleanHandler::TrueFalse;
    } else if (booleanHandler == "yes|no") {
        return BooleanHandler::YesNo;
    } else {
        return BooleanHandler::OneZero;
    }
}
} // namespace tks
