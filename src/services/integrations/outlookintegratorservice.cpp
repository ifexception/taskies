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

#include "outlookguard.h"

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
    : pLogger(logger)
{
}

OutlookResult OutlookIntegratorService::FetchCalendarMeetings() const
{
    wxAutomationObject outlookInstance;
    //OutlookGuard outlookGuard{ outlookInstance };

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

    /*const wxVariant missingArgument(new wxVariantDataErrorCode(DISP_E_PARAMNOTFOUND));
    const wxVariant logonCall =
        namespaceObject.CallMethod("Logon", missingArgument, missingArgument, true, true);
    if (logonCall.IsNull()) {
        pLogger->error("Failed to call \"Logon\" method");
        return OutlookResult::Fail("Failed to logon to Outlook namespace");
    }*/

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
        } else {
            pLogger->info("Account.DisplayName = {0}", displayName.GetString().ToStdString());
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
        /*const wxVariant sortResult = */calendarFolderItemsObject.CallMethod("Sort", sortByStartParam);
        /*if (sortResult.GetVoidPtr() == nullptr) {
            pLogger->error("Failed to call \"Sort\" with {0} parameter",
                sortByStartParam.GetString().ToStdString());

            return OutlookResult::Fail(fmt::format(
                "Failed to call method \"GetDefaultFolder.Sort\" with parameter \"{0}\"",
                sortByStartParam.GetString().ToStdString()));
        }*/

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

        const wxVariant filteredItemsCount = filteredItemsObject.GetProperty("Count");
        if (filteredItemsCount.IsNull() && !filteredItemsCount.IsType("long")) {
            pLogger->error("Failed to get \"Count\" property");
            return OutlookResult::Fail("Failed to get \"Items.Count\" property");
        }

        const long itemsCount = filteredItemsCount.GetLong();

        wxVariant itemObjectDispatchPtr = filteredItemsObject.CallMethod("GetFirst");
        if (itemObjectDispatchPtr.IsNull()) {
            pLogger->error("Error calling \"GetFirst\" method");
            return OutlookResult::Fail("Failed to call method \"Items.GetFirst\"");
        }

        wxAutomationObject itemObject;
        if (!VariantToObject(itemObjectDispatchPtr, itemObject)) {
            pLogger->error("Could not convert variant to \"Item\" object");
            return OutlookResult::Fail("Conversion error occurred");
        }

        do {
            if (!itemObject.IsOk()) {
                pLogger->info("Retrieved all meetings for \"{0}\"",
                    displayName.GetString().ToStdString());
                break;
            }

            pLogger->info("=== MEETING INFO ===");
            pLogger->info(
                "=== === === === === === === === === === === === === === === === === ===");

            wxVariant entryIDProperty = itemObject.GetProperty("EntryID");
            if (!entryIDProperty.IsNull()) {
                pLogger->info("EntryID\t|\t{0}", entryIDProperty.GetString().ToStdString());
            }
            wxVariant subjectProperty = itemObject.GetProperty("Subject");
            if (!subjectProperty.IsNull()) {
                pLogger->info("Subject\t|\t{0}", subjectProperty.GetString().ToStdString());
            }
            wxVariant bodyProperty = itemObject.GetProperty("Body");
            if (!bodyProperty.IsNull()) {
                pLogger->info("Body\t\t|\t{0}", bodyProperty.GetString().ToStdString());
            }
            wxVariant startProperty = itemObject.GetProperty("Start");
            if (!startProperty.IsNull()) {
                pLogger->info("Start\t\t|\t{0}", startProperty.GetString().ToStdString());
            }
            wxVariant endProperty = itemObject.GetProperty("End");
            if (!endProperty.IsNull()) {
                pLogger->info("End\t\t|\t{0}", endProperty.GetString().ToStdString());
            }
            wxVariant durationProperty = itemObject.GetProperty("Duration");
            if (!durationProperty.IsNull()) {
                pLogger->info("Duration\t|\t{0}", durationProperty.GetString().ToStdString());
            }
            wxVariant locationProperty = itemObject.GetProperty("Location");
            if (!locationProperty.IsNull()) {
                pLogger->info("Location\t|\t{0}", locationProperty.GetString().ToStdString());
            }

            pLogger->info("=== ENDS ===");

            itemObjectDispatchPtr = filteredItemsObject.CallMethod("GetNext");
            if (itemObjectDispatchPtr.IsNull()) {
                pLogger->error("Failed to call \"GetNext\" method");
                return OutlookResult::Fail("Failed to call method \"Items.GetNext\"");
            }

            itemObject.SetDispatchPtr(itemObjectDispatchPtr);
        } while (true);

        /*for (long i = 0; i < itemsCount; i++) {
            if (!itemObject.IsOk()) {
                pLogger->error("\"Item\" object is an invalid state");
                return OutlookResult::Fail("\"Item\" object is an invalid state");
            }

            pLogger->info("=== MEETING INFO ===");
            pLogger->info(
                "=== === === === === === === === === === === === === === === === === ===");

            wxVariant entryIDProperty = itemObject.GetProperty("EntryID");
            if (!entryIDProperty.IsNull()) {
                pLogger->info("EntryID\t|\t{0}", entryIDProperty.GetString().ToStdString());
            }
            wxVariant subjectProperty = itemObject.GetProperty("Subject");
            if (!subjectProperty.IsNull()) {
                pLogger->info("Subject\t|\t{0}", subjectProperty.GetString().ToStdString());
            }
            wxVariant bodyProperty = itemObject.GetProperty("Body");
            if (!bodyProperty.IsNull()) {
                pLogger->info("Body\t\t|\t{0}", bodyProperty.GetString().ToStdString());
            }
            wxVariant startProperty = itemObject.GetProperty("Start");
            if (!startProperty.IsNull()) {
                pLogger->info("Start\t|\t{0}", startProperty.GetString().ToStdString());
            }
            wxVariant endProperty = itemObject.GetProperty("End");
            if (!endProperty.IsNull()) {
                pLogger->info("End\t\t|\t0}", endProperty.GetString().ToStdString());
            }
            wxVariant durationProperty = itemObject.GetProperty("Duration");
            if (!durationProperty.IsNull()) {
                pLogger->info("Duration\t|\t{0}", durationProperty.GetString().ToStdString());
            }
            wxVariant locationProperty = itemObject.GetProperty("Location");
            if (!locationProperty.IsNull()) {
                pLogger->info("Location\t|\t{0}", locationProperty.GetString().ToStdString());
            }

            pLogger->info("=== ENDS ===");

            itemObjectDispatchPtr = filteredItemsObject.CallMethod("GetNext");
            if (itemObjectDispatchPtr.IsNull()) {
                pLogger->error("Failed to call \"GetNext\" method");
                return OutlookResult::Fail("Failed to call method \"Items.GetNext\"");
            }

            if (!VariantToObject(itemObjectDispatchPtr, itemObject)) {
                pLogger->error("Could not convert variant to \"Item\" object");
                return OutlookResult::Fail("Conversion error occurred");
            }
        }*/
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
