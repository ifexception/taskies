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
#pragma region DeleteEntity
const std::string DeleteConfirmationEmployerMessage =
    "Are you sure you want to delete the employer \"{0}\"?";
const std::string DeleteConfirmationClientMessage =
    "Are you sure you want to delete the client \"{0}\"?";
const std::string DeleteConfirmationProjectMessage =
    "Are you sure you want to delete the project \"{0}\"?";
const std::string DeleteConfirmationCategoryMessage =
    "Are you sure you want to delete the category \"{0}\"?";
const std::string DeleteConfirmationTaskMessage =
    "Are you sure you want to delete the task \"{0}\"?";
const std::string DeleteConfirmationAttributeGroupMessage =
    "Are you sure you want to delete the attribute group?";
const std::string DeleteConfirmationAttributesMessage =
    "Are you sure you want to delete the attributes?";
const std::string DeleteConfirmationStaticTaskAttributeValuesMessage =
    "Are you sure you want to delete the static task attribute values?";
#pragma endregion
} // namespace tks::Messages
