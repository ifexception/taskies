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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <wx/msw/ole/automtn.h>
#include <wx/msw/ole/oleutils.h>

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>

#include "outlookmeetingmodel.h"

namespace tks::Services::Outlook
{
struct OutlookResult {
    bool Success;
    std::string Message;

    static OutlookResult OK();
    static OutlookResult PartialOK(const std::string& message);
    static OutlookResult Fail(const std::string& errorMessage);
};

constexpr int olFolderCalendar = 9;

class OutlookClassicService
{
public:
    OutlookClassicService(std::shared_ptr<spdlog::logger> logger);
    ~OutlookClassicService() = default;

    OutlookResult FetchAccountNames(std::vector<std::string>& accountNames);
    OutlookResult FetchCalendarMeetings(const std::string& accountName,
        std::vector<OutlookMeetingModel>& meetingModels);

private:
    std::shared_ptr<spdlog::logger> pLogger;
    wxAutomationObject mOutlookInstance;

    OutlookResult GetOutlookInstance();
    OutlookResult GetAccountsObject(wxAutomationObject& accountsObject);
    void ReadMeetings(wxAutomationObject& itemObject,
        std::vector<OutlookMeetingModel>& meetingModels);

    bool VariantToObject(const wxVariant& v, wxAutomationObject& o) const;
};
} // namespace tks::Services::Outlook
