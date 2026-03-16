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

namespace tks::Messages
{
const std::string MessageDialogExtendedMessage =
    "Please try again or click \"OK\" to open your browser to log an issue";

#pragma region Employer
const std::string EditEmployerMessage = "A database error occured when fetching the employer";
const std::string UnsetDefaultEmployerMessage =
    "A database error occured while trying unset the default employer";
const std::string CreateEmployerMessage =
    "A database error occured when trying to create an employer";
const std::string UpdateEmployerMessage =
    "A database error occured when trying update the employer";
const std::string DeleteEmployerMessage =
    "A database error occured when trying to delete the employer";
const std::string FindDefaultEmployerMessage =
    "A database error occured when trying to find default employer";
#pragma endregion

} // namespace tks::Messages
