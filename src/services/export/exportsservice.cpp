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

#include "exportsservice.h"

#include "../../common/logmessages.h"
#include "../../common/queryhelper.h"

#include "../../utils/utils.h"

namespace tks::Services::Export
{
ExportsService::ExportsService(const std::string& databaseFilePath,
    const std::shared_ptr<spdlog::logger> logger)
    : pLogger(logger)
    , pDb(nullptr)
{
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::OpenDatabaseConnection, databaseFilePath);

    int rc = sqlite3_open(databaseFilePath.c_str(), &pDb);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::OpenDatabaseTemplate, databaseFilePath, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::ForeignKeys, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::ForeignKeys, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::JournalMode, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::JournalMode, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::Synchronous, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::Synchronous, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::TempStore, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::TempStore, rc, error);

        return;
    }

    rc = sqlite3_exec(pDb, QueryHelper::MmapSize, nullptr, nullptr, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecQueryTemplate, QueryHelper::MmapSize, rc, error);

        return;
    }
}

ExportsService::~ExportsService()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int ExportsService::FilterExportCsvData(const std::string& sql,
    const std::size_t valueCount,
    std::unordered_map<std::int64_t, Row>& rows) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            auto res = sqlite3_column_int64(stmt, 0);
            auto taskId = static_cast<std::int64_t>(res);

            Row row;

            for (size_t i = 0; i < valueCount; i++) {
                int index = static_cast<int>(i);
                index++;

                const unsigned char* res = sqlite3_column_text(stmt, index);
                const auto& value = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, index));

                row.Values.push_back(value);
            }
            rows[taskId] = row;

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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, rows.size(), "<csv_export>");

    return 0;
}

int ExportsService::FilterExportCsvAttributesData(const std::string& sql,
    std::unordered_map<std::int64_t, HeaderValueRow>& headerValueRows) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            std::int64_t taskId = sqlite3_column_int64(stmt, ATTRIBUTE_PROP_INDEX_TASKID);

            HeaderValuePair headerValuePair;

            const unsigned char* resName = sqlite3_column_text(stmt, ATTRIBUTE_PROP_INDEX_NAME);
            const auto& valueName = std::string(reinterpret_cast<const char*>(resName),
                sqlite3_column_bytes(stmt, ATTRIBUTE_PROP_INDEX_NAME));

            const unsigned char* resValue = sqlite3_column_text(stmt, ATTRIBUTE_PROP_INDEX_VALUE);
            const auto& valueValue = std::string(reinterpret_cast<const char*>(resValue),
                sqlite3_column_bytes(stmt, ATTRIBUTE_PROP_INDEX_VALUE));

            headerValuePair.Header = valueName;
            headerValuePair.Value = valueValue;

            headerValueRows[taskId].HeaderValuePairs.push_back(headerValuePair);
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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::FilterEntities, headerValueRows.size(), "<csv_attributes_export>");

    return 0;
}

int ExportsService::GetAttributeNames(const std::string& fromDate,
    const std::string& toDate,
    std::optional<std::int64_t> taskId,
    bool isPreview,
    std::vector<std::string>& attributeNames) const
{
    std::string sql =
        isPreview ? ExportsService::getAttributeHeaderNamesPreview : getAttributeHeaderNames;

    size_t sqlSize = isPreview ? ExportsService::getAttributeHeaderNamesPreview.size()
                               : getAttributeHeaderNames.size();

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sqlSize), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // fromDate
    rc = sqlite3_bind_text(
        stmt, bindIndex, fromDate.c_str(), static_cast<int>(fromDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "from_date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // toDate
    rc = sqlite3_bind_text(
        stmt, bindIndex, toDate.c_str(), static_cast<int>(toDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "to_date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    if (isPreview && taskId.has_value()) {
        bindIndex++;

        rc = sqlite3_bind_int64(stmt, bindIndex, taskId.value());

        if (rc != SQLITE_OK) {
            const char* error = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

            sqlite3_finalize(stmt);
            return -1;
        }
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            int columnIndex = 0;

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            std::string name = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attributeNames.push_back(name);
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
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::ExecStepTemplate, "ExportsService", sql, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessages::FilterEntities,
        attributeNames.size(),
        fmt::format(
            "[{0}, {1}] - \"{2}\"", fromDate, toDate, taskId.has_value() ? taskId.value() : -1));

    return 0;
}

std::string ExportsService::getAttributeHeaderNames =
    "SELECT "
    "attributes.name "
    "FROM tasks "
    "INNER JOIN workdays ON tasks.workday_id = workdays.workday_id "
    "INNER JOIN task_attribute_values ON tasks.task_id = task_attribute_values.task_id "
    "INNER JOIN attributes ON task_attribute_values.attribute_id = attributes.attribute_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.is_active = 1 "
    "AND task_attribute_values.is_active = 1 "
    "GROUP BY attributes.name "
    "HAVING COUNT(DISTINCT attributes.name) > 0";

std::string ExportsService::getAttributeHeaderNamesPreview =
    "SELECT "
    "attributes.name "
    "FROM tasks "
    "INNER JOIN workdays ON tasks.workday_id = workdays.workday_id "
    "INNER JOIN task_attribute_values ON tasks.task_id = task_attribute_values.task_id "
    "INNER JOIN attributes ON task_attribute_values.attribute_id = attributes.attribute_id "
    "WHERE workdays.date >= ? "
    "AND workdays.date <= ? "
    "AND tasks.is_active = 1 "
    "AND task_attribute_values.is_active = 1 "
    "AND tasks.task_id = ? "
    "GROUP BY attributes.name "
    "HAVING COUNT(DISTINCT attributes.name) > 0";
} // namespace tks::Persistence
