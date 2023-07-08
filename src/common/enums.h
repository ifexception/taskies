// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2023 Szymon Welgus
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
enum class EditListEntityType { Employer, Client, Project, Category };

enum class WindowState : int { Normal = 1, Minimized = 2, Hidden = 3, Maximized = 4 };

enum class BuildConfiguration { Undefined, Debug, Release };

enum class NotificationType { Information = 1, Error};

std::string BuildConfigurationToString(BuildConfiguration buildConfiguration);
std::string WindowStateToString(WindowState windowState);
std::string NotificationTypeToString(NotificationType notificationType);
} // namespace tks
