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
const std::string DeleteClientMessage = "Something went wrong while trying to delete the client";
const std::string GetByIdClientMessage = "Something went wrong while trying get a client";
const std::string FilterClientsByEmployerMessage =
    "Something went wrong while trying get clients by employer";
#pragma endregion

#pragma region Project
const std::string FilterProjectsMessage = "Something went wrong while trying to filter projects";
const std::string CreateProjectMessage = "Something went wrong while trying to save a project";
const std::string UpdateProjectMessage = "Something went wrong while trying to update the project";
const std::string DeleteProjectMessage = "Something went wrong while trying to update the project";
const std::string GetByIdProjectMessage = "Something went wrong while trying get a project";
const std::string FilterProjectsByEmployerMessage =
    "Something went wrong while trying get projects by employer";
const std::string UnsetDefaultProjectMessage =
    "Something went wrong while trying to unset default employer";
#pragma endregion

#pragma region Category
const std::string FilterCategoriesMessage =
    "Something went wrong while trying to filter categories";
const std::string CreateCategoryMessage =
    "Something went wrong while trying to save a \"{0}\" category";
const std::string UpdateCategoryMessage =
    "Something went wrong while trying to update the category";
const std::string DeleteCategoryMessage =
    "Something went wrong while trying to delete the category";
const std::string GetByIdCategoryMessage = "Something went wrong while trying get a category";
const std::string FilterCategoriesByProjectMessage =
    "Something went wrong while trying get categories by project";
#pragma endregion

#pragma region AttributeGroups
const std::string FilterAttributeGroupsMessage =
    "Something went wrong while trying to filter attribute groups";
const std::string CreateAttributeGroupMessage =
    "Something went wrong while trying to save a attribute group";
const std::string UpdateAttributeGroupMessage =
    "Something went wrong while trying to update the attribute group";
const std::string DeleteAttributeGroupMessage =
    "Something went wrong while trying to delete the attribute group";
const std::string GetByIdAttributeGroupMessage =
    "Something went wrong while trying get a attribute group";
const std::string UnsetDefaultAttributeGroupMessage =
    "Something went wrong while trying to unset default attribute group";
const std::string FilterAttributeGroupsByStaticFlagMessage =
    "Something went wrong while trying get static attribute groups";
const std::string CheckUsageAttributeGroupMessage =
    "Something went wrong while check the usages of the attribute group";
#pragma endregion

#pragma region Attributes Types
const std::string FilterAttributeTypesMessage =
    "Something went wrong while trying to filter attribute types";
#pragma endregion

#pragma region Attributes
const std::string FilterAttributesMessage =
    "Something went wrong while trying to filter attributes";
const std::string FilterAttributesByAttributeGroupMessage =
    "Something went wrong while trying to filter attributes linked to attribute group";
const std::string CreateAttributeMessage = "Something went wrong while trying to save a attribute";
const std::string UpdateAttributeMessage =
    "Something went wrong while trying to update the attribute";
const std::string DeleteAttributeMessage =
    "Something went wrong while trying to delete the attribute";
const std::string GetByIdAttributeMessage = "Something went wrong while trying get a attribute";
const std::string FilterAttributesByStaticFlagMessage =
    "Something went wrong while trying get static linked attributes";
const std::string CheckUsageAttributeMessage =
    "Something went wrong while check the usages of the attribute";
#pragma endregion

#pragma region StaticAttributes
const std::string FilterStaticAttributesMessage =
    "Something went wrong while trying to filter static attributes";
const std::string CreateStaticAttributeMessage =
    "Something went wrong while trying to save static attributes";
const std::string UpdateStaticAttributeMessage =
    "Something went wrong while trying to update static attributes";
const std::string DeleteStaticAttributeMessage =
    "Something went wrong while trying to delete the static attributes";
const std::string FilterStaticAttributesByAttributeGroupIdMessage =
    "Something went wrong while trying get static attributes by attribute group";
const std::string CheckUsageStaticAttributeMessage =
    "Something went wrong while check the usages of the static attributes";
#pragma endregion

#pragma region TaskAttributeValues
const std::string CreateTaskAttributeValuesMessage =
    "Something went wrong while trying to save the task attribute values";
const std::string UpdateTaskAttributeValuesMessage =
    "Something went wrong while trying to update the task attribute values";
const std::string DeleteTaskAttributeValuesMessage =
    "Something went wrong while trying to delete the task attribute value(s)";
const std::string FilterTaskAttributeValuesByTaskIdMessage =
    "Something went wrong while trying get task attribute values by task";
#pragma endregion

#pragma region Workday
const std::string GetWorkdayIdByDateMessage =
    "Something went wrong while trying get workday by selected date";
#pragma endregion

#pragma region AttendedMeetings
const std::string FilterAttendedMeetingsByTodayDateMessage =
    "Something went wrong while trying get todays attended meetings";
const std::string CreateAttendedMeetingMessage =
    "Something went wrong while trying get save an attended meeting";
const std::string DeleteAttendedMeetingMessage =
    "Something went wrong while trying get delete the attended meeting";
#pragma endregion

#pragma region Tasks
const std::string GetByIdTaskMessage = "Something went wrong while trying get a task";
const std::string GetDescriptionByIdTaskMessage = "Something went wrong while trying get the task's description";
const std::string CreateTaskMessage = "Something went wrong while trying get save a task";
const std::string UpdateTaskMessage =
    "Something went wrong while trying to update the task";
const std::string DeleteTaskMessage =
    "Something went wrong while trying to delete the task";
#pragma endregion

#pragma region TaskDuration
const std::string DurationCalculationMessage =
    "Something went wrong when fetching time worked";
const std::string DurationIncrementMessage =
    "Something went wrong when incrementing time worked";
#pragma endregion

} // namespace tks::Messages
