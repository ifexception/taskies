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

#include "attendedmeetingspersistence.h"

#include "../common/logmessages.h"
#include "../common/queryhelper.h"

#include "../utils/utils.h"

namespace tks::Persistence
{
AttendedMeetingsPersistence::AttendedMeetingsPersistence(std::shared_ptr<spdlog::logger> logger,
    const std::string& databaseFilePath)
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

AttendedMeetingsPersistence::~AttendedMeetingsPersistence()
{
    sqlite3_close(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::CloseDatabaseConnection);
}

int AttendedMeetingsPersistence::GetByEntryId(const std::string& entryId,
    Model::AttendedMeetingModel& attendedMeetingModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttendedMeetingsPersistence::getByEntryId.c_str(),
        static_cast<int>(AttendedMeetingsPersistence::getByEntryId.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            "AttendedMeetingsPersistence",
            AttendedMeetingsPersistence::getByEntryId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // entry_id
    rc = sqlite3_bind_text(
        stmt, bindIndex, entryId.c_str(), static_cast<int>(entryId.size()), SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate,
            "AttendedMeetingsPersistence",
            "entry_id",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;

            int columnIndex = 0;
            attendedMeetingModel.AttendedMeetingId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.EntryId = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Subject = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Start = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.End = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attendedMeetingModel.Duration = sqlite3_column_int(stmt, columnIndex++);

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Location = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attendedMeetingModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            attendedMeetingModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            attendedMeetingModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

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
        pLogger->error(LogMessages::ExecStepTemplate,
            "AttendedMeetingsPersistence",
            AttendedMeetingsPersistence::getByEntryId,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityGetById, "attended_meetings", entryId);

    return 0;
}

int AttendedMeetingsPersistence::GetByTodaysDate(const std::int32_t unixFromDateTime,
    const std::int32_t unixToDateTime,
    std::vector<Model::AttendedMeetingModel>& attendedMeetingModels) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttendedMeetingsPersistence::getByTodaysDate.c_str(),
        static_cast<int>(AttendedMeetingsPersistence::getByTodaysDate.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::PrepareStatementTemplate,
            "AttendedMeetingsPersistence",
            AttendedMeetingsPersistence::getByTodaysDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // date_created >=
    rc = sqlite3_bind_int(stmt, bindIndex, unixFromDateTime);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate,
            "AttendedMeetingsPersistence",
            "date_created",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // date_created <=
    rc = sqlite3_bind_int(stmt, bindIndex, unixToDateTime);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate,
            "AttendedMeetingsPersistence",
            "date_created",
            bindIndex,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bool done = false;
    while (!done) {
        switch (sqlite3_step(stmt)) {
        case SQLITE_ROW: {
            rc = SQLITE_ROW;
            Model::AttendedMeetingModel attendedMeetingModel;

            int columnIndex = 0;
            attendedMeetingModel.AttendedMeetingId = sqlite3_column_int64(stmt, columnIndex++);

            const unsigned char* res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.EntryId = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Subject = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Start = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.End = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attendedMeetingModel.Duration = sqlite3_column_int(stmt, columnIndex++);

            res = sqlite3_column_text(stmt, columnIndex);
            attendedMeetingModel.Location = std::string(
                reinterpret_cast<const char*>(res), sqlite3_column_bytes(stmt, columnIndex++));

            attendedMeetingModel.DateCreated = sqlite3_column_int(stmt, columnIndex++);
            attendedMeetingModel.DateModified = sqlite3_column_int(stmt, columnIndex++);
            attendedMeetingModel.IsActive = !!sqlite3_column_int(stmt, columnIndex++);

            attendedMeetingModels.push_back(attendedMeetingModel);
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
        pLogger->error(LogMessages::ExecStepTemplate,
            "AttendedMeetingsPersistence",
            AttendedMeetingsPersistence::getByTodaysDate,
            rc,
            error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);

    std::string searchFmt = "date_created >= " + std::to_string(unixFromDateTime) +
                            "date_created <= " + std::to_string(unixToDateTime);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::FilterEntities, "attended_meetings", searchFmt);

    return 0;
}

std::int64_t AttendedMeetingsPersistence::Create(
    const Model::AttendedMeetingModel& attendedMeetingModel) const
{
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(pDb,
        AttendedMeetingsPersistence::create.c_str(),
        static_cast<int>(AttendedMeetingsPersistence::create.size()),
        &stmt,
        nullptr);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::PrepareStatementTemplate, AttendedMeetingsPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    int bindIndex = 1;

    // entry_id
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attendedMeetingModel.EntryId.c_str(),
        static_cast<int>(attendedMeetingModel.EntryId.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "entry_id", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // subject
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attendedMeetingModel.Subject.c_str(),
        static_cast<int>(attendedMeetingModel.Subject.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "subject", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // start
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attendedMeetingModel.Start.c_str(),
        static_cast<int>(attendedMeetingModel.Start.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "start", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // end
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attendedMeetingModel.End.c_str(),
        static_cast<int>(attendedMeetingModel.End.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "end", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // duration
    rc = sqlite3_bind_int(stmt, bindIndex, attendedMeetingModel.Duration);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "name", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    // location
    rc = sqlite3_bind_text(stmt,
        bindIndex,
        attendedMeetingModel.Location.c_str(),
        static_cast<int>(attendedMeetingModel.Location.size()),
        SQLITE_TRANSIENT);

    if (rc != SQLITE_OK) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(LogMessages::BindParameterTemplate, "location", bindIndex, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    bindIndex++;

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_CONSTRAINT) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, AttendedMeetingsPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return SQLITE_CONSTRAINT * -1;
    }

    if (rc != SQLITE_DONE) {
        const char* error = sqlite3_errmsg(pDb);
        pLogger->error(
            LogMessages::ExecStepTemplate, AttendedMeetingsPersistence::create, rc, error);

        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    auto rowId = sqlite3_last_insert_rowid(pDb);
    SPDLOG_LOGGER_TRACE(pLogger, LogMessages::EntityCreated, "attended_meeting", rowId);

    return rowId;
}

std::string AttendedMeetingsPersistence::getByEntryId = "SELECT "
                                                        "entry_id, "
                                                        "subject, "
                                                        "start, "
                                                        "end, "
                                                        "duration, "
                                                        "location, "
                                                        "date_created, "
                                                        "date_modified, "
                                                        "is_active "
                                                        "FROM attended_meetings "
                                                        "WHERE entry_id = ? "
                                                        "AND is_active = 1;";

std::string AttendedMeetingsPersistence::getByTodaysDate = "SELECT "
                                                           "entry_id, "
                                                           "subject, "
                                                           "start, "
                                                           "end, "
                                                           "duration, "
                                                           "location, "
                                                           "date_created, "
                                                           "date_modified, "
                                                           "is_active "
                                                           "FROM attended_meetings "
                                                           "WHERE date_created >= ? "
                                                           "AND date_created <= ?"
                                                           "AND is_active = 1;";

std::string AttendedMeetingsPersistence::create = "INSERT INTO "
                                                  "attended_meetings "
                                                  "("
                                                  "entry_id, "
                                                  "subject, "
                                                  "start, "
                                                  "end, "
                                                  "duration, "
                                                  "location "
                                                  ") "
                                                  "VALUES (?,?,?,?,?,?);";
} // namespace tks::Persistence
