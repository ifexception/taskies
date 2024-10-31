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

#define TASKIES_MAJOR 0
#define TASKIES_MINOR 1
#define TASKIES_PATCH 8
#define TASKIES_SRLC "alpha"

#define TASKIES_VERSION (TASKIES_MAJOR * 1000 + TASKIES_MINOR * 100 + TASKIES_PATCH)

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

#define COMPANY_NAME "Taskies"
#define COPYRIGHT_STR "Copyright (C) 2024 Szymon Welgus"

#define FILE_DESCRIPTION "Taskies"
#define TASKIES_FILE_VERSION TASKIES_MAJOR, TASKIES_MINOR, TASKIES_PATCH

#define FILE_VERSION_STR \
    STRINGIFY(TASKIES_MAJOR) \
    "." STRINGIFY(TASKIES_MINOR) "." STRINGIFY(TASKIES_PATCH)

#define PRODUCT_VERSION TASKIES_FILE_VERSION
#define PRODUCT_VERSION_STR FILE_VERSION_STR
#define PRODUCT_NAME FILE_DESCRIPTION

#define INTERNAL_FILE_NAME FILE_DESCRIPTION ".exe"
#define ORIGINAL_FILE_NAME INTERNAL_FILE_NAME
