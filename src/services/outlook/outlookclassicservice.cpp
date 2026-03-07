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

#include "outlookclassicservice.h"

#include "../../utils/mswutils.h"

namespace tks::Services::Outlook
{
OutlookResult OutlookResult::OK()
{
    return OutlookResult{ true, "" };
}

OutlookResult OutlookResult::PartialOK(const std::string& message)
{
    return OutlookResult{ true, message };
}

OutlookResult OutlookResult::Fail(const std::string& errorMessage)
{
    return OutlookResult{ false, errorMessage };
}

OutlookClassicService::OutlookClassicService(std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
{
}

OutlookResult OutlookClassicService::FetchAccountNames(std::vector<std::string>& accountNames) const
{
    wxAutomationObject outlookInstance;

    if (!outlookInstance.GetInstance("Outlook.Application")) {
        pLogger->error("Could not create Outlook instance");
        return OutlookResult::Fail("Failed to open Outlook application");
    }

    wxVariant mapiVariant("MAPI");
    const wxVariant namespaceDispatchPtr = outlookInstance.CallMethod("GetNamespace", mapiVariant);

    if (namespaceDispatchPtr.IsNull()) {
        pLogger->error("Failed to call \"GetNamespace\" method");
        return OutlookResult::Fail("Failed to get Outlook namespace");
    }

    wxAutomationObject namespaceObject;
    if (!VariantToObject(namespaceDispatchPtr, namespaceObject)) {
        pLogger->error("Could not convert variant to Namespace object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    const wxVariant accountsDispatchPtr = namespaceObject.GetProperty("Accounts");
    if (accountsDispatchPtr.IsNull()) {
        pLogger->error("Failed to get \"Accounts\" property");
        return OutlookResult::Fail("Failed to get to Outlook \"Namespace.Accounts\" property");
    }

    wxAutomationObject accountsObject;
    if (!VariantToObject(accountsDispatchPtr, accountsObject)) {
        pLogger->error("Could not convert variant to \"Accounts\" object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    const wxVariant accountCountProperty = accountsObject.GetProperty("Count");
    if (accountCountProperty.IsNull()) {
        pLogger->error("Failed to get \"Count\" property");
        return OutlookResult::Fail("Failed to get \"Accounts.Count\" property");
    }

    if (!(accountCountProperty.IsType("long") && accountCountProperty.GetLong() > 0)) {
        return OutlookResult::Fail("Type is incorrect or no accounts found in Outlook");
    }

    const long accountCount = accountCountProperty.GetLong();

    for (long i = 1; i <= accountCount; ++i) {
        wxVariant indexParam = i;
        const wxVariant accountDispatchPtr = accountsObject.CallMethod("Item", indexParam);
        if (accountDispatchPtr.IsNull()) {
            pLogger->error("Failed to call method \"Item\" with index {0:d}", i);
            return OutlookResult::Fail(
                fmt::format("Failed to call method \"Accounts.Item\" with index \"{0:d}\"", i));
        }

        wxAutomationObject accountObject;
        if (!VariantToObject(accountDispatchPtr, accountObject)) {
            pLogger->error("Could not convert variant to \"Account\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        const wxVariant displayName = accountObject.GetProperty("DisplayName");
        if (displayName.IsNull()) {
            pLogger->error("Failed to get property \"DisplayName\"");
            return OutlookResult::Fail(
                fmt::format("Failed to get property \"DisplayName\" at index \"{0:d}\"", i));
        } else {
            pLogger->info("Account.DisplayName = {0}", displayName.GetString().ToStdString());
        }

        accountNames.push_back(displayName.GetString().ToStdString());
    }

    return OutlookResult::OK();
}

OutlookResult OutlookClassicService::FetchCalendarMeetings(const std::string& accountName,
    std::vector<OutlookMeetingModel>& meetingModels) const
{
    wxAutomationObject outlookInstance;

    if (!outlookInstance.GetInstance("Outlook.Application")) {
        pLogger->error("Could not open/find Outlook instance");
        return OutlookResult::Fail("Failed to open Outlook application");
    }

    wxVariant mapiVariant("MAPI");
    const wxVariant namespaceDispatchPtr = outlookInstance.CallMethod("GetNamespace", mapiVariant);

    if (namespaceDispatchPtr.IsNull()) {
        pLogger->error("Failed to call \"GetNamespace\" method");
        return OutlookResult::Fail("Failed to get Outlook namespace");
    }

    wxAutomationObject namespaceObject;
    if (!VariantToObject(namespaceDispatchPtr, namespaceObject)) {
        pLogger->error("Could not convert variant to \"Namespace\" object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    const wxVariant accountsDispatchPtr = namespaceObject.GetProperty("Accounts");
    if (accountsDispatchPtr.IsNull()) {
        pLogger->error("Failed to get \"Accounts\" property");
        return OutlookResult::Fail("Failed to get to Outlook \"Namespace.Accounts\" property");
    }

    wxAutomationObject accountsObject;
    if (!VariantToObject(accountsDispatchPtr, accountsObject)) {
        pLogger->error("Could not convert variant to \"Accounts\" object");
        return OutlookResult::Fail("Conversion error occurred");
    }

    const wxVariant accountCountProperty = accountsObject.GetProperty("Count");
    if (accountCountProperty.IsNull()) {
        pLogger->error("Failed to get \"Count\" property");
        return OutlookResult::Fail("Failed to get \"Accounts.Count\" property");
    }

    if (!(accountCountProperty.IsType("long") && accountCountProperty.GetLong() > 0)) {
        return OutlookResult::Fail("Type is incorrect or no accounts found in Outlook");
    }

    const long accountCount = accountCountProperty.GetLong();

    for (long i = 1; i <= accountCount; ++i) {
        wxVariant indexParam = i;
        const wxVariant accountDispatchPtr = accountsObject.CallMethod("Item", indexParam);
        if (accountDispatchPtr.IsNull()) {
            pLogger->error("Failed to call method \"Item\" with index {0:d}", i);
            return OutlookResult::Fail(
                fmt::format("Failed to call method \"Accounts.Item\" with index \"{0:d}\"", i));
        }

        wxAutomationObject accountObject;
        if (!VariantToObject(accountDispatchPtr, accountObject)) {
            pLogger->error("Could not convert variant to \"Account\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        const wxVariant displayName = accountObject.GetProperty("DisplayName");
        if (displayName.IsNull()) {
            pLogger->error("Failed to get property \"DisplayName\"");
            return OutlookResult::Fail("Failed to get \"DisplayName\" property");
        } else {
            pLogger->info("Account.DisplayName = {0}", displayName.GetString().ToStdString());
        }

        if (displayName.GetString().ToStdString() != accountName) {
            continue;
        }

        const wxVariant deliveryStoreDispatchPtr = accountObject.GetProperty("DeliveryStore");
        if (deliveryStoreDispatchPtr.IsNull()) {
            pLogger->error("Failed to get \"DeliveryStore\" property");
            return OutlookResult::Fail("Failed to get \"Account.DeliveryStore\" property");
        }

        wxAutomationObject deliveryStoreObject;
        if (!VariantToObject(deliveryStoreDispatchPtr, deliveryStoreObject)) {
            pLogger->error("Could not convert variant to \"DeliveryStore\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        wxVariant calendarFolderParam = 9; // olFolderCalendar
        wxVariant calendarFolderDispatchPtr =
            deliveryStoreObject.CallMethod("GetDefaultFolder", calendarFolderParam);
        if (calendarFolderDispatchPtr.IsNull()) {
            pLogger->error("Failed to call \"GetDefaultFolder\" with {0} parameter",
                calendarFolderParam.GetInteger());

            return OutlookResult::Fail(fmt::format(
                "Failed to call method \"DeliveryStore.GetDefaultFolder\" with parameter \"{0}\"",
                calendarFolderParam.GetInteger()));
        }

        wxAutomationObject calendarFolderObject;
        if (!VariantToObject(calendarFolderDispatchPtr, calendarFolderObject)) {
            pLogger->error("Could not convert variant to \"CalendarFolder\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        wxAutomationObject calendarFolderItemsObject;
        if (!calendarFolderObject.GetObject(calendarFolderItemsObject, "Items")) {
            pLogger->error("Failed to get \"Items\" object");
            return OutlookResult::Fail("Failed to get \"GetDefaultFolder.Items\" object");
        }

        wxVariant includeRecurrencesParam = true;
        if (!calendarFolderItemsObject.PutProperty("IncludeRecurrences", includeRecurrencesParam)) {
            pLogger->error("Failed to set \"IncludeRecurrences\" property");
            return OutlookResult::Fail(
                "Failed to get \"GetDefaultFolder.Items.IncludeRecurrences\" object");
        }

        wxVariant sortByStartParam = "[Start]";
        calendarFolderItemsObject.CallMethod("Sort", sortByStartParam);

        wxDateTime today = wxDateTime::Now();
        wxString todaysDateString = today.Format("%Y/%m/%d");
        wxString restrictionFormatted = wxString::Format(
            "[Start] <= '%s 23:59' AND [End] >= '%s 00:00'", todaysDateString, todaysDateString);

        wxVariant restrictionParam = restrictionFormatted;
        const wxVariant filteredItemsDispatchPtr =
            calendarFolderItemsObject.CallMethod("Restrict", restrictionParam);
        if (filteredItemsDispatchPtr.IsNull()) {
            pLogger->error("Failed to call \"Restrict\" method with \"{0}\"",
                restrictionFormatted.ToStdString());

            return OutlookResult::Fail(fmt::format(
                "Failed to call method \"GetDefaultFolder.Restrict\" with parameter \"{0}\"",
                restrictionFormatted.ToStdString()));
        }

        wxAutomationObject filteredItemsObject;
        if (!VariantToObject(filteredItemsDispatchPtr, filteredItemsObject)) {
            pLogger->error("Could not convert variant to \"Items.Restrict\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        wxVariant itemObjectDispatchPtr = filteredItemsObject.CallMethod("GetFirst");

        // Method "GetFirst" will "fail" if there are no meeting items for this account
        // Checking if there is a void ptr in our variant tells us GetFirst could not get
        // any meeting items (this is for now an assumption, but works)
        if (!itemObjectDispatchPtr.GetVoidPtr() || itemObjectDispatchPtr.IsNull()) {
            pLogger->warn("\"GetFirst\" method did not return a valid void ptr or is null, because "
                          "most likely there are NO meetings for this account");
            return OutlookResult::PartialOK("No meetings found");
        }

        wxAutomationObject itemObject;
        if (!VariantToObject(itemObjectDispatchPtr, itemObject)) {
            pLogger->error("Could not convert variant to \"Item\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        do {
            if (!itemObject.IsOk()) {
                pLogger->info(
                    "Retrieved all meetings for \"{0}\"", displayName.GetString().ToStdString());
                break;
            }

            pLogger->info("=== MEETING INFO ===");
            pLogger->info(
                "=== === === === === === === === === === === === === === === === === ===");

            OutlookMeetingModel model;
            wxVariant entryIDProperty = itemObject.GetProperty("EntryID");
            if (!entryIDProperty.IsNull()) {
                pLogger->info("EntryID\t|\t{0}", entryIDProperty.GetString().ToStdString());
                model.EntryId = entryIDProperty.GetString().ToStdString();
            }

            wxVariant subjectProperty = itemObject.GetProperty("Subject");
            if (!subjectProperty.IsNull()) {
                pLogger->info("Subject\t|\t{0}", subjectProperty.GetString().ToStdString());
                model.Subject = subjectProperty.GetString().ToStdWstring();
            }

            wxVariant startProperty = itemObject.GetProperty("Start");
            if (!startProperty.IsNull()) {
                pLogger->info("[VT_DATE] Start\t|\t{0}", startProperty.GetString().ToStdString());

                auto formattedStart = MswUtils::ConvertAppointmentItemDateTimeToISODateTime(
                    startProperty.GetString().ToStdString());
                pLogger->info("[ISO] Start\t|\t{0}", formattedStart);
                model.Start = formattedStart;
            }
            wxVariant endProperty = itemObject.GetProperty("End");
            if (!endProperty.IsNull()) {
                pLogger->info("[VT_DATE] End\t|\t{0}", endProperty.GetString().ToStdString());

                auto formattedEnd = MswUtils::ConvertAppointmentItemDateTimeToISODateTime(
                    endProperty.GetString().ToStdString());
                pLogger->info("[ISO] End\t\t|\t{0}", formattedEnd);
                model.End = formattedEnd;
            }
            wxVariant durationProperty = itemObject.GetProperty("Duration");
            if (!durationProperty.IsNull()) {
                pLogger->info("Duration\t|\t{0}", durationProperty.GetString().ToStdString());
                model.Duration = durationProperty.GetLong();
            }
            wxVariant locationProperty = itemObject.GetProperty("Location");
            if (!locationProperty.IsNull()) {
                pLogger->info("Location\t|\t{0}", locationProperty.GetString().ToStdString());
                model.Location = locationProperty.GetString().ToStdString();
            }

            pLogger->info("=== ENDS ===");

            meetingModels.push_back(model);

            itemObjectDispatchPtr = filteredItemsObject.CallMethod("GetNext");
            if (itemObjectDispatchPtr.IsNull()) {
                pLogger->error("Failed to call \"GetNext\" method");
                return OutlookResult::Fail("Failed to call method \"Items.GetNext\"");
            }

            itemObject.SetDispatchPtr(itemObjectDispatchPtr);
        } while (true);
    }

    return OutlookResult::OK();
}

bool OutlookClassicService::VariantToObject(const wxVariant& v, wxAutomationObject& o) const
{
    wxCHECK_MSG(!o.GetDispatchPtr(), false, "o already contains an object");

    if (!v.GetVoidPtr())
        return false;

    o.SetDispatchPtr(v.GetVoidPtr());
    return true;
}
} // namespace tks::Services::Outlook
