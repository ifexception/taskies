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

#include "outlookmeetingmodel.h"

#include "../../utils/utils.h"

namespace tks::Services::Outlook
{
OutlookMeetingModel::OutlookMeetingModel()
    : EntryId("")
    , Subject(L"")
    , Start("")
    , End("")
    , Duration()
    , Location("")
{
}

const std::string OutlookMeetingModel::TrimmedSubject() const
{
    std::string trimmedSubject = Utils::RemoveEmoticons(Subject);
    return trimmedSubject;
}

bool OutlookMeetingModel::operator==(const OutlookMeetingModel& other)
{
    return EntryId == other.EntryId;
}
std::string OutlookMeetingModel::DebugPrint() const
{
    std::string debugPrint = "Subject\n" + TrimmedSubject() + "\nStart: " + Start + " End: " + End +
                             "\nDuration: " + std::to_string(Duration) + "\nLocation:\n" +
                             Location;
    return "\n==DEBUG PRINT==\n\n" + debugPrint;
}
} // namespace tks::Services::Integrations
