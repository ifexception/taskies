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

#include "csvexporter.h"

#include <cassert>
#include <unordered_map>

#include "exportsservice.h"

namespace tks::Services::Export
{
CsvExporter::CsvExporter(std::shared_ptr<spdlog::logger> logger,
    CsvExportOptions options,
    const std::string& databaseFilePath)
    : pLogger(logger)
    , mOptions(options)
    , mDatabaseFilePath(databaseFilePath)
    , bIsPreview(false)
{
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

bool CsvExporter::GeneratePreview(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    bIsPreview = true;
    pQueryBuilder->IsPreview(true);

    bool success =
        GenerateExport(projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::Generate(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    bIsPreview = false;
    pQueryBuilder->IsPreview(false);

    bool success =
        GenerateExport(projections, joinProjections, fromDate, toDate, exportedDataPreview);

    return success;
}

bool CsvExporter::GenerateExport(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    /* initialize variables */
    int rc = -1;
    ExportsService exportsService(mDatabaseFilePath, pLogger);
    std::unordered_map<std::int64_t, Row> rows;

    /*
     * get the headers from the projections built out from selected items from the list view
     * use the `UserColumn` as this is what the user renamed a potential header to
     * if a user did not rename a header, then it defaults to the "display" name
     */
    std::vector<std::string> headers = GetHeadersFromProjections(projections);
    if (headers.empty()) {
        pLogger->warn("No headers were found in the projections. Nothing further to do");
        return true;
    }

    /*
     * build the dynamic query factoring the projections built out from the selected items
     * from the list view, including the computed join projection, plus the from and to date range
     */
    const auto& sql = pQueryBuilder->BuildQuery(projections, joinProjections, fromDate, toDate);

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
    rc = exportsService.FilterExportCsvData(sql, headers.size(), rows);
    if (rc != 0) {
        pLogger->error("Failed to filter projected export data from generated SQL query. See "
                       "earlier logs for error detail");
        return false;
    }

    /* `SData` is our main struct to store the headers and rows */
    SData exportData;

    /* insert the headers we got from the projections into the `Headers` field of `SData` */
    for (auto& header : headers) {
        exportData.Headers.push_back(header);
    }

    /* set the task row values into the `Rows` field of `SData` */
    exportData.Rows = rows;

    if (mOptions.IncludeAttributes) {
        /* see `GenerateAttributes` for more detail */
        bool attributeAddedSuccessfully = GenerateAttributes(fromDate, toDate, exportData);
        if (!attributeAddedSuccessfully) {
            pLogger->error("Failed to generate and attach attributes to export data. See earlier "
                           "logs for error detail");
            return false;
        }
    }

    /*
     * initialize the csv mapped options object to map delimiter and text qualifier options
     * to a char value to be used in the processor
     */
    CsvMappedOptions mappedOptions(mOptions);

    /* initialize the export value processor with the csv options and their mapped options */
    CsvExportProcessor exportProcessor(mOptions, mappedOptions);

    std::stringstream exportedData;

    /* check if the user opted out to include headers */
    if (!mOptions.ExcludeHeaders) {
        /* append all headers from the `SData` `Headers` field to our stringstream object */
        for (auto i = 0; i < exportData.Headers.size(); i++) {
            exportedData << exportData.Headers[i];
            if (i < exportData.Headers.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }
        exportedData << "\n";
    }

    /*
     * loop over row from our `SData` struct `Rows` field
     */
    for (const auto& row : exportData.Rows) {
        /* get our row values std::vector */
        const auto& rowValues = row.second.Values;
        for (size_t i = 0; i < rowValues.size(); i++) {
            /*
             * process / modify each value individually through the export processor
             * and apply the csv options where applicable and alter the value accordingly
             */
            std::string value = rowValues[i];
            exportProcessor.ProcessData(value);

            /* append the processed value to our stringstream object */
            exportedData << value;

            if (i < rowValues.size() - 1) {
                exportedData << mappedOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    /* verify the data in stringstream is in a good state */
    if (!exportedData.good()) {
        pLogger->error("Exported data in stringstream object is not in a good state");
        return false;
    }

    /* set the out string and return */
    exportedDataPreview = exportedData.str();
    return true;
}

bool CsvExporter::GenerateAttributes(const std::string& fromDate,
    const std::string& toDate,
    SData& data)
{
    /* initialize variables */
    std::optional<std::int64_t> taskId;
    ExportsService exportsService(mDatabaseFilePath, pLogger);
    int rc = -1;
    std::vector<std::string> attributeNames;
    std::unordered_map<std::int64_t, HeaderValueRow> attributeHeaderValueRows;

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
            "Failed to get attribute names for data range. See earlier logs for error detail");
        return false;
    }

    /* we have not found any attributes associated with the task or tasks so we can return */
    if (attributeNames.empty()) {
        pLogger->warn("No attribute names were found for data range. Nothing to do");
        return true;
    }

    /*
     * generate the query to get the attributes as a "header-value pair for from and to range
     * and if we are in preview mode, where `taskId` matches
     */
    const std::string& attributeSql = pQueryBuilder->BuildAttributesQuery(fromDate, toDate, taskId);

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
        auto& headerValuePair = attributeHeaderValueRows[taskIdKey].HeaderValuePairs;

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

std::vector<std::string> CsvExporter::GetHeadersFromProjections(
    const std::vector<Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> headers;

    for (const auto& projection : projections) {
        const auto& userNamedHeader = projection.ColumnProjection.UserColumn;
        headers.push_back(userNamedHeader);
    }

    return headers;
}
} // namespace tks::Services::Export
