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

namespace tks::ErrorMessages
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
    "A database error occured when trying to update the employer";
const std::string DeleteEmployerMessage =
    "A database error occured when trying to delete the employer";
const std::string FindDefaultEmployerMessage =
    "A database error occured when trying to find default employer";
const std::string FilterEmployersMessage = "A database error occured when fetching employers";
#pragma endregion

#pragma region Client
const std::string EditClientMessage = "A database error occured when fetching the client";
const std::string CreateClientMessage = "A database error occured when trying to create a client";
const std::string UpdateClientMessage = "A database error occured when trying to update the client";
const std::string DeleteClientMessage = "A database error occured when trying to delete the client";
const std::string FilterClientsMessage = "A database error occured when fetching clients";
#pragma endregion

#pragma region Project
const std::string EditProjectMessage = "A database error occured when fetching the project";
const std::string UnsetDefaultProjectMessage =
    "A database error occured while trying unset the default project";
const std::string CreateProjectMessage = "A database error occured when trying to create a project";
const std::string UpdateProjectMessage =
    "A database error occured when trying to update the project";
const std::string DeleteProjectMessage =
    "A database error occured when trying to delete the project";
#pragma endregion
} // namespace tks::ErrorMessages
