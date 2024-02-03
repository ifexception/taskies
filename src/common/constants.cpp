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

#include "constants.h"

namespace tks::LogMessage
{
const char* OpenDatabaseTemplate = "{0} - Failed to open database \"taskies.db\" at \"{2}\". Error {3}: \"{4}\"";
const char* ExecQueryTemplate = "{0} - Failed to execute \"{1}\" statement. Error {3}: \"{4}\"";
const char* PrepareStatementTemplate = "{0} - Failed to prepare statment \"{1}\". Error {2}: \"{3}\"";
const char* BindParameterTemplate = "{0} - Failed to bind parameter \"{1}\" at \"{2}\" index. Error {3}: \"{4}\"";
const char* ExecStepTemplate = "{0} - Failed to step through statement \"{1}\". Error {2}: \"{3}\"";
const char* ExecStepMoreResultsThanExpectedTemplate =
    "{0} - Failed step execute, returned more than one (1) result when expected one (1) result. Error {1}: \"{2}\"";

// Info Messages
const char* InfoOpenDatabaseConnection = "{0} - Open database connection at \"{1}\"";
const char* InfoCloseDatabaseConnection = "{0} - Close database connection";
const char* InfoBeginFilterEntities = "{0} - Filter \"{1}\" with search term \"{2}\"";
const char* InfoEndFilterEntities = "{0} - Filtered \"{1}\" entities with search term \"{2}\"";
const char* InfoBeginGetByIdEntity = "{0} - Retrieve \"{1}\" with ID \"{2}\"";
const char* InfoEndGetByIdEntity = "{0} - Retreived entity with ID \"{1}\"";
const char* InfoBeginCreateEntity = "{0} - Create \"{1}\" with name \"{2}\"";
const char* InfoEndCreateEntity = "{0} - Created with ID \"{1}\"";
const char* InfoBeginUpdateEntity = "{0} - Update \"{1}\" with ID \"{2}\"";
const char* InfoEndUpdateEntity = "{0} - Updated with ID \"{1}\"";
const char* InfoBeginDeleteEntity = "{0} - Delete \"{1}\" with ID \"{2}\"";
const char* InfoEndDeleteEntity = "{0} - Deleted ID \"{1}\"";
} // namespace tks::LogMessage
