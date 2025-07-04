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

#include "taskviewmodel.h"

#include <date/date.h>
#include <fmt/format.h>

#include "../../utils/utils.h"

namespace tks::Services
{
TaskViewModel::TaskViewModel()
    : TaskId(-1)
    , Billable(false)
    , UniqueIdentifier()
    , Hours(-1)
    , Minutes(-1)
    , Description()
    , DateCreated(0)
    , DateModified(0)
    , IsActive(false)
    , ProjectId(-1)
    , CategoryId(-1)
    , WorkdayId(-1)
    , ProjectName()
    , CategoryName()
{
}

const std::string TaskViewModel::GetDuration() const
{
    return fmt::format("{0:02}:{1:02}", Hours, Minutes);
}

const std::string TaskViewModel::GetTrimmedDescription()
{
    return Utils::ReplaceNewlineWithEllipses(Description);
}

const std::string TaskViewModel::GetDateCreatedString() const
{
    date::sys_seconds dateTime{ std::chrono::seconds{ DateCreated } };
    std::string dateString = date::format("%Y-%m-%d %I:%M:%S %p", dateTime);
    return dateString;
}

const std::string TaskViewModel::GetDateModifiedString() const
{
    date::sys_seconds dateTime{ std::chrono::seconds{ DateModified } };
    std::string dateString = date::format("%Y-%m-%d %I:%M:%S %p", dateTime);
    return dateString;
}
} // namespace tks::Services
