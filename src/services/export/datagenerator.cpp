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

#include "datagenerator.h"

#include <cassert>
#include <unordered_map>

#include "exportsservice.h"
#include "row.h"

namespace tks::Services::Export
{
DataGenerator::DataGenerator(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath,
    bool isPreview,
    bool includeAttributes)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
    , bIsPreview(isPreview)
    , bIncludeAttributes(includeAttributes)
    , mQueryBuilder(isPreview)
{
}

bool DataGenerator::FillData(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    /*out*/ SData& data)
{
    /* initialize variables */
    int rc = -1;
    ExportsService exportsService(mDatabaseFilePath, pLogger);
    std::unordered_map<std::int64_t, Row<std::string>> rows;

    /*
     * get the headers from the projections built out from selected items from the list view
     * use the `UserColumn` as this is what the user renamed a potential header to
     * if a user did not rename a header, then it defaults to the "display" name
     */
    FillHeadersFromProjections(projections, data);

    /*
     * build the dynamic query factoring the projections built out from the selected items
     * from the list view, including the computed join projection, plus the from and to date range
     */
    const std::string& sql =
        mQueryBuilder.BuildQuery(projections, joinProjections, fromDate, toDate);

    /*
     * get the actual values (in the order the projections were built from the user selected items)
     * from the database
     * `headers.size()` indicates the number of values per row we need to retrieve
     * the `taskId` is the unique key for which each row that is fetched
     * the `taskId` is crucial for linking attributes
     * if the `includeAttributes` option is false, then it is not used
     * each row contains a std::vector of std::string's
     * each std::string contains the corresponding value relating to the headers position
     * see the function `FilterExportCsvData` for more detail
     */
    rc = exportsService.FilterExportDataFromGeneratedSql(sql, data.Headers.size(), rows);
    if (rc != 0) {
        pLogger->error("Failed to filter projected export data from generated SQL query. See "
                       "earlier logs for error detail");
        return false;
    }

    /* set the task row values into the `Rows` field of `SData` */
    data.Rows = rows;

    if (bIncludeAttributes) {
        /* see `GenerateAndExportAttributes` for more detail */
        bool attributeAddedSuccessfully = FillAttributes(fromDate, toDate, data);
        if (!attributeAddedSuccessfully) {
            pLogger->error("Failed to generate and attach attributes to export data. See earlier "
                           "logs for error detail");
            return false;
        }
    }

    return true;
}

bool DataGenerator::FillAttributes(const std::string& fromDate,
    const std::string& toDate,
    SData& data)
{
    /* initialize variables */
    std::optional<std::int64_t> taskId;
    ExportsService exportsService(mDatabaseFilePath, pLogger);
    int rc = -1;
    std::vector<std::string> attributeNames;
    std::unordered_map<std::int64_t, Row<HeaderValuePair>> attributeHeaderValueRows;

    if (bIsPreview) {
        /* if we are previewing data, `data.Rows.size()` should be one */
        assert(data.Rows.size() == 1);
        /*
         * with that information, we know that the below loop will only iterate over the rows
         * once and get the correct key / `taskId`
         */
        for (auto& row : data.Rows) {
            taskId = std::make_optional(row.first);
        }
    }

    /*
     * fetch all the attribute names (headers) that have been used for from and to date range
     * if we are in preview mode, only fetch attribute names for the date range and where
     * `taskId` matches
     * SQLite by defaults return the attribute names sorted alphabetically ascending
     */
    rc = exportsService.GetAttributeNames(fromDate, toDate, taskId, bIsPreview, attributeNames);
    if (rc != 0) {
        pLogger->error(
            "Failed to get attribute names for date range. See earlier logs for error detail");
        return false;
    }

    /* we have not found any attributes associated with the task or tasks so we can return */
    if (attributeNames.empty()) {
        pLogger->warn("No attribute names were found for date range. Nothing to do");
        return true;
    }

    /*
     * generate the query to get the attributes as a "header-value pair for from and to range
     * and if we are in preview mode, where `taskId` matches
     */
    const std::string& attributeSql = mQueryBuilder.BuildAttributesQuery(fromDate, toDate, taskId);

    /*
     * get the actual attribute names (headers) and their values and insert the pair into
     * the std::unordered_map with (again) the `taskId` serving as the unique key
     */
    rc = exportsService.FilterExportCsvAttributesData(attributeSql, attributeHeaderValueRows);
    if (rc != 0) {
        pLogger->error("Failed to filter attribute data from generated attribute SQL query. See "
                       "earlier logs for more detail");
        return false;
    }

    /* append the attribute names (headers) to the `SData` field `Headers` (order is important) */
    for (auto& attributeName : attributeNames) {
        data.Headers.push_back(attributeName);
    }

    /*
     * for each `taskId` key and corresponding row
     */
    for (auto& [taskIdKey, row] : data.Rows) {
        /* get the attribute header value pairs where we match with the `taskId` key */
        auto& headerValuePair = attributeHeaderValueRows[taskIdKey].Values;

        /*
         * we loop over attribute names (headers)
         * we do another loop over the header-value pairs retrieved from the database
         * until we find a header match
         * we then store value from the header-value pair in the `value` variable
         * importantly, we then break out the loop so we insert the value at the correct index
         * in the row of the `taskId` we are currently appending attributes to
         * if there is no match, then we just insert a blank (empty string) into the
         * row values std::vector at that index
         * and repeat
         */
        for (size_t i = 0; i < attributeNames.size(); i++) {
            std::string value = "";

            for (auto& headerValue : headerValuePair) {
                if (headerValue.Header == attributeNames[i]) {
                    value = headerValue.Value;
                    break;
                }
            }

            if (!value.empty()) {
                row.Values.push_back(value);
            } else {
                row.Values.push_back("");
            }
        }
    }

    return true;
}

void DataGenerator::FillHeadersFromProjections(const std::vector<Projection>& projections,
    SData& data)
{
    for (const auto& projection : projections) {
        const auto& userNamedHeader = projection.ColumnProjection.UserColumn;
        data.Headers.push_back(userNamedHeader);
    }
}
} // namespace tks::Services::Export
