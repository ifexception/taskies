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

#pragma once

namespace tks::LogMessages
{
/// <summary>
/// Use this template when to log message when opening connection to database
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Open database connection at "{DatabaseLocation}""
/// </code>
/// </example>
/// </summary>
extern const char* OpenDatabaseConnection;

/// <summary>
/// Use this template when to log Information message when closing connection to database
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Close database connection"
/// </code>
/// </example>
/// </summary>
extern const char* CloseDatabaseConnection;

/// <summary>
/// Use this template when a statement that should have returned one (1) result, but returned more
/// than one (1) result
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed step execute, returned more than one (1) result when expected one (1)
/// result". Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecQueryDidNotReturnOneResultTemplate;

/// <summary>
/// Use this template when we could not open a connection to the data
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed to open database "{DatabaseName}" at "{LocationToDatabaseFile}". Error {Code}:
/// "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* OpenDatabaseTemplate;

/// <summary>
/// Use this template when a (simple) SQLite query failed to execute
/// <para>
/// <code>
/// <a href="https://www.sqlite.org/c3ref/exec.html">sqlite3_exec()</a>
/// </code>
/// </para>
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed to execute "{Query}" statement. Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecQueryTemplate;

/// <summary>
/// Use this template when a statement could not be prepared
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed to prepare statement "{StatementQuery}". Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* PrepareStatementTemplate;

/// <summary>
/// Use this template when a parameter could not be bound to the statement at specified location
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed to bind parameter "{ParameterName}" at "{Index}" index. Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* BindParameterTemplate;

/// <summary>
/// Use this template when a statement could not be stepped through
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Failed to step through statement "{Query}". Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecStepTemplate;

/// <summary>
/// Use this template to log Information when creating an entity
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Created "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* EntityCreated;

/// <summary>
/// Use this template to log Information when filtering entities
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Filtered "{Count}" entities with search term "{SearchTerm}""
/// </code>
/// </example>
/// </summary>
extern const char* FilterEntities;

/// <summary>
/// Use this template to log Information when getting an entity by id
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Retreived {Entity} with ID "{EntityId}""
/// </code>
/// </example>
/// </summary>
extern const char* EntityGetById;

/// <summary>
/// Use this template to log Information when updating an entity
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Updated "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* EntityUpdated;

/// <summary>
/// Use this template to log Information when an entity is deleted
/// <example>
/// <para>Usage:</para>
/// <code>
/// "Deleted "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* EntityDeleted;

/// <summary>
/// Use this template to log Information an entity usage
/// <example>
/// <para>Usage:</para>
/// <code>
/// ""{Entity}" with ID "{ID}" is used status "{Status}""
/// </code>
/// </example>
/// </summary>
extern const char* EntityUsage;
} // namespace tks::LogMessages
