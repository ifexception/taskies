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
#pragma endregion

#pragma region Employer
const std::string FilterEmployerPrepareStatementMessage =
    "Something went wrong while trying to filter employers";
const std::string CreateEmployerPrepareStatementMessage =
    "Something went wrong while trying to save an employer";
#pragma endregion
} // namespace tks::Messages
