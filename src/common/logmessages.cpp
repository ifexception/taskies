// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2025 Szymon Welgus
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

#include "logmessages.h"

namespace tks::LogMessages
{
const char* OpenDatabaseConnection = "Open database connection at \"{0}\"";

const char* CloseDatabaseConnection = "Close database connection";

const char* OpenDatabaseTemplate =
    "Failed to open database \"taskies.db\" at \"{0}\". Error {1}: \"{2}\"";

const char* ExecQueryTemplate = "Failed to execute \"{0}\" statement. Error {1}: \"{2}\"";

const char* ExecQueryDidNotReturnOneResultTemplate =
    "Returned more than one (1) result when expected one (1) result. Error {0}: \"{1}\"";

const char* PrepareStatementTemplate = "Failed to prepare statment \"{0}\". Error {1}: \"{2}\"";

const char* BindParameterTemplate =
    "Failed to bind parameter \"{0}\" at \"{1}\" index. Error {2}: \"{3}\"";

const char* ExecStepTemplate = "Failed to step through statement \"{0}\". Error {1}: \"{2}\"";

const char* EntityCreated = "Created \"{0}\" with ID \"{1}\"";

const char* FilterEntities = "Filtered \"{0}\" entities with search term \"{1}\"";

const char* EntityGetById = "Retreived \"{0}\" with ID \"{1}\"";

const char* EntityUpdated = "Updated \"{0}\" with ID \"{1}\"";

const char* EntityDeleted = "Deleted \"{0}\" with ID \"{1}\"";
} // namespace tks::LogMessages
