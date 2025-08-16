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

#include "excelexportservice.h"

#include <wx/string.h>

namespace tks::Services::Export
{
ExcelInstanceCheck::ExcelInstanceCheck()
    : mRegPath("Excel.Application/CurVer")
    , mKey(wxRegKey::HKCR, wxString(mRegPath))
{
}

bool ExcelInstanceCheck::IsExcelInstalled() const
{
    return mKey.Exists();
}

ExcelExportService::ExcelExportService()
    : mExcelInstanceCheck()
{
}

bool ExcelExportService::ExportToExcel(const SData& data)
{
    return false;
}
} // namespace tks::Service::Export
