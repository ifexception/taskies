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

#include "exportsservice.h"

#include "../../common/logmessages.h"

#include "../../common/messages/sqlitemessages.h"

#include "../../utils/utils.h"

constexpr int ATTRIBUTE_PROP_INDEX_TASKID = 0;
constexpr int ATTRIBUTE_PROP_INDEX_NAME = 1;
constexpr int ATTRIBUTE_PROP_INDEX_VALUE = 2;

namespace tks::Services::Export
{
ExportsService::ExportsService(const std::string& databaseFilePath,
    const std::shared_ptr<spdlog::logger> logger)
    : PersistenceBase(logger, databaseFilePath)
    , pLogger(logger)
{
}

SqliteResult ExportsService::FilterExportDataFromGeneratedSql(const std::string& sql,
    const std::size_t valueCount,
    std::unordered_map<std::int64_t, Row<std::string>>& rows) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            /* `taskId` is our key and will always be first */
            auto res = sqlite3_column_int64(stmt, 0);
            auto taskId = static_cast<std::int64_t>(res);

            Row<std::string> row;

            /* loop over how many headers / columns the user selected to export */
            for (size_t i = 0; i < valueCount; i++) {
                /*
                 * the projection is not aware that `taskId` is selected first
                 * so `index` needs to be manually incremented by 1 to match the correct
                 * column index of the query in SQLite
                 */
                int index = static_cast<int>(i);
                index++;

                const unsigned char* res = sqlite3_column_text(stmt, index);
                const auto& value = std::string(
                    reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, index));

                /* append the value to our row */
                row.Values.push_back(value);
            }
            /* set the row and our `taskId` key */
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
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, rows.size(), "<csv_export>");

    return SqliteResult::OK();
}

SqliteResult ExportsService::FilterExportCsvAttributesData(const std::string& sql,
    std::unordered_map<std::int64_t, Row<HeaderValuePair>>& headerValueRows) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            /* `taskId` is always selected first so correctly link with the main export data */
            std::int64_t taskId = sqlite3_column_int64(stmt, ATTRIBUTE_PROP_INDEX_TASKID);

            HeaderValuePair headerValuePair;

            /* the query always will return three columns */
            /* we get the attribute name (header) at ATTRIBUTE_PROP_INDEX_NAME index */
            const unsigned char* resName = sqlite3_column_text(stmt, ATTRIBUTE_PROP_INDEX_NAME);
            const auto& valueName = std::string(reinterpret_cast<const char*>(resName),
                sqlite3_column_bytes(stmt, ATTRIBUTE_PROP_INDEX_NAME));

            /* we get the attribute value at ATTRIBUTE_PROP_INDEX_VALUE index */
            const unsigned char* resValue = sqlite3_column_text(stmt, ATTRIBUTE_PROP_INDEX_VALUE);
            const auto& valueValue = std::string(reinterpret_cast<const char*>(resValue),
                sqlite3_column_bytes(stmt, ATTRIBUTE_PROP_INDEX_VALUE));

            /* build up the header value pair struct */
            headerValuePair.Header = valueName;
            headerValuePair.Value = valueValue;

            /* attach the header value pair struct to our header value vector and `taskId` index */
            headerValueRows[taskId].Values.push_back(headerValuePair);
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
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(
        pLogger, LogMessages::FilterEntities, headerValueRows.size(), "<csv_attributes_export>");

    return SqliteResult::OK();
}

SqliteResult ExportsService::GetAttributeNames(const std::string& fromDate,
    const std::string& toDate,
    std::optional<std::int64_t> taskId,
    bool isPreview,
    std::vector<std::string>& attributeNames) const
{
    std::string sql = isPreview ? ExportsService::getAttributeNamesPreview : getAttributeNames;

    size_t sqlSize =
        isPreview ? ExportsService::getAttributeNamesPreview.size() : getAttributeNames.size();

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb, sql.c_str(), static_cast<int>(sqlSize), &stmt, nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate, sql, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(
            Messages::PrepareStatementMessage, rc, std::string(error));
    }

    int bindIndex = 1;

    // fromDate
    rc = sqlite3_bind_text(
        stmt, bindIndex, fromDate.c_str(), static_cast<int>(fromDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "from_date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    bindIndex++;

    // toDate
    rc = sqlite3_bind_text(
        stmt, bindIndex, toDate.c_str(), static_cast<int>(toDate.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "to_date", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return SqliteResult::FailDetailed(Messages::BindStatementMessage, rc, std::string(error));
    }

    // task_id
    if (isPreview && taskId.has_value()) {
        bindIndex++;

        rc = sqlite3_bind_int64(stmt, bindIndex, taskId.value());

        if (rc != SQLITE_OK) {
            const char* error = sqlite3_errmsg(pDb);
            pLogger->error(LogMessages::BindParameterTemplate, "task_id", bindIndex, rc, error);

            sqlite3_finalize(stmt);
            return SqliteResult::FailDetailed(
                Messages::BindStatementMessage, rc, std::string(error));
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
        return SqliteResult::FailDetailed(Messages::StepStatementMessage, rc, std::string(error));
    }

    sqlite3_finalize(stmt);
    SPDLOG_LOGGER_TRACE(pLogger,
        LogMessages::FilterEntities,
        attributeNames.size(),
        fmt::format(
            "[{0}, {1}] - \"{2}\"", fromDate, toDate, taskId.has_value() ? taskId.value() : -1));

    return SqliteResult::OK();
}

std::string ExportsService::getAttributeNames =
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

std::string ExportsService::getAttributeNamesPreview =
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
} // namespace tks::Services::Export
