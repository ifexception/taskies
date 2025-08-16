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

#include "../../common/enums.h"

namespace tks::Services::Export
{
CsvExporter::CsvExporter(std::shared_ptr<spdlog::logger> logger,
    CsvExportOptions options,
    const std::string& databaseFilePath,
    bool isPreview)
    : pLogger(logger)
    , mOptions(options)
    , mDatabaseFilePath(databaseFilePath)
    , bIsPreview(isPreview)
    , pDataExporter(nullptr)
{
    pDataExporter = std::make_unique<DataExporter>(
        pLogger, mDatabaseFilePath, bIsPreview, mOptions.IncludeAttributes);
}

bool CsvExporter::Generate(const std::vector<Projection>& projections,
    const std::vector<ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedData)
{
    /* `SData` is our main struct to store the headers and rows */
    SData exportData;

    bool success = pDataExporter->GenerateExportData(
        projections, joinProjections, fromDate, toDate, exportData);
    if (!success) {
        pLogger->error("Failed to generate export data. See earlier logs for detail");
        return false;
    }

    /*
     * initialize the csv mapped options object to map delimiter and text qualifier options
     * to a char value to be used in the processor
     */
    CsvMappedOptions mappedOptions(mOptions);

    /* initialize the export value processor with the csv options and their mapped options */
    CsvExportProcessor exportProcessor(mOptions, mappedOptions);

    std::stringstream exportedDataStringStream;

    /* check if the user opted out to include headers */
    if (!mOptions.ExcludeHeaders) {
        /* append all headers from the `SData` `Headers` field to our stringstream object */
        for (auto i = 0; i < exportData.Headers.size(); i++) {
            exportedDataStringStream << exportData.Headers[i];
            if (i < exportData.Headers.size() - 1) {
                exportedDataStringStream << mappedOptions.Delimiter;
            }
        }
        exportedDataStringStream << "\n";
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
            exportedDataStringStream << value;

            if (i < rowValues.size() - 1) {
                exportedDataStringStream << mappedOptions.Delimiter;
            }
        }

        exportedDataStringStream << "\n";
    }

    /* verify the data in stringstream is in a good state */
    if (!exportedDataStringStream.good()) {
        pLogger->error("Exported data in stringstream object is not in a good state");
        return false;
    }

    /* set the out string and return */
    exportedData = exportedDataStringStream.str();
    return true;
}
} // namespace tks::Services::Export
