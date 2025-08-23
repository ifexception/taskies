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
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include "columnjoinprojection.h"
#include "projection.h"
#include "sqliteexportquerybuilder.h"
#include "data.h"

namespace tks::Services::Export
{
struct DataGenerator final {
    DataGenerator() = delete;
    DataGenerator(const DataGenerator&) = delete;
    DataGenerator(std::shared_ptr<spdlog::logger> logger,
        const std::string& databaseFilePath,
        bool isPreview,
        bool includeAttributes);
    ~DataGenerator() = default;

    DataGenerator& operator=(const DataGenerator&) = delete;

    bool FillData(const std::vector<Projection>& projections,
        const std::vector<ColumnJoinProjection>& joinProjections,
        const std::string& fromDate,
        const std::string& toDate,
        /*out*/ SData& data);

    bool FillAttributes(const std::string& fromDate,
        const std::string& toDate,
        /*out*/ SData& data);

    static void FillHeadersFromProjections(const std::vector<Projection>& projections, SData& data);

    std::shared_ptr<spdlog::logger> pLogger;
    std::string mDatabaseFilePath;
    bool bIsPreview;
    bool bIncludeAttributes;

    SQLiteExportQueryBuilder mQueryBuilder;
};
} // namespace tks::Services::Export
