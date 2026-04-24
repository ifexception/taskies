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
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/results/sqliteresult.h"

#include "../../persistence/base/persistencebase.h"

#include "headervaluepair.h"
#include "row.h"

namespace tks::Services::Export
{
struct ExportsService final : public Persistence::PersistenceBase {
public:
    ExportsService() = delete;
    ExportsService(const ExportsService&) = delete;
    explicit ExportsService(const std::string& databaseFilePath,
        const std::shared_ptr<spdlog::logger> logger);
    virtual ~ExportsService() = default;

    ExportsService& operator=(ExportsService&) = delete;

    SqliteResult FilterExportDataFromGeneratedSql(const std::string& sql,
        const std::size_t valueCount,
        /*out*/ std::unordered_map<std::int64_t, Row<std::string>>& rows) const;

    SqliteResult FilterExportCsvAttributesData(const std::string& sql,
        /*out*/ std::unordered_map<std::int64_t, Row<HeaderValuePair>>& headerValueRows) const;

    SqliteResult GetAttributeNames(const std::string& fromDate,
        const std::string& toDate,
        std::optional<std::int64_t> taskId,
        bool isPreview,
        /*out*/ std::vector<std::string>& attributeNames) const;

    static std::string getAttributeNames;
    static std::string getAttributeNamesPreview;
};
} // namespace tks::Services::Export
