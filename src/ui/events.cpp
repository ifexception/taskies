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

#include "events.h"

wxDEFINE_EVENT(tksEVT_ERROR, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_ADDNOTIFICATION, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_TASKDATEADDED, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_TASKDATEDELETED, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_TASKDATEDCHANGEDTO, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_TASKDATEDCHANGEDFROM, wxCommandEvent);
wxDEFINE_EVENT(tksEVT_TASKDLGATTRIBUTESADDED, wxCommandEvent);
