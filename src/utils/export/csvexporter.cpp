// Productivity tool to help you track the time you spend on tasks
// Copyright (C) 2024 Szymon Welgus
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

#include <algorithm>
#include <cctype>

#include "../../common/constants.h"

#include "../utils.h"

namespace tks::Utils
{

CsvExportProcessor::CsvExportProcessor(Services::Export::CsvExportOptions options)
    : mOptions(options)
{
}

void CsvExportProcessor::ProcessData(std::stringstream& data, std::string& value)
{
    TryProcessEmptyValues(value);
    TryProcessNewLines(value);
    TryApplyTextQualifier(data, value);
}

void CsvExportProcessor::TryProcessNewLines(std::string& value) const
{
    if (mOptions.NewLinesHandler == NewLines::Merge) {
        std::string newline = "\n";
        std::string space = " ";

        value = Utils::ReplaceAll(value, newline, space);
    }
}

void CsvExportProcessor::TryProcessEmptyValues(std::string& value) const
{
    if (value.empty()) {
        if (mOptions.EmptyValuesHandler == EmptyValues::Null) {
            value = "NULL";
        }
    }
}

void CsvExportProcessor::TryApplyTextQualifier(std::stringstream& data, std::string& value) const
{
    std::string quote = "\"";
    std::string doubleQuote = "\"\"";

    value = Utils::ReplaceAll(value, quote, doubleQuote);

    if (mOptions.TextQualifier != '\0' && value.find(mOptions.Delimiter) != std::string::npos) {
        data << mOptions.TextQualifier << value << mOptions.TextQualifier;
    } else {
        data << value;
    }
}

CsvExporter::CsvExporter(const std::string& databaseFilePath, std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , mDatabaseFilePath(databaseFilePath)
{
    pQueryBuilder = std::make_unique<Services::Export::SQLiteExportQueryBuilder>(false);
}

std::vector<std::string> CsvExporter::ComputeProjectionModel(
    const std::vector<Services::Export::Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionMap;

    for (const auto& projection : projections) {
        const auto& columnProjection = projection.columnProjection.UserColumn;

        projectionMap.push_back(columnProjection);
    }

    return projectionMap;
}

bool CsvExporter::GeneratePreview(Services::Export::CsvExportOptions options,
    const std::vector<Services::Export::Projection>& projections,
    const std::vector<Services::Export::ColumnJoinProjection>& joinProjections,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    int rc = -1;
    std::vector<std ::vector<std::pair<std::string, std::string>>> projectionModel;

    mOptions = options;

    pQueryBuilder->IsPreview(true);
    const auto& sql = pQueryBuilder->Build(projections, joinProjections, fromDate, toDate);

    const auto& computedProjectionModel = ComputeProjectionModel(projections);

    ExportDao exportDao(mDatabaseFilePath, pLogger);
    rc = exportDao.FilterExportData(sql, computedProjectionModel, projectionModel);
    if (rc != 0) {
        return false;
    }

    CsvExportProcessor exportProcessor(mOptions);

    std::stringstream exportedData;

    if (!mOptions.ExcludeHeaders) {
        for (auto i = 0; i < computedProjectionModel.size(); i++) {
            exportedData << computedProjectionModel[i];
            if (i < computedProjectionModel.size() - 1) {
                exportedData << mOptions.Delimiter;
            }
        }
        exportedData << "\n";
    }

    for (const auto& rowModel : projectionModel) {
        for (auto i = 0; i < rowModel.size(); i++) {
            auto& rowValue = rowModel[i];
            std::string value = rowValue.second;

            exportProcessor.ProcessData(exportedData, value);

            if (i < rowModel.size() - 1) {
                exportedData << mOptions.Delimiter;
            }
        }

        exportedData << "\n";
    }

    exportedDataPreview = exportedData.str();

    return true;
}

ExportDao::ExportDao(const std::string& databaseFilePath, const std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , pDb(nullptr)
{
    pLogger->info(LogMessage::InfoOpenDatabaseConnection, "ExportDao", databaseFilePath);
    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::OpenDatabaseTemplate, "ExportDao", databaseFilePath, rc, std::string(err));
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::ForeignKeys, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ExportDao", Utils::sqlite::pragmas::ForeignKeys, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::JournalMode, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ExportDao", Utils::sqlite::pragmas::JournalMode, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::Synchronous, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ExportDao", Utils::sqlite::pragmas::Synchronous, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::TempStore, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ExportDao", Utils::sqlite::pragmas::TempStore, rc, err);
        return;
    }

    rc = sqlite3_exec(pDb, Utils::sqlite::pragmas::MmapSize, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecQueryTemplate, "ExportDao", Utils::sqlite::pragmas::MmapSize, rc, err);
        return;
    }
}

ExportDao::~ExportDao()
{
    sqlite3_close(pDb);
    pLogger->info(LogMessage::InfoCloseDatabaseConnection, "ExportDao");
}

int ExportDao::FilterExportData(const std::string& sql,
    const std::vector<std::string>& projectionMap,
    std::vector<std ::vector<std::pair<std::string, std::string>>>& projectionModel)
{
    pLogger->info(LogMessage::InfoBeginFilterEntities, "ExportDao", "<na>", "");

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::PrepareStatementTemplate, "ExportDao", sql, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            std::vector<std::pair<std::string, std::string>> rowProjectionModel;

            for (size_t i = 0; i < projectionMap.size(); i++) {
                int index = static_cast<int>(i);
                const auto& key = projectionMap[i];

                const unsigned char* res = sqlite3_column_text(stmt, index);
                const auto& value = std::string(reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, index));

                rowProjectionModel.push_back(std::make_pair(key, value));
            }

            projectionModel.push_back(rowProjectionModel);
            break;
        }
        case SQLITE_DONE:
            rc = SQLITE_DONE;
            done = true;
            break;
        default:
            break;
        }
    }

    if (rc != SQLITE_DONE) {
        const char* err = sqlite3_errmsg(pDb);
        pLogger->error(LogMessage::ExecStepTemplate, "ExportDao", sql, rc, err);
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    pLogger->info(LogMessage::InfoEndFilterEntities, "ExportDao", projectionModel.size(), "");
    return 0;
}
} // namespace tks::Utils
