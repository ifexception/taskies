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

#pragma once

#include <memory>
#include <string>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/msw/ole/automtn.h>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "../../common/enums.h"

#include "data.h"
#include "dataexportgenerator.h"
#include "excelexportprocessor.h"

#include "../../utils/utils.h"

namespace tks::Services::Export
{
constexpr int xlWorkbookDefaultSaveAs = 51;

struct ExcelExporterService {
    ExcelExporterService(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool includeAttributes,
        NewLines newLinesOption,
        BooleanHandler booleanHandlerOption);
    ~ExcelExporterService() = default;

    bool ExportToExcel(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        const std::string& saveLocation);

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;
    bool bIncludeAttributes;

    std::unique_ptr<DataExportGenerator> pDataExportGenerator;
    ExcelExportProcessor mExportDataProcessor;

    bool VariantToObject(const wxVariant& v, wxAutomationObject& o);
};
} // namespace tks::Services::Export
