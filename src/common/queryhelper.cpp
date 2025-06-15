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

#include "queryhelper.h"

namespace tks::QueryHelper
{
const char* ForeignKeys = "PRAGMA foreign_keys = ON;";
const char* JournalMode = "PRAGMA journal_mode = WAL;";
const char* Synchronous = "PRAGMA synchronous = normal;";
const char* TempStore = "PRAGMA temp_store = memory;";
const char* MmapSize = "PRAGMA mmap_size = 30000000000;";

const char* Optimize = "PRAGMA optimize;";
}
