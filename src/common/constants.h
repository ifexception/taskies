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

#pragma once

constexpr int MIN_CHARACTER_COUNT = 2;
constexpr int MAX_CHARACTER_COUNT_NAMES = 255;
constexpr int MAX_CHARACTER_COUNT_DESCRIPTIONS = 3000;

constexpr auto INITIAL_NUMBER_OF_ITEMS = 32;

namespace tks::LogMessage
{
/// <summary>
/// Use this template when we could not open a connection to the data
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed to open database "{DatabaseName}" at "{LocationToDatabaseFile}". Error {Code}: "{Message}""
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
/// /// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed to execute "{QueryName}" statement. Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecQueryTemplate;

/// <summary>
/// Use this template when a statement could not be prepared
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed to prepare statement "{StatementQuery}". Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* PrepareStatementTemplate;

/// <summary>
/// Use this template when a parameter could not be bound to the statement at specified location
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed to bind parameter "{ParameterName}" at "{Index}" index. Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* BindParameterTemplate;

/// <summary>
/// Use this template when a statement could not be stepped through
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed to step through statement "{Query}". Error {Code}: "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecStepTemplate;

/// <summary>
/// Use this template when a statement that should have returned one (1) result, but returned more than one (1) result
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Failed step execute, returned more than one (1) result when expected one (1) result". Error {Code}:
/// "{Message}""
/// </code>
/// </example>
/// </summary>
extern const char* ExecStepMoreResultsThanExpectedTemplate;

/// <summary>
/// Use this template when to log Information message when opening connection to database
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Close database connection at "{DatabaseLocation}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoOpenDatabaseConnection;

/// <summary>
/// Use this template when to log Information message when closing connection to database
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Close database connection"
/// </code>
/// </example>
/// </summary>
extern const char* InfoCloseDatabaseConnection;

/// <summary>
/// Use this template to log Information when starting the "Filter" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Filter "{Entity}" with search term "{SearchTerm}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoBeginFilterEntities;

/// <summary>
/// Use this template to log Information when ending the "Filter" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Filtered "{Count}" entities with search term "{SearchTerm}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoEndFilterEntities;

/// <summary>
/// Use this template to log Information when beginning the "GetById" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Retrieve "{Entity}" with ID "{EntityId}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoBeginGetByIdEntity;

/// <summary>
/// Use this template to log Information when ending the "GetById" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Retreived entity with ID "{EntityId}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoEndGetByIdEntity;

/// <summary>
/// Use this template to log Information when beginning the "Create" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Create "{Entity}" with name "{Name}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoBeginCreateEntity;

/// <summary>
/// Use this template to log Information when ending the "Create" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Created "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoEndCreateEntity;

/// <summary>
/// Use this template to log Information when beginning the "Update" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Update "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoBeginUpdateEntity;

/// <summary>
/// Use this template to log Information when ending the "Update" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Updated with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoEndUpdateEntity;

/// <summary>
/// Use this template to log Information when beginning the "Delete" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Delete "{Entity}" with ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoBeginDeleteEntity;

/// <summary>
/// Use this template to log Information when ending the "Delete" method
/// <example>
/// <para>Usage:</para>
/// <code>
/// "{ClassName} - Deleted ID "{ID}""
/// </code>
/// </example>
/// </summary>
extern const char* InfoEndDeleteEntity;
} // namespace tks::LogMessage
