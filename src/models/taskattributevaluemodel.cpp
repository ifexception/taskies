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

#include "taskattributevaluemodel.h"

#include <date/date.h>

namespace tks::Model
{
TaskAttributeValueModel::TaskAttributeValueModel()
    : TaskAttributeValueId(-1)
    , TextValue()
    , BooleanValue()
    , NumericValue()
    , DateCreated(0)
    , DateModified(0)
    , IsActive(false)
    , TaskId(-1)
    , AttributeId(-1)
    , AttributeTypeId(-1)
{
}

const std::string TaskAttributeValueModel::GetDateCreatedString() const
{
    date::sys_seconds dateTime{ std::chrono::seconds{ DateCreated } };
    std::string dateString = date::format("%Y-%m-%d %I:%M:%S %p", dateTime);
    return dateString;
}

const std::string TaskAttributeValueModel::GetDateModifiedString() const
{
    date::sys_seconds dateTime{ std::chrono::seconds{ DateModified } };
    std::string dateString = date::format("%Y-%m-%d %I:%M:%S %p", dateTime);
    return dateString;
}
}
