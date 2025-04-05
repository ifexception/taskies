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

#include <string>

namespace tks
{
enum class EditListEntityType { Employer, Client, Project, Category, AttributeGroup, Attribute };

enum class WindowState : int { Normal = 1, Minimized = 2, Hidden = 3, Maximized = 4 };

enum class BuildConfiguration { Undefined, Debug, Release };

enum class NotificationType { Information = 1, Error };

enum class DelimiterType : int { None = 0, Comma, Semicolon, Pipe, Tab, Space };

enum class TextQualifierType : int { None = 1, DoubleQuotes, SingleQuotes };

enum class EmptyValues : int { None = 0, Blank, Null };

enum class NewLines : int { None = 0, Preserve, Merge };

enum class BooleanHandler : int { None = 0, OneZero, TrueFalse, YesNo };

enum class JoinType { None = 0, InnerJoin = 1, LeftJoin = 2 };

enum class FieldType { Default = 0, Formatted };

enum class TaskDurationType { Default = 1, Billable };

enum class TaskDurationField { Day = 1, Week, Month, Range = 10 };

enum class AttributeTypes { Text = 1, Boolean, Numeric };

std::string BuildConfigurationToString(BuildConfiguration buildConfiguration);
std::string WindowStateToString(WindowState windowState);
std::string NotificationTypeToString(NotificationType notificationType);

std::string MapTextQualifierEnumToValue(TextQualifierType textQualifier);

std::string MapDelimiterEnumToValue(DelimiterType delimiter);
} // namespace tks
