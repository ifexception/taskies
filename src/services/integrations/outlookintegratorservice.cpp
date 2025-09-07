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

#include "outlookintegratorservice.h"

namespace tks::Services::Integrations
{
OutlookResult OutlookResult::OK()
{
    return OutlookResult{ true, "" };
}

OutlookResult OutlookResult::Fail(const std::string& errorMessage)
{
    return OutlookResult{ false, errorMessage };
}

OutlookIntegratorService::OutlookIntegratorService(std::shared_ptr<spdlog::logger> logger)
    : mOutlookInstance()
{
}

OutlookResult OutlookIntegratorService::GetAllCalendarMeetings()
{
    if (!mOutlookInstance.GetInstance("Outlook.Application")) {
        pLogger->error("Could not create Outlook instance");
        return OutlookResult::Fail("Failed to open Outlook application");
    }

    wxVariant mapiVariant("MAPI");
    const wxVariant namespaceDispatchPtr = mOutlookInstance.CallMethod("GetNamespace", mapiVariant);

    if (namespaceDispatchPtr.IsNull()) {
        pLogger->error("Failed to call \"GetNamespace\" method");
        return OutlookResult::Fail("Failed to get Outlook namespace");
    }

    wxAutomationObject namespaceObject;
    if (!VariantToObject(namespaceDispatchPtr, namespaceObject)) {
        pLogger->error("Could not convert variant to Namespace object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    const wxVariant missingArgument(new wxVariantDataErrorCode(DISP_E_PARAMNOTFOUND));
    const wxVariant logonCall =
        namespaceObject.CallMethod("Logon", missingArgument, missingArgument, true, true);
    if (logonCall.IsNull()) {
        pLogger->error("Failed to call \"Logon\" method");
        return OutlookResult::Fail("Failed to logon to Outlook namespace");
    }

    const wxVariant accountsDispatchPtr = namespaceObject.GetProperty("Accounts");
    if (accountsDispatchPtr.IsNull()) {
        pLogger->error("Failed to get \"Accounts\" property");
        return OutlookResult::Fail("Failed to get to Outlook \"Accounts\" property");
    }

    wxAutomationObject accountsObject;
    if (!VariantToObject(accountsDispatchPtr, accountsObject)) {
        pLogger->error("Could not convert variant to Accounts object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    return OutlookResult::OK();
}

bool OutlookIntegratorService::VariantToObject(const wxVariant& v, wxAutomationObject& o) const
{
    wxCHECK_MSG(!o.GetDispatchPtr(), false, "o already contains an object");

    if (!v.GetVoidPtr())
        return false;

    o.SetDispatchPtr(v.GetVoidPtr());
    return true;
}
} // namespace tks::Services::Integrations
