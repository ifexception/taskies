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

#include <cctype>

#include "../../common/constants.h"

#include "../utils.h"

namespace tks::Utils
{
CsvExportOptions::CsvExportOptions()
    : Delimiter(',')
    , TextQualifier('"')
    , EmptyValuesHandler(EmptyValues::Blank)
    , NewLinesHandler(NewLines::Merge)
    , ExcludeHeaders(false)
{
}

CsvExportProcessor::CsvExportProcessor(CsvExportOptions options)
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
    pQueryBuilder = std::make_unique<SQLiteExportQueryBuilder>(false);
}

std::vector<std::string> CsvExporter::ComputeProjectionModel(const std::vector<Projection>& projections)
{
    if (projections.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionMap;

    for (const auto& projection : projections) {
        const auto& columnProjection = projection.columnProjection.userColumn;

        projectionMap.push_back(columnProjection);
    }

    return projectionMap;
}

bool CsvExporter::GeneratePreview(CsvExportOptions options,
    const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate,
    std::string& exportedDataPreview)
{
    int rc = -1;
    std::vector<std ::vector<std::pair<std::string, std::string>>> projectionModel;

    mOptions = options;

    pQueryBuilder->IsPreview(true);
    const auto& sql = pQueryBuilder->Build(projections, firstLevelJoinTables, secondLevelJoinTables, fromDate, toDate);

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

// #####################################################################################################################

ColumnProjection::ColumnProjection()
    : databaseColumn("")
    , userColumn("")
    , columnTableName("")
    , identifier("")
{
}

ColumnProjection::ColumnProjection(std::string databaseColumn, std::string userColumn, std::string columnTableName)
    : databaseColumn(databaseColumn)
    , userColumn(userColumn)
    , columnTableName(columnTableName)
{
}

void ColumnProjection::SetIdentifier(const std::string value)
{
    identifier = value;
}

Projection::Projection()
    : orderIndex(-1)
    , columnProjection()
{
}

Projection::Projection(int orderIndex, ColumnProjection columnProjection)
    : orderIndex(orderIndex)
    , columnProjection(columnProjection)
{
}

SQLiteExportQueryBuilder::SQLiteExportQueryBuilder(bool isPreview)
    : bIsPreview(isPreview)
{
}

const bool SQLiteExportQueryBuilder::IsPreview() const
{
    return bIsPreview;
}

void SQLiteExportQueryBuilder::IsPreview(const bool preview)
{
    bIsPreview = preview;
}

std::string SQLiteExportQueryBuilder::Build(const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate)
{
    return BuildQuery(projections, firstLevelJoinTables, secondLevelJoinTables, fromDate, toDate);
}

std::string SQLiteExportQueryBuilder::BuildQuery(const std::vector<Projection>& projections,
    const std::vector<FirstLevelJoinTable>& firstLevelJoinTables,
    const std::vector<SecondLevelJoinTable>& secondLevelJoinTables,
    const std::string& fromDate,
    const std::string& toDate)
{
    const auto& columns = ComputeProjections(projections);
    const auto& firstLevelJoins = ComputeFirstLevelJoins(firstLevelJoinTables);
    const auto& secondLevelJoins = ComputeSecondLevelJoins(secondLevelJoinTables);
    const auto& where = BuildWhere(fromDate, toDate);

    std::string query = BuildQueryString(columns, firstLevelJoins, secondLevelJoins, where);
    return query;
}

std::string SQLiteExportQueryBuilder::BuildQueryString(const std::vector<std::string>& columns,
    const std::vector<std::string>& firstLevelJoins,
    const std::vector<std::string>& secondLevelJoins,
    const std::string& where)
{
    std::stringstream query;

    query << "SELECT ";
    AppendColumns(query, columns);

    query << "FROM tasks ";
    query << "INNER JOIN workdays ";
    query << "ON tasks.workday_id = workdays.workday_id ";

    if (!firstLevelJoins.empty()) {
        AppendJoins(query, firstLevelJoins);
    }

    if (!secondLevelJoins.empty()) {
        query << " ";
        AppendJoins(query, secondLevelJoins);
    }

    AppendClause(query, " WHERE ", where);

    if (bIsPreview) {
        AppendClause(query, " LIMIT ", "10");
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeFirstLevelJoins(
    const std::vector<FirstLevelJoinTable>& joinTables)
{
    if (joinTables.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinTable : joinTables) {
        std::string joinStatement = ComputeFirstLevelJoin(joinTable);
        computedJoins.push_back(joinStatement);
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeFirstLevelJoin(const FirstLevelJoinTable& joinTable)
{
    std::stringstream query;
    if (joinTable.joinType == JoinType::InnerJoin) {
        query << "INNER JOIN " << joinTable.tableName << " ON "
              << "tasks"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeSecondLevelJoins(
    const std::vector<SecondLevelJoinTable>& joinTables)
{
    if (joinTables.empty()) {
        return std::vector<std::string>();
    }

    std::vector<std::string> computedJoins;
    for (const auto& joinTable : joinTables) {
        auto joinStatement = ComputeSecondLevelJoin(joinTable);

        computedJoins.push_back(joinStatement);
    }

    return computedJoins;
}

std::string SQLiteExportQueryBuilder::ComputeSecondLevelJoin(const SecondLevelJoinTable& joinTable)
{
    std::stringstream query;

    if (joinTable.joinType == JoinType::InnerJoin) {
        query << "INNER JOIN " << joinTable.tableName << " ON "
              << "projects"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }
    if (joinTable.joinType == JoinType::LeftJoin) {
        query << "LEFT JOIN " << joinTable.tableName << " ON "
              << "projects"
              << "." << joinTable.idColumn << " = " << joinTable.tableName << "." << joinTable.idColumn;
    }

    return query.str();
}

std::vector<std::string> SQLiteExportQueryBuilder::ComputeProjections(const std::vector<Projection>& projections)
{
    if (projections.size() == 0) {
        return std::vector<std::string>();
    }

    std::vector<std::string> projectionsOut;

    for (const auto& projection : projections) {
        std::string projectionOut = ComputeSingleProjection(projection);
        projectionsOut.push_back(projectionOut);
    }

    return projectionsOut;
}

std::string SQLiteExportQueryBuilder::ComputeSingleProjection(const Projection& projection)
{
    std::stringstream query;
    ColumnProjection cp = projection.columnProjection;

    if (!cp.userColumn.empty() && cp.identifier.empty()) {
        query << cp.columnTableName << "." << cp.databaseColumn << " AS "
              << "\"" << cp.userColumn << "\"";
    } else if (!cp.userColumn.empty() && !cp.identifier.empty()) {
        query << "(printf('%02d', " << cp.columnTableName << ".hours)"
              << " || "
              << "':'"
              << " || "
              << "printf('%02d'," << cp.columnTableName << ".minutes))"
              << " AS "
              << "\"" << cp.userColumn << "\"";
    } else if (cp.userColumn.empty() && !cp.identifier.empty()) {
        query << "(printf('%02d', " << cp.columnTableName << ".hours)"
              << " || "
              << "':'"
              << " || "
              << "printf('%02d'," << cp.columnTableName << ".minutes))"
              << " AS "
              << "Duration";
    } else {
        query << cp.columnTableName << "." << cp.databaseColumn;
    }

    return query.str();
}
std::string SQLiteExportQueryBuilder::BuildWhere(const std::string& fromDate, const std::string& toDate)
{
    if (fromDate.empty() || toDate.empty()) {
        return std::string();
    }

    std::stringstream whereClause;

    whereClause << "workdays.date"
                << " >= "
                << "'" << fromDate << "'"
                << " AND "
                << "workdays.date"
                << " <= "
                << "'" << toDate << "'";

    return whereClause.str();
}

void SQLiteExportQueryBuilder::AppendColumns(std::stringstream& query, const std::vector<std::string>& columns)
{
    for (auto i = 0; i < columns.size(); i++) {
        const auto& column = columns[i];
        if (!column.empty()) {
            query << column;
            if (i != columns.size() - 1) {
                query << ", ";
            }
        }
    }
    query << " ";
}

void SQLiteExportQueryBuilder::AppendJoins(std::stringstream& query, const std::vector<std::string>& joins)
{
    for (auto i = 0; i < joins.size(); i++) {
        const auto& join = joins[i];
        if (!join.empty()) {
            query << join;
            if (i != joins.size() - 1) {
                query << " ";
            }
        }
    }
}

void SQLiteExportQueryBuilder::AppendClause(std::stringstream& query, std::string name, std::string clause)
{
    if (!clause.empty()) {
        query << name << clause;
    }
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
