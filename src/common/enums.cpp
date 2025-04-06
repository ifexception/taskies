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

std::string AttributeTypeToString(AttributeTypes attributeType)
{
    switch (attributeType) {
    case tks::AttributeTypes::Text:
        return "Text";
    case tks::AttributeTypes::Boolean:
        return "Boolean";
    case tks::AttributeTypes::Numeric:
        return "Numeric";
    default:
        return "";
    }
}

std::string MapTextQualifierEnumToValue(TextQualifierType textQualifier)
{
    switch (textQualifier) {
    case TextQualifierType::None:
        return "";
    case TextQualifierType::DoubleQuotes:
        return "\"";
    case TextQualifierType::SingleQuotes:
        return "'";
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
} // namespace tks
