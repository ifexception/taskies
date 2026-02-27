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

#include "mswutils.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#endif // _WIN32

namespace tks::MswUtils
{
// https://stackoverflow.com/a/29285933/7277716
OutlookInstanceCheck::OutlookInstanceCheck()
    : mRegPath("Outlook.Application\\CurVer")
    , mKey(wxRegKey::HKCR, wxString(mRegPath))
{
}

bool OutlookInstanceCheck::operator()() const
{
    if (mKey.Exists()) {
        wxString value = mKey.QueryDefaultValue();
        return !value.empty();
    }

    return false;
}

bool IsOutlookRunning()
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    TCHAR tOutlookName[] = TEXT("OUTLOOK");
    unsigned int i = 0;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return FALSE;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

            // Get a handle to the process.
            HANDLE hProcess =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (hProcess != NULL) {
                HMODULE hMod;
                DWORD cbNeeded;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
                    GetModuleBaseName(
                        hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                }
                CloseHandle(hProcess);

                if (wcsstr(szProcessName, tOutlookName)) {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
} // namespace tks::MswUtils
