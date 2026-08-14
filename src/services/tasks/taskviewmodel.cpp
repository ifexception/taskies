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
    , WorkdayDate()
    , EmployerName()
    , ClientName()
    , ProjectName()
    , ProjectDisplayName()
    , CategoryName()
    , CategoryColor(-1)
    , IsMeeting(false)
    , TaskAttributeValueModels()
{
}

const std::string TaskViewModel::GetDuration() const
{
    return fmt::format("{0:02}:{1:02}", Hours, Minutes);
}

const std::string TaskViewModel::TryGetUniqueIdentifier() const
{
    return UniqueIdentifier.has_value() ? UniqueIdentifier.value() : "";
}

const std::string TaskViewModel::GetTrimmedDescription() const
{
    return Utils::ReplaceNewlineWithEllipses(Description);
}

const std::string TaskViewModel::TryGetTaskAttributeValues()
{
    if (TaskAttributeValueModels.size() != 0) {
        std::string value = "";

        for (size_t i = 0; i < TaskAttributeValueModels.size(); i++) {
            if (TaskAttributeValueModels[i].BooleanValue.has_value()) {
                value += std::to_string(TaskAttributeValueModels[i].BooleanValue.value());
            } else if (TaskAttributeValueModels[i].NumericValue.has_value()) {
                value += std::to_string(TaskAttributeValueModels[i].NumericValue.value());
            } else if (TaskAttributeValueModels[i].TextValue.has_value()) {
                value += TaskAttributeValueModels[i].TextValue.value();
            } else {
                value = "<unknown>";
            }

            if (i != TaskAttributeValueModels.size() - 1) {
                value += " | ";
            }
        }
        return value;
    }
    return std::string();
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
