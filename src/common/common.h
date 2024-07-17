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

#pragma once

#include <vector>
#include <string>

namespace tks::Common
{
std::string GetProgramName();

std::string GetProgramIconBundleName();

std::string GetExitIconBundleName();

std::string GetAddTaskIconBundleName();

std::string GetAboutIconBundleName();

std::string GetPreferencesIconBundleName();

std::string GetLicense();

namespace Resources
{
std::string Bell();
std::string BellNotification();
std::string Wizard();
}

namespace Static
{
std::vector<std::string> DelimiterList();
std::vector<std::string> TextQualifierList();
std::vector<std::string> EndOfLineList();
std::vector<std::string> EmptyValueHandlerList();
std::vector<std::string> NewLinesHandlerList();
}
}
