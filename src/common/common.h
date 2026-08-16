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

#include <vector>
#include <string>
#include <utility>

#include "enums.h"
#include "enumclientdata.h"

namespace tks
{
namespace Common
{
std::string GetProgramName();

std::string GetProgramNameLowerCase();

std::string GetProgramIconBundleName();

std::string GetExitIconBundleName();

std::string GetAddTaskIconBundleName();

std::string GetAboutIconBundleName();

std::string GetSettingsIconBundleName();

std::string GetQuickExportIconBundleName();

std::string GetEditTaskIconBundleName();

std::string GetCopyPasteIconBundleName();

std::string GetCopyRowIconBundleName();

std::string GetCopyWithPresetIconBundleName();

std::string GetCopyRowWithPresetIconBundleName();
std::string GetDeleteTaskIconBundleName();

std::string GetLicense();

namespace Resources
{
std::string Wizard();
} // namespace Resources

namespace Static
{
std::vector<EnumClientData<DelimiterType>> DelimitersList();
std::vector<EnumClientData<TextQualifierType>> TextQualifiersList();
std::vector<EnumClientData<EmptyValues>> EmptyValuesList();
std::vector<EnumClientData<NewLines>> NewLinesList();
std::vector<EnumClientData<BooleanHandler>> BooleanHandlerList();
std::vector<EnumClientData<TasksViewColumnTextAlignment>> TasksViewColumnTextAlignmentChoices();
std::vector<EnumClientData<TasksViewColumnEllipsisMode>> TasksViewColumnEllipsizeModeChoices();
} // namespace Static
} // namespace Common
} // namespace tks
