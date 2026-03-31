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
#pragma region Common
const std::string PrepareStatementMessage = "A database error occurred, and your data could not be "
                                            "saved. Please try again or check your inputs";
const std::string BindStatementMessage =
    "Unable to process the data provided. Please check your inputs and try again";
const std::string StepStatementMessage =
    "A database error occurred while querying/saving your data";
const std::string StepStatementReturnedMultipleRowsMessage =
    "Operation returned more rows than expected when trying to find your data";
#pragma endregion

#pragma region Employer
const std::string FilterEmployersMessage = "Something went wrong while trying to filter employers";
const std::string CreateEmployerMessage = "Something went wrong while trying to save an employer";
const std::string UpdateEmployerMessage =
    "Something went wrong while trying to update the employer";
const std::string DeleteEmployerMessage =
    "Something went wrong while trying to update the employer";
const std::string UnsetDefaultEmployerMessage =
    "Something went wrong while trying to unset default employer";
const std::string SelectDefaultEmployerMessage =
    "Something went wrong while trying get a default employer";
const std::string GetByIdEmployerMessage = "Something went wrong while trying get an employer";
#pragma endregion

#pragma region Client
const std::string FilterClientsMessage = "Something went wrong while trying to filter clients";
const std::string CreateClientMessage = "Something went wrong while trying to save a client";
const std::string UpdateClientMessage = "Something went wrong while trying to update the client";
const std::string DeleteClientMessage = "Something went wrong while trying to update the client";
const std::string GetByIdClientMessage = "Something went wrong while trying get a client";
const std::string FilterClientsByEmployerMessage =
    "Something went wrong while trying get clients filtered by employer";
#pragma endregion

#pragma region Project
const std::string FilterProjectsMessage = "Something went wrong while trying to filter projects";
const std::string CreateProjectMessage = "Something went wrong while trying to save a project";
const std::string UpdateProjectMessage = "Something went wrong while trying to update the project";
const std::string DeleteProjectMessage = "Something went wrong while trying to update the project";
const std::string GetByIdProjectMessage = "Something went wrong while trying get a project";
const std::string FilterProjectsByEmployerMessage =
    "Something went wrong while trying get projects filtered by employer";
const std::string UnsetDefaultProjectMessage =
    "Something went wrong while trying to unset default employer";
#pragma endregion
} // namespace tks::Messages
